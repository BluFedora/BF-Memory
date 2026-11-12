/******************************************************************************/
/*!
 * @file   fixed_mt_allocators.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Contains non-growing thread-safe allocators.
 *
 * @copyright Copyright (c) 2023-2026 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_FIXED_MT_ALLOCATORS_HPP
#define LIB_FOUNDATION_MEMORY_FIXED_MT_ALLOCATORS_HPP

#include "basic_types.hpp"  // byte, AllocationResult

#include <atomic>        // std::atomic<T*>
#include <shared_mutex>  // shared_mutex

namespace Memory
{
  /*!
   * @brief
   *   This allocator is very good for temporary scoped memory allocations.
   *   There is no individual deallocation but a whole clear operation.
   */
  class MT_LinearAllocator
  {
    static_assert(std::atomic<byte*>::is_always_lock_free, "Atomic pointer expected to be lock-free.");

   private:
    byte* const        m_MemoryBgn;
    byte* const        m_MemoryEnd;
    std::atomic<byte*> m_Current;

   public:
    MT_LinearAllocator(byte* const memory_block, const MemoryIndex memory_block_size) noexcept;

    void             Clear() noexcept { m_Current.store(m_MemoryBgn); }
    AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& /* source_info */) noexcept;
    void             Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept;
  };

  class MT_PoolSource
  {
    friend class MT_Pool;

   private:
    struct Slab;

   private:
    IPolymorphicAllocator& m_ParentAllocator;
    const SizeAlign        m_ObjectSizeAlign;
    const MemoryIndex      m_SlabCapacity;
    SizeAlign              m_SlabSizeAlign;
    const MemoryIndex      m_ObjectOffset;
    std::atomic<Slab*>     m_ActiveSlab;
    Slab*                  m_AllSlabs;
    std::shared_mutex      m_GrowMutex;

   public:
    MT_PoolSource(IPolymorphicAllocator& parent_allocator, const MemoryIndex object_size, const MemoryIndex object_align, const MemoryIndex slab_capacity);
    ~MT_PoolSource() noexcept;

   private:
    void EnsurePoolHasItems(MT_Pool* const pool);
  };

  class MT_Pool
  {
    friend class MT_PoolSource;

   private:
    struct FreeNode;

   private:
    MT_PoolSource* const m_Source;
    const MemoryIndex    m_BatchSize;
    MT_PoolSource::Slab* m_Slab;
    MemoryIndex          m_BgnIndex;
    MemoryIndex          m_EndIndex;
    FreeNode*            m_FreeNodesHead;

   public:
    MT_Pool(MT_PoolSource* const source_pool, const MemoryIndex batch_size);

    AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& /* source_info */) noexcept;

    void Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept;
  };
}  // namespace Memory

#endif  // LIB_FOUNDATION_MEMORY_FIXED_MT_ALLOCATORS_HPP

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2023-2026 Shareef Abdoul-Raheem

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
