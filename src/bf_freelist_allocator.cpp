/******************************************************************************/
/*!
* @file   bf_freelist_allocator.cpp
* @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
* @brief
*   This allocator is a the most generic custom allocator with the heaviest
*   overhead. This has the largest header size of any allocator but can be
*   used as a direct replacement for "malloc/new" and "free/delete".
*
*   For allocating a first fit policy is used.
*   For deallocating an address-ordered pilicy is used.
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2020
*/
/******************************************************************************/
#include "bf/memory/bf_freelist_allocator.hpp"

#include <memory> /* memset */

#include <cassert> /* assert */

#define cast(o, T) reinterpret_cast<T>((o))

namespace bf
{
  FreeListAllocator::FreeListAllocator(char* memory_block, const std::size_t memory_block_size) :
    MemoryManager(memory_block, memory_block_size),
    m_FreeList(cast(memory_block, FreeListNode*)),
    m_UsedBytes(0)
  {
    m_FreeList->size = memory_block_size - header_size;
    m_FreeList->next = nullptr;
  }

  void* FreeListAllocator::allocate(std::size_t size)
  {
    FreeListNode* curr_node = m_FreeList;
    FreeListNode* prev_node = nullptr;

    while (curr_node)
    {
      // Block is not big enough so skip over it.
      if (curr_node->size < size)
      {
        prev_node = curr_node;
        curr_node = curr_node->next;
        continue;
      }

      const std::size_t block_size        = curr_node->size;
      const std::size_t space_after_alloc = block_size - size;
      FreeListNode*     block_next        = curr_node->next;

      if (space_after_alloc > sizeof(FreeListNode))
      {
        const std::size_t   offset_from_block = header_size + size;
        FreeListNode* const new_node          = cast(reinterpret_cast<char*>(curr_node) + offset_from_block, FreeListNode*);

        new_node->size = block_size - offset_from_block - header_size;
        new_node->next = block_next;

        curr_node->size = size;
        block_next      = new_node;
      }

      if (prev_node)
      {
        prev_node->next = block_next;
      }
      else
      {
        m_FreeList = block_next;
      }

      m_UsedBytes += curr_node->size;

      return cast(curr_node, char*) + header_size;
    }

    return nullptr;
  }

  void FreeListAllocator::deallocate(void* ptr, std::size_t num_bytes)
  {
    checkPointer(ptr);

    AllocationHeader* const header = reinterpret_cast<AllocationHeader*>(reinterpret_cast<char*>(ptr) - header_size);
    FreeListNode* const     node   = static_cast<FreeListNode*>(header);

    // The freelist allocator can give out blocks larger than the requested size.
    assert(num_bytes <= header->size);

#if BF_MEMORY_DEBUG_WIPE_MEMORY
    std::memset(ptr, BF_MEMORY_DEBUG_SIGNATURE, header->size);
#endif

    m_UsedBytes -= node->size;

    FreeListNode*     current    = m_FreeList;
    FreeListNode*     previous   = nullptr;
    const void* const node_begin = node->begin();
    const void* const node_end   = node->end();

    while (current)
    {
      //
      // if current is past the end of this current block.
      //                      or
      // The last block be passed by vacn be merged with us.
      //

      if (current->begin() >= node_end || (previous && previous->end() == node_begin))
      {
        break;
      }

      previous = current;
      current  = current->next;
    }

    //
    // Merge Node => Current
    //
    if (current && current->begin() == node_end)
    {
      if (previous)
      {
        previous->next = node;
      }
      else
      {
        m_FreeList = node;
      }

      node->size += (current->size + header_size);
      node->next = current->next;
    }
    else if (previous)
    {
      //
      // Merge Prev => Node
      //
      if (previous->end() == node_begin)
      {
        previous->size += (node->size + header_size);
      }
      else  // Add To Freelist Normally
      {
        node->next     = previous->next;
        previous->next = node;
      }
    }
    else  // Add To Freelist Normally
    {
      node->next = m_FreeList;
      m_FreeList = node;
    }
  }
}  // namespace bifrost

#undef cast

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
