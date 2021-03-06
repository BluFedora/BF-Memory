/******************************************************************************/
/*!
* @file   bf_stack_allocator.cpp
* @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
* @brief
*   This allocator is a designed for allocations where you can guarantee
*   deallocation is in a LIFO (Last in First Out) order in return you get
*   some speed.
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2021
*/
/******************************************************************************/
#include "bf/memory/bf_stack_allocator.hpp"

#include <cassert> /* assert */
#include <memory>  /* align  */

namespace bf
{
  StackAllocator::StackAllocator(char* const memory_block, const std::size_t memory_size) :
    MemoryManager(memory_block, memory_size),
    m_StackPtr(memory_block),
    m_MemoryLeft(memory_size)
  {
  }

  void* StackAllocator::allocate(const std::size_t size)
  {
    const std::size_t requested_memory = size + sizeof(StackHeader);
    std::size_t       block_size       = m_MemoryLeft;
    void*             memory_start     = m_StackPtr;

    if (requested_memory <= m_MemoryLeft && std::align(alignof(StackHeader), requested_memory, memory_start, block_size))
    {
      StackHeader* header = reinterpret_cast<StackHeader*>(memory_start);
      header->block_size  = requested_memory;
      header->align_size  = (m_MemoryLeft - block_size);

      const auto full_size = (header->block_size + header->align_size);

      m_MemoryLeft -= full_size;
      m_StackPtr += full_size;

      return reinterpret_cast<char*>(header) + sizeof(StackHeader);
    }

    return nullptr;
  }

  void StackAllocator::deallocate(void* ptr, std::size_t num_bytes)
  {
    checkPointer(ptr);

    StackHeader* header      = reinterpret_cast<StackHeader*>(static_cast<char*>(ptr) - sizeof(StackHeader));
    const auto   full_size   = (header->block_size + header->align_size);
    const auto   block_start = reinterpret_cast<char*>(header) - header->align_size;

    assert(header->block_size == num_bytes && "Incorrect number of bytes passed in.");
    assert((block_start + full_size) == m_StackPtr &&
           "StackAllocator::dealloc : For this type of allocator you MUST deallocate in the reverse order of allocation.");

    m_StackPtr -= full_size;
    m_MemoryLeft += full_size;

#if BF_MEMORY_DEBUG_WIPE_MEMORY
    std::memset(block_start, BF_MEMORY_DEBUG_SIGNATURE, full_size);
#endif
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
