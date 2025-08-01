/******************************************************************************/
/*!
 * @file   default_heap.cpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *    The default global thread-safe heap allocator.
 *
 * @copyright Copyright (c) 2023 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#include "memory/default_heap.hpp"

#if !BF_MEMORY_NO_DEFAULT_HEAP

#include <new>  // align_val_t, nothrow

#include <cstdio>

struct CxxFreeStoreAllocator
{
  AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& source_info) const noexcept
  {
    (void)source_info;

    std::printf("[Memory] Allocation from %s:%s(%u) (size = %zu).\n", source_info.file, source_info.function, source_info.line, size);

    void* const ptr = ::operator new(size, std::align_val_t{alignment}, std::nothrow);

    return ptr ? AllocationResult{ptr, size} : AllocationResult::Null();
  }

  void Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) const noexcept
  {
    ::operator delete(ptr, size, std::align_val_t{alignment});
  }
};

#if BF_MEMORY_DEBUG_HEAP
using HeapAllocator = Allocator<CxxFreeStoreAllocator, AllocationMarkPolicy::MARKED, BoundCheckingPolicy::CHECKED, Memory::NoMemoryTracking>;
#else
using HeapAllocator = Allocator<CxxFreeStoreAllocator, AllocationMarkPolicy::UNMARKED, BoundCheckingPolicy::UNCHECKED, Memory::NoMemoryTracking>;
#endif

static HeapAllocator s_DefaultHeap = {};

IPolymorphicAllocator& Memory::DefaultHeap() noexcept
{
  return s_DefaultHeap;
}

#endif

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
