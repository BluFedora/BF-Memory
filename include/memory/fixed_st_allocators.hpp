/******************************************************************************/
/*!
 * @file   fixed_st_allocators.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Contains non-growing single threaded allocators.
 *
 * @copyright Copyright (c) 2023 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_FIXED_ST_ALLOCATORS_HPP
#define LIB_FOUNDATION_MEMORY_FIXED_ST_ALLOCATORS_HPP

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
  struct LinearAllocatorState
  {
   private:
    byte* const       m_MemoryBgn;
    const byte* const m_MemoryEnd;
    byte*             m_Current;

   public:
    LinearAllocatorState(byte* const memory_block, const MemoryIndex memory_block_size);

    void             Clear() { m_Current = m_MemoryBgn; }
    bool             CanServiceAllocation(const MemoryIndex size, const MemoryIndex alignment) const;
    AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment);
    void             Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment);
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