/******************************************************************************/
/*!
 * @file   fixed_mt_allocators.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Contains non-growing thread-safe allocators.
 *
 * @copyright Copyright (c) 2023 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_FIXED_ST_ALLOCATORS_HPP
#define LIB_FOUNDATION_MEMORY_FIXED_ST_ALLOCATORS_HPP

#include "basic_types.hpp"  // byte, AllocationResult

#include <atomic>  // std::atomic<T*>

namespace Memory
{
  //-------------------------------------------------------------------------------------//
  // Concurrent Linear Allocator
  //-------------------------------------------------------------------------------------//

  /*!
   * @brief
   *   This allocator is very good for temporary scoped memory allocations.
   *   There is no individual deallocation but a whole clear operation.
   */
  class ConcurrentLinearAllocator
  {
    static_assert(std::atomic<byte*>::is_always_lock_free, "Atomic pointer expected to be lock-free.");

   private:
    byte* const        m_MemoryBgn;
    byte* const        m_MemoryEnd;
    std::atomic<byte*> m_Current;

   public:
    ConcurrentLinearAllocator(byte* const memory_block, const MemoryIndex memory_block_size) noexcept;

    void             Clear() noexcept { m_Current.store(m_MemoryBgn); }
    AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& /* source_info  */) noexcept;
    void             Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept;
  };
}  // namespace Memory

#endif  // LIB_FOUNDATION_MEMORY_FIXED_ST_ALLOCATORS_HPP

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2023 Shareef Abdoul-Raheem

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
