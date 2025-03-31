/******************************************************************************/
/*!
 * @file   growing_st_allocators.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Contains growing single threaded allocators.
 *
 * @copyright Copyright (c) 2023-2025 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_GROWING_ST_ALLOCATORS_HPP
#define LIB_FOUNDATION_MEMORY_GROWING_ST_ALLOCATORS_HPP

#include "basic_types.hpp"  // IPolymorphicAllocator, MemoryIndex

namespace Memory
{
  //-------------------------------------------------------------------------------------//
  // Growing Pool Allocator: Like PoolAllocator except that it grows in chunks.
  //-------------------------------------------------------------------------------------//

  struct PoolAllocatorBlock;

  class GrowingPoolAllocator
  {
   private:
    struct ChunkFooter
    {
      // byte[m_ChunkMemSize];
      ChunkFooter* next;
    };

   private:
    IPolymorphicAllocator& m_ParentAllocator;
    MemoryIndex            m_BlockSize;
    MemoryIndex            m_Alignment;
    MemoryIndex            m_ChunkMemSize;
    ChunkFooter*           m_Chunks;
    PoolAllocatorBlock*    m_PoolHead;

   public:
    GrowingPoolAllocator(
     IPolymorphicAllocator& parent_allocator,
     const MemoryIndex      block_size,
     const MemoryIndex      block_alignment,
     const MemoryIndex      num_blocks_per_chunk) noexcept;

    IPolymorphicAllocator& ParentAllocator() const { return m_ParentAllocator; }

    void             Clear() noexcept;
    AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& source_info) noexcept;
    void             Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept;
    void             FreeMemory() noexcept;

    ~GrowingPoolAllocator() noexcept { FreeMemory(); }
  };

  template<MemoryIndex BlockSize, MemoryIndex BlockAlignment, MemoryIndex NumBlocksPerChunk>
  class StaticGrowingPoolAllocator : public GrowingPoolAllocator
  {
    static_assert(NumBlocksPerChunk > 0u, "Number of items in each chunk must be greater than 0.");
    static_assert(BlockSize >= sizeof(void*), "BlockAlignment must be >= sizeof(PoolAllocatorBlock).");
    static_assert(BlockAlignment > 0u, "BlockAlignment must be greater than alignof(void*).");

   public:
    StaticGrowingPoolAllocator(IPolymorphicAllocator& parent_allocator) :
      GrowingPoolAllocator(parent_allocator, BlockSize, BlockAlignment, NumBlocksPerChunk)
    {
    }
  };

  template<typename T, MemoryIndex NumBlocksPerChunk>
  using ObjectPool = StaticGrowingPoolAllocator<sizeof(T), alignof(T), NumBlocksPerChunk>;

}  // namespace Memory

#endif  // LIB_FOUNDATION_MEMORY_GROWING_ST_ALLOCATORS_HPP

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2023-2025 Shareef Abdoul-Raheem

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
