/******************************************************************************/
/*!
* @file   bf_linear_allocator.cpp
* @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
* @brief
*   This allocator is very good for temporary memory allocations throughout
*   the frame. There is no individual deallocation but a whole clear operation
*   that should happen at the beginning (or end) of each 'frame'.
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2021
*/
/******************************************************************************/
#include "bf/memory/bf_linear_allocator.hpp"

#include <new> /* bad_alloc  */

#include <cassert>  // assert

namespace bf
{
  LinearAllocator::LinearAllocator(void* memory_block, const std::size_t memory_block_size) :
    MemoryManager((char*)memory_block, memory_block_size),
    m_MemoryOffset(0u)
  {
  }

  void LinearAllocator::clear()
  {
#ifdef BF_MEMORY_DEBUG_WIPE_MEMORY
    std::memset(begin(), BF_MEMORY_DEBUG_SIGNATURE, m_MemoryOffset);
#endif

    m_MemoryOffset = 0;
  }

  void* LinearAllocator::allocate(const std::size_t size)
  {
    char* const ptr = currentBlock();

    if ((ptr + size) <= end())
    {
      m_MemoryOffset += size;
      return ptr;
    }

    throw std::bad_alloc();
    return nullptr;
  }

  void LinearAllocator::deallocate(void* ptr, std::size_t)
  {
#ifdef BF_MEMORY_DEBUG_WIPE_MEMORY
    checkPointer(ptr);
#endif
  }

  char* LinearAllocator::currentBlock() const
  {
    return begin() + m_MemoryOffset;
  }

  void LinearAllocatorSavePoint::save(LinearAllocator& allocator)
  {
    m_Allocator = &allocator;
    m_OldOffset = allocator.m_MemoryOffset;
  }

  void LinearAllocatorSavePoint::restore()
  {
    assert(m_Allocator && "must be called after a call to save.");

    m_Allocator->m_MemoryOffset = m_OldOffset;
    m_Allocator                 = nullptr;
  }
}  // namespace bf

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2019-2021 Shareef Abdoul-Raheem

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
