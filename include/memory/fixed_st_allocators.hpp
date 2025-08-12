/******************************************************************************/
/*!
 * @file   fixed_st_allocators.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Contains non-growing single threaded allocators.
 *
 * @copyright Copyright (c) 2023-2024 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_FIXED_ST_ALLOCATORS_HPP
#define LIB_FOUNDATION_MEMORY_FIXED_ST_ALLOCATORS_HPP

#include "alignment.hpp"    // DefaultAlignment
#include "basic_types.hpp"  // byte, AllocationResult

namespace Memory
{
  //-------------------------------------------------------------------------------------//
  // Linear Allocator
  //-------------------------------------------------------------------------------------//

  /*!
   * @brief
   *   This allocator is very good for temporary scoped memory allocations.
   *   There is no individual deallocation but a whole clear operation.
   */
  class LinearAllocator
  {
    friend class LinearAllocatorSavePoint;

   private:
    byte*       m_MemoryBgn;
    const byte* m_MemoryEnd;
    byte*       m_Current;

   public:
    LinearAllocator() = default;
    LinearAllocator(byte* const memory_block, const MemoryIndex memory_block_size) noexcept;

    MemoryIndex UsedMemory() const { return m_Current - m_MemoryBgn; }
    MemoryIndex TotalMemory() const { return m_MemoryEnd - m_MemoryBgn; }
    const byte* MemoryBgn() const { return m_MemoryBgn; }
    const byte* MemoryEnd() const { return m_MemoryEnd; }
    bool        IsPtrInRange(const void* const ptr) const { return m_MemoryBgn <= static_cast<const byte*>(ptr) && static_cast<const byte*>(ptr) < m_MemoryEnd; }

    void             Init(byte* const memory_block, const MemoryIndex memory_block_size);
    void             Clear() noexcept { m_Current = m_MemoryBgn; }
    bool             CanServiceAllocation(const MemoryIndex size, const MemoryIndex alignment) const noexcept;
    AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& /* source_info */) noexcept;
    void             Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept;
  };

  inline LinearAllocator LinearAllocatorFromMemoryRequirements(void* const buffer, const MemoryRequirements mem_reqs)
  {
    MemAssert(mem_reqs.IsBufferValid(buffer), "Buffer improperly aligned.");
    return Memory::LinearAllocator{reinterpret_cast<byte*>(buffer), mem_reqs.size};
  }

  /*!
   * @copydoc LinearAllocator
   */
  template<MemoryIndex k_BufferSize, MemoryIndex alignment = DefaultAlignment>
  class FixedLinearAllocator : public LinearAllocator
  {
   private:
    alignas(alignment) byte m_Buffer[k_BufferSize];

   public:
    FixedLinearAllocator() noexcept :
      LinearAllocator(m_Buffer, sizeof(m_Buffer)),
      m_Buffer{}
    {
    }
  };

  class LinearAllocatorSavePoint
  {
   protected:
    LinearAllocator* m_Allocator;     //!< The allocator to restore to.
    byte*            m_RestorePoint;  //!< The point in memory to go back to.

   public:
    void Save(LinearAllocator& allocator) noexcept;
    void Restore() noexcept;
  };

  struct LinearAllocatorScope : protected LinearAllocatorSavePoint
  {
    LinearAllocatorScope(LinearAllocator& allocator) noexcept :
      LinearAllocatorSavePoint{}
    {
      Save(allocator);
    }

    LinearAllocatorScope(const LinearAllocatorScope& rhs) noexcept            = delete;
    LinearAllocatorScope(LinearAllocatorScope&& rhs) noexcept                 = delete;
    LinearAllocatorScope& operator=(const LinearAllocatorScope& rhs) noexcept = delete;
    LinearAllocatorScope& operator=(LinearAllocatorScope&& rhs) noexcept      = delete;

    ~LinearAllocatorScope() noexcept { Restore(); }
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
  class StackAllocator
  {
   private:
    byte*       m_StackPtr;
    const byte* m_MemoryEnd;

   public:
    StackAllocator(byte* const memory_block, MemoryIndex memory_block_size) noexcept;

    AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& /* source_info  */) noexcept;
    void             Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept;
  };

  //-------------------------------------------------------------------------------------//
  // Pool Allocator
  //-------------------------------------------------------------------------------------//

  struct PoolAllocatorBlock
  {
    PoolAllocatorBlock* next;
  };

  struct PoolAllocatorSetupResult
  {
    PoolAllocatorBlock* head;
    PoolAllocatorBlock* tail;
    MemoryIndex         num_elements;
  };

  /*!
   * @brief
   *  Features O(1) allocation and O(1) deletion by chunking up the memory into
   *  fixed sized block.
   *
   *  The PoolAllocatorBlock does not actually take up any memory since
   *  it is only used when it is in the pool freelist.
   */
  class PoolAllocator
  {
   private:
    byte* const         m_MemoryBgn;
    byte* const         m_MemoryEnd;
    MemoryIndex         m_BlockSize;
    MemoryIndex         m_Alignment;
    PoolAllocatorBlock* m_PoolHead;
    MemoryIndex         m_NumElements;

   public:
    PoolAllocator(byte* const memory_block, const MemoryIndex memory_size, const MemoryIndex block_size, const MemoryIndex alignment) noexcept;

    void             Reset() noexcept;
    MemoryIndex      IndexOf(const void* ptr) const noexcept;
    void*            FromIndex(const MemoryIndex index) noexcept;  // The index must have been from 'IndexOf'
    AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& /* source_info  */) noexcept;
    void             Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept;

    static PoolAllocatorSetupResult SetupPool(byte* const memory_block, const MemoryIndex memory_size, const MemoryIndex block_size, const MemoryIndex alignment) noexcept;
  };

  /*!
   * @copydoc PoolAllocator
   */
  template<MemoryIndex kblock_size, MemoryIndex num_blocks, MemoryIndex alignment = DefaultAlignment>
  class FixedPoolAllocator : public PoolAllocator
  {
    static constexpr std::size_t header_alignment  = alignof(PoolAllocatorBlock);
    static constexpr std::size_t actual_alignment  = alignment < header_alignment ? header_alignment : alignment;
    static constexpr std::size_t actual_block_size = Memory::AlignSize<actual_alignment>(kblock_size);
    static constexpr std::size_t memory_block_size = actual_block_size * num_blocks;

   private:
    /*!
     * @brief
     *   `buffer` is not initialized by design since the `PoolAllocator` ctor writes to it.
     */
    alignas(actual_alignment) byte m_Buffer[memory_block_size];

   public:
    FixedPoolAllocator() :
      PoolAllocator(m_Buffer, sizeof(m_Buffer), actual_block_size)
    {
    }
  };

  //-------------------------------------------------------------------------------------//
  // FreeList Allocator
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
  class FreeListAllocator
  {
   private:
    FreeListNode* m_Freelist;

   public:
    FreeListAllocator(byte* const memory_block, MemoryIndex memory_block_size);

    AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& /* source_info  */) noexcept;
    void             Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept;

   private:
    AllocationResult AllocateInternal(const MemoryIndex size) noexcept;
    void             DeallocateInternal(void* const ptr, const MemoryIndex size) noexcept;
  };

}  // namespace Memory

#endif  // LIB_FOUNDATION_MEMORY_FIXED_ST_ALLOCATORS_HPP

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2023-2024 Shareef Abdoul-Raheem

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
