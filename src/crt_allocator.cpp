/******************************************************************************/
/*!
 * @file   crt_allocator.cpp
 * @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
 * @date   2019-12-26
 * @brief
 *  This allocator is a wrapper around the built in memory allocator.
 *  Implemented using "malloc" and "free".
 *
 * @copyright Copyright (c) 2019-2022
 */
/******************************************************************************/
#include "bf/memory/crt_allocator.hpp"

#include <cstdlib> /* calloc, malloc, free */

#if 0
#include <new>

void* ::operator new(std::size_t size)
{
  void* const ptr = malloc(size);

  if (!ptr)
  {
    throw std::bad_alloc{};
  }

  return ptr;
}

void* ::operator new(std::size_t size, const std::nothrow_t&) noexcept
{
  try
  {
    return malloc(size);
  }
  catch (...)
  {
  }

  return nullptr;
}

void* ::operator new[](std::size_t size)
{
  return malloc(size);
}

void* ::operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
  try
  {
    return malloc(size);
  }
  catch (...)
  {
  }

  return nullptr;
}

void ::operator delete(void* ptr)
{
  free(ptr);
}

void ::operator delete(void* ptr, const std::nothrow_t&) noexcept
{
  free(ptr);
}

void ::operator delete[](void* ptr)
{
  free(ptr);
}

void ::operator delete[](void* ptr, const std::nothrow_t&) noexcept
{
  free(ptr);
}
#endif

namespace bf
{
  static AllocationResult crt_alloc(IAllocator* const self, const std::size_t size)
  {
    void* const result = std::malloc(size);

    if (result != nullptr)
    {
      return {result, size};
    }

    return AllocationResult::Null();
  }

  static void crt_dealloc(IAllocator* const self, const AllocationResult mem_block)
  {
    std::free(mem_block.ptr);
  }

  CRTAllocator::CRTAllocator() :
    IAllocator(&crt_alloc, &crt_dealloc, alignof(max_align_t))
  {
  }
}  // namespace bf

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2019-2022 Shareef Abdoul-Raheem

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
