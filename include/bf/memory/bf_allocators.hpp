/******************************************************************************/
/*!
 * @file   bf_allocators.hpp
 * @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
 * @date   2021-03-15
 * @brief
 *   Defines global allocators for the program.
 *
 * @copyright Copyright (c) 2021-2022
 */
/******************************************************************************/
#ifndef BF_ALLOCATORS_HPP
#define BF_ALLOCATORS_HPP

#include "bf_imemory_manager.hpp"   // IMemoryManager
#include "bf_linear_allocator.hpp"  // LinearAllocator

namespace bf
{
  // Usage:
  //   Declare a `MemoryContext` on the stack and reassign `general_heap` and/or `temp_heap`.
  //
  //   MemoryContext ctx;
  //   ctx.general_heap = &my_new_allocator.
  //
  //  By default the allocators are set to whatever the parent context had.
  //
  //  NOTE: These memory contexts are thread local.
  //
  struct MemoryContext
  {
    MemoryContext*   parent_ctx;
    IMemoryManager*  general_heap;
    LinearAllocator* temp_heap;

    MemoryContext(const MemoryContext& rhs) = delete;
    MemoryContext(MemoryContext&& rhs)      = delete;
    MemoryContext& operator=(const MemoryContext& rhs) = delete;
    MemoryContext& operator=(MemoryContext&& rhs) = delete;

    MemoryContext(IMemoryManager* general_heap = nullptr, LinearAllocator* temp_heap = nullptr);
    ~MemoryContext();
  };

  MemoryContext&   ParentMemoryContext();
  IMemoryManager&  GeneralHeap();
  LinearAllocator& TempHeap();
}  // namespace bf

#endif /* BF_ALLOCATORS_HPP */

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2021-2022 Shareef Abdoul-Raheem

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
