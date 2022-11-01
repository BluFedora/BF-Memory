/******************************************************************************/
/*!
 * @file   basic_allocators.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @date   2022-09-17
 * @brief
 *   Contains basic single-threaded allocators that manage a single continuous
 *   block of memory.
 *
 * @copyright Copyright (c) 2022 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef BF_BASIC_ALLOCATORS_HPP
#define BF_BASIC_ALLOCATORS_HPP

#include "memory_api.hpp"  // IAllocator

namespace bf
{
  //-------------------------------------------------------------------------------------//
  // Linear Allocator
  //-------------------------------------------------------------------------------------//

  /*!
   * @brief
   *   This allocator is very good for temporary scoped memory allocations.
   *   There is no individual deallocation but a whole clear operation.
   */
  struct LinearAllocator : public IAllocator
  {
    byte*       memory_bgn;
    const byte* memory_end;
    byte*       current;

    LinearAllocator(byte* const memory_block, std::size_t memory_block_size);

    void clear(void);
  };

  /*!
   * @copydoc LinearAllocator
   */
  template<std::size_t k_BufferSize, std::size_t alignment = alignof(std::max_align_t)>
  struct FixedLinearAllocator : public LinearAllocator
  {
    alignas(alignment) byte buffer[k_BufferSize];

    FixedLinearAllocator() :
      LinearAllocator(buffer, k_BufferSize),
      buffer{}
    {
    }
  };

  struct LinearAllocatorSavePoint
  {
    LinearAllocator* allocator;      //!< private: The allocator to restore to.
    byte*            restore_point;  //!< private: The point in memory to go back to.

    void save(LinearAllocator& allocator);
    void restore();
  };

  struct LinearAllocatorScope : private LinearAllocatorSavePoint
  {
   public:
    inline LinearAllocatorScope(LinearAllocator& allocator) :
      LinearAllocatorSavePoint{}
    {
      save(allocator);
    }

    inline ~LinearAllocatorScope() { restore(); }

    LinearAllocatorScope(const LinearAllocatorScope& rhs)                = delete;
    LinearAllocatorScope(LinearAllocatorScope&& rhs) noexcept            = delete;
    LinearAllocatorScope& operator=(const LinearAllocatorScope& rhs)     = delete;
    LinearAllocatorScope& operator=(LinearAllocatorScope&& rhs) noexcept = delete;
  };

  //-------------------------------------------------------------------------------------//
  // Stack Allocator
  //-------------------------------------------------------------------------------------//

  /*!
   * @brief
   *   This allocator is a designed for allocations where you can guarantee
   *   deallocation is in a LIFO (Last in First Out) order in return you get
   *   some speed.
   */
  struct StackAllocator : public IAllocator
  {
    byte*       stack_ptr;
    const byte* mem_block_end;

    StackAllocator(byte* const memory_block, std::size_t memory_block_size);
  };

  //-------------------------------------------------------------------------------------//
  // Pool Allocator
  //-------------------------------------------------------------------------------------//

  struct PoolAllocatorBlock
  {
    PoolAllocatorBlock* next;
  };

  /*!
   * @brief
   *  Features O(1) allocation and O(1) deletion by chunking up the memory into
   *  fixed sized block.
   *
   *  The PoolAllocatorBlock does not actually take up any memory since
   *  it is only used when it is in the pool freelist.
   */
  struct PoolAllocator : public IAllocator
  {
    byte* const         memory_bgn;
    PoolAllocatorBlock* pool_head;
    std::size_t         block_size;
    std::size_t         num_elements;

    /*!
     * @brief
     *   Make sure \p pool_block_size is aligned up to the designed alignment.
     */
    PoolAllocator(byte* const memory_block, std::size_t memory_block_size, std::size_t pool_block_size);

    std::size_t indexOf(const void* ptr) const;
    void*       fromIndex(std::size_t index);  // The index must have been from 'indexOf'
    void        reset();

    // memory_bgn bust be atleast `block_size` * `num_elements` in size.
    static PoolAllocatorBlock* setupFreelist(byte* const memory_bgn, std::size_t block_size, std::size_t num_elements);
  };

  /*!
   * @copydoc PoolAllocator
   */
  template<std::size_t kblock_size, std::size_t num_blocks, std::size_t alignment = alignof(std::max_align_t)>
  struct FixedPoolAllocator : public PoolAllocator
  {
    static constexpr std::size_t header_alignment  = alignof(PoolAllocatorBlock);
    static constexpr std::size_t actual_alignment  = alignment < header_alignment ? header_alignment : alignment;
    static constexpr std::size_t actual_block_size = bfAlignUpSize(kblock_size, actual_alignment);
    static constexpr std::size_t memory_block_size = actual_block_size * num_blocks;

    /*!
     * @brief
     *   `buffer` is not initialized by design since the `PoolAllocator` ctor writes to every byte anyway.
     */
    alignas(actual_alignment) byte buffer[memory_block_size];

    FixedPoolAllocator() :
      PoolAllocator(buffer, sizeof(buffer), actual_block_size)
    {
    }
  };

  //-------------------------------------------------------------------------------------//
  // Chunk Pool Allocator: Like PoolAllocator except that it grows in chunks.
  //-------------------------------------------------------------------------------------//

  template<std::size_t k_BlockSize, std::size_t k_NumBlocksInChunk>
  struct FixedChunkPoolAllocator : public IAllocator
  {
    static_assert(k_NumBlocksInChunk > 0u, "Number of items in each chunk must be greater than 0.");

    static constexpr std::size_t BlockSize        = k_BlockSize < sizeof(PoolAllocatorBlock) ? sizeof(PoolAllocatorBlock) : k_BlockSize;
    static constexpr std::size_t NumBlocksInChunk = k_NumBlocksInChunk;

    struct Chunk
    {
      byte   buffer[BlockSize * k_NumBlocksInChunk];
      Chunk* next;
    };

    Chunk*              chunks;
    PoolAllocatorBlock* pool_head;
    IAllocator&         chunk_allocator;

    FixedChunkPoolAllocator(IAllocator& allocator) :
      IAllocator(
       +[](IAllocator* const self_, const std::size_t size) -> AllocationResult {
         bfMemAssert(size <= BlockSize, "This Allocator is made for Objects of size %u (not %u)!", unsigned(BlockSize), unsigned(size));

         FixedChunkPoolAllocator* const self = static_cast<FixedChunkPoolAllocator*>(self_);

       pool_alloc:
         PoolAllocatorBlock* const block = self->pool_head;

         if (block != nullptr)
         {
           self->pool_head = block->next;

           return {reinterpret_cast<void*>(block), BlockSize};
         }

         Chunk* const new_chunk = bfMemAllocate<Chunk>(self->chunk_allocator);

         if (new_chunk)
         {
           new_chunk->next = std::exchange(self->chunks, new_chunk);

           self->pool_head = PoolAllocator::setupFreelist(
            new_chunk->buffer,
            BlockSize,
            k_NumBlocksInChunk);

           goto pool_alloc;
         }

         return AllocationResult::Null();
       },
       +[](IAllocator* const self_, const AllocationResult mem_block) -> void {
         FixedChunkPoolAllocator* const self = static_cast<FixedChunkPoolAllocator*>(self_);

         bfMemAssert(mem_block.num_bytes <= BlockSize, "That allocation did not come from this allocator.");

         PoolAllocatorBlock* const block = static_cast<PoolAllocatorBlock*>(mem_block.ptr);

         block->next = std::exchange(self->pool_head, block);
       }),
      chunks{nullptr},
      pool_head{nullptr},
      chunk_allocator{allocator}
    {
    }

    ~FixedChunkPoolAllocator()
    {
      Chunk* chunk = chunks;

      while (chunk)
      {
        Chunk* const next_chunk = chunk->next;

        bfMemDeallocate(chunk_allocator, chunk);

        chunk = next_chunk;
      }
    }
  };

  //-------------------------------------------------------------------------------------//
  // Freelist Allocator
  //-------------------------------------------------------------------------------------//

  struct FreeListNode;

  /*!
   * @brief
   *   This allocator is a the most generic custom allocator with the heaviest
   *   overhead.
   *
   *   This has the largest header size of any of the basic allocators but can be
   *   used as a direct replacement for "malloc/free" and "new/delete" if
   *   not called from multiple threads.
   *
   *   - Allocation   : A first fit policy is used.
   *   - Deallocation : Added to freelist in address order, block merging is attempted.
   */
  struct FreeListAllocator : public IAllocator
  {
    FreeListNode* freelist;

    FreeListAllocator(byte* const memory_block, std::size_t memory_block_size);
  };

}  // namespace bf

#endif /* BF_MEMORY_API_HPP */

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2022 Shareef Abdoul-Raheem

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
/******************************************************************************/
