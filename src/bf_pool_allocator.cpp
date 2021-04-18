/******************************************************************************/
/*!
* @file   bf_pool_allocator.cpp
* @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
* @brief
*   This allocator is a designed for static (known at compile time)
*   pools of objects. Features O(1) allocation and O(1) deletion.
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2021
*/
/******************************************************************************/
#include "bf/memory/bf_pool_allocator.hpp"

#include "bf/memory/bf_memory_utils.h" /* bfAlignUpSize */

#include <algorithm> /* max     */
#include <cassert>   /* assert  */
#include <cstdint>   /* uint8_t */

namespace bf
{
  PoolAllocatorImpl::PoolAllocatorImpl(char* memory_block, std::size_t memory_block_size, std::size_t sizeof_block, std::size_t alignof_block) :
    MemoryManager(memory_block, memory_block_size),
    m_PoolStart{nullptr},
    m_BlockSize{bfAlignUpSize(std::max(sizeof_block, sizeof(PoolHeader)), std::max(alignof_block, alignof(PoolHeader)))}
  {
    reset();
  }

  void* PoolAllocatorImpl::allocate(std::size_t size)
  {
    assert(size <= m_BlockSize && "This Allocator is made for Objects of a certain size!");

    PoolHeader* const header = m_PoolStart;

    if (header)
    {
      m_PoolStart = header->next;

      checkPointer(reinterpret_cast<void*>(header));

      return reinterpret_cast<void*>(header);
    }

    throw std::bad_alloc();

#if _MSC_VER
    __debugbreak();
#endif

    return nullptr;
  }

  void PoolAllocatorImpl::deallocate(void* ptr, std::size_t num_bytes)
  {
    checkPointer(ptr);

    PoolHeader* const header = static_cast<PoolHeader*>(ptr);

    assert(num_bytes == m_BlockSize);

#ifdef BF_MEMORY_DEBUG_WIPE_MEMORY
    std::memset(ptr, BF_MEMORY_DEBUG_SIGNATURE, m_BlockSize);
#endif

    header->next = m_PoolStart;
    m_PoolStart  = header;
  }

  std::size_t PoolAllocatorImpl::indexOf(const void* ptr) const
  {
    const std::size_t index = (std::size_t(ptr) - std::size_t(begin())) / m_BlockSize;

    checkPointer(ptr);

    return index;
  }

  void* PoolAllocatorImpl::fromIndex(std::size_t index)
  {
    void* const ptr = begin() + m_BlockSize * index;

    checkPointer(ptr);

    return ptr;
  }

  void PoolAllocatorImpl::reset()
  {
    m_PoolStart = reinterpret_cast<PoolHeader*>(begin());

    const std::size_t num_elements = capacity();
    PoolHeader*       header       = m_PoolStart;

    for (std::size_t i = 0; i < num_elements - 1; ++i)
    {
      header->next = reinterpret_cast<PoolHeader*>(reinterpret_cast<std::uint8_t*>(header) + m_BlockSize);
      header       = header->next;
    }

    header->next = nullptr;
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
