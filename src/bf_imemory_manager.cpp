/******************************************************************************/
/*!
* @file   bf_imemory_manager.cpp
* @author Shareef Abdoul-Raheem (http://sblufedora.github.io/)
* @brief
*  Outlines a basic interface for the various types of memory managers.
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2021
*/
/******************************************************************************/
#include "bf/memory/bf_imemory_manager.hpp"

#include "bf/memory/bf_memory_utils.h" /* bfAlignUpPointer */

#include <cstdint>

namespace bf
{
#define bfCastByte(arr) ((unsigned char*)(arr))

  static std::uint8_t grabOffset(const void* self)
  {
    return static_cast<const std::uint8_t*>(self)[-1];
  }

  static std::size_t alignedAllocationSize(std::size_t header_size, std::size_t size, std::size_t alignment)
  {
    return header_size + sizeof(std::uint8_t) + size + (alignment - 1);
  }

  void* IMemoryManager::allocateAligned(std::size_t size, std::size_t alignment)
  {
    return allocateAligned(0, size, alignment);
  }

  void IMemoryManager::deallocateAligned(void* ptr, std::size_t size, std::size_t alignment)
  {
    deallocateAligned(0, ptr, size, alignment);
  }

  void* IMemoryManager::allocateAligned(std::size_t header_size, std::size_t size, std::size_t alignment)
  {
    if (size == 0)
    {
      return nullptr;
    }

    const std::size_t allocation_size = alignedAllocationSize(header_size, size, alignment);
    void* const       allocation      = allocate(allocation_size);

    if (allocation)
    {
      void*              data_start = bfAlignUpPointer(bfCastByte(allocation) + header_size, alignment);
      const std::uint8_t new_offset = std::uint8_t(std::uintptr_t(data_start) - std::uintptr_t(allocation) - header_size);

#if BF_MEMORY_DEBUG_WIPE_MEMORY
      std::memset(
       static_cast<std::uint8_t*>(allocation) + header_size,
       BF_MEMORY_DEBUG_SIGNATURE,
       new_offset);
#endif

      static_cast<std::uint8_t*>(data_start)[-1] = new_offset;

      return data_start;
    }

    return nullptr;
  }

  void* IMemoryManager::grabHeader(std::size_t header_size, void* ptr)
  {
    return bfCastByte(ptr) - grabOffset(ptr) - header_size;
  }

  void IMemoryManager::deallocateAligned(std::size_t header_size, void* ptr, std::size_t size, std::size_t alignment)
  {
    deallocate(grabHeader(header_size, ptr), alignedAllocationSize(header_size, size, alignment));
  }

  MemoryManager::MemoryManager(char* memory_block, const std::size_t memory_block_size) :
    IMemoryManager(),
    m_MemoryBlockBegin(memory_block),
    m_MemoryBlockEnd(memory_block + memory_block_size)
  {
#if BF_MEMORY_DEBUG_WIPE_MEMORY
    std::memset(begin(), BF_MEMORY_DEBUG_SIGNATURE, memory_block_size);
#endif
  }

  void MemoryManager::checkPointer(const void* ptr) const
  {
    if (ptr < begin() || ptr > end())
    {
      throw std::exception(/* "This pointer is not within this pool. "*/);
    }
  }
}  // namespace bifrost

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
