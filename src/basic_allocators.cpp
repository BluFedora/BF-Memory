/******************************************************************************/
/*!
 * @file   basic_allocators.cpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @date   2022-09-17
 * @brief
 *   Contains basic single-threaded allocators that manage a single continuous
 *   block of memory.
 *
 * @copyright Copyright (c) 2022 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#include "bf/memory/basic_allocators.hpp"

#include <algorithm>  // max

namespace bf
{
  template<auto fn_ptr, typename T>
  static AllocationResult alloc_wrapper(IAllocator* const self, const std::size_t size)
  {
    return fn_ptr(static_cast<T*>(self), size);
  }

  template<auto fn_ptr, typename T>
  static void dealloc_wrapper(IAllocator* const self, const AllocationResult mem_block)
  {
    return fn_ptr(static_cast<T* const>(self), mem_block);
  }

  //-------------------------------------------------------------------------------------//
  // Linear Allocator
  //-------------------------------------------------------------------------------------//

  static AllocationResult linear_alloc(LinearAllocator* const self, const std::size_t size)
  {
    byte* const result = self->current;

    if ((result + size) <= self->memory_end)
    {
      self->current += size;
      return {result, size};
    }

    return AllocationResult::Null();
  }

  static void linear_dealloc(LinearAllocator* const self, const AllocationResult mem_block)
  {
    /* NO-OP */

    (void)self;
    (void)mem_block;
  }

  LinearAllocator::LinearAllocator(byte* const memory_block, std::size_t memory_block_size) :
    IAllocator(alloc_wrapper<&linear_alloc, LinearAllocator>, dealloc_wrapper<&linear_dealloc, LinearAllocator>),
    memory_bgn{memory_block},
    memory_end{memory_block + memory_block_size},
    current{memory_block}
  {
  }

  void LinearAllocator::clear()
  {
    current = memory_bgn;
  }

  bool LinearAllocator::canServiceAllocation(const std::size_t allocation_size, const std::size_t allocation_alignment) const
  {
    const void* const aligned_ptr = Memory::AlignPointer(current, allocation_alignment);

    return (reinterpret_cast<const byte*>(aligned_ptr) + allocation_size) <= memory_end;
  }

  void LinearAllocatorSavePoint::save(LinearAllocator& allocator)
  {
    this->allocator     = &allocator;
    this->restore_point = allocator.current;
  }

  void LinearAllocatorSavePoint::restore()
  {
    bfMemAssert(this->allocator, "Savepoint must be active before restore can be called.");
    this->allocator->current = this->restore_point;
    this->allocator          = nullptr;
  }

  //-------------------------------------------------------------------------------------//
  // Stack Allocator
  //-------------------------------------------------------------------------------------//

  using StackAllocatorHeader = std::size_t;  //<! Memory size by this block, used for error checking.

  static AllocationResult stack_alloc(StackAllocator* const self, const std::size_t size)
  {
    const std::size_t needed_memory = size + sizeof(StackAllocatorHeader);
    byte* const       memory_start  = self->stack_ptr;

    if (memory_start + needed_memory <= self->mem_block_end)
    {
      std::memcpy(memory_start, &size, sizeof(StackAllocatorHeader));
      self->stack_ptr += needed_memory;
      return {memory_start + sizeof(StackAllocatorHeader), size};
    }

    return AllocationResult::Null();
  }

  static void stack_dealloc(StackAllocator* const self, const AllocationResult mem_block)
  {
    byte* const          block_start = static_cast<byte*>(mem_block.ptr) - sizeof(StackAllocatorHeader);
    StackAllocatorHeader block_size;
    std::memcpy(&block_size, block_start, sizeof(StackAllocatorHeader));

    bfMemAssert(block_size == mem_block.num_bytes, "Incorrect number of bytes passed in.");
    bfMemAssert((static_cast<byte*>(mem_block.ptr) + block_size) == self->stack_ptr,
                "StackAllocator::dealloc : For this type of allocator you MUST deallocate in the reverse order of allocation.");

    self->stack_ptr = block_start;
  }

  StackAllocator::StackAllocator(byte* const memory_block, std::size_t memory_block_size) :
    IAllocator(alloc_wrapper<&stack_alloc, StackAllocator>, dealloc_wrapper<&stack_dealloc, StackAllocator>),
    stack_ptr{memory_block},
    mem_block_end{memory_block + memory_block_size}
  {
  }

  //-------------------------------------------------------------------------------------//
  // Pool Allocator
  //-------------------------------------------------------------------------------------//

  static AllocationResult pool_alloc(PoolAllocator* const self, const std::size_t size)
  {
    bfMemAssert(size <= self->block_size, "This Allocator is made for Objects of a certain size!");

    PoolAllocatorBlock* const block = self->pool_head;

    if (block != nullptr)
    {
      self->pool_head = block->next;

      return {reinterpret_cast<void*>(block), self->block_size};
    }

    return AllocationResult::Null();
  }

  static void pool_dealloc(PoolAllocator* const self, const AllocationResult mem_block)
  {
    bfMemAssert(mem_block.num_bytes <= self->block_size, "That allocation did not come from this allocator.");

    PoolAllocatorBlock* const block = static_cast<PoolAllocatorBlock*>(mem_block.ptr);

    block->next = std::exchange(self->pool_head, block);
  }

  PoolAllocator::PoolAllocator(byte* const memory_block, std::size_t memory_block_size, std::size_t pool_block_size) :
    IAllocator(alloc_wrapper<&pool_alloc, PoolAllocator>, dealloc_wrapper<&pool_dealloc, PoolAllocator>),
    memory_bgn{memory_block},
    pool_head{nullptr},
    block_size{std::max(pool_block_size, sizeof(PoolAllocatorBlock))},
    num_elements{memory_block_size / block_size}
  {
    reset();
  }

  std::size_t PoolAllocator::indexOf(const void* ptr) const
  {
    bfMemAssert(ptr < reinterpret_cast<const byte*>(memory_bgn) + block_size * num_elements, "");
    return (reinterpret_cast<const byte*>(ptr) - memory_bgn) / block_size;
  }

  void* PoolAllocator::fromIndex(std::size_t index)
  {
    bfMemAssert(index < num_elements, "Invalid index");

    return memory_bgn + block_size * index;
  }

  void PoolAllocator::reset()
  {
    pool_head = setupFreelist(memory_bgn, block_size, num_elements);
  }

  PoolAllocatorBlock* PoolAllocator::setupFreelist(byte* const memory_bgn, std::size_t block_size, std::size_t num_elements)
  {
    bfMemAssert(block_size >= sizeof(PoolAllocatorBlock), "Each block must be atlease PoolAllocatorBlock in size.");

    PoolAllocatorBlock* result;

    if (num_elements)
    {
      result = reinterpret_cast<PoolAllocatorBlock*>(memory_bgn);

      PoolAllocatorBlock* header = result;

      for (std::size_t i = 0; i < num_elements - 1; ++i)
      {
        header->next = reinterpret_cast<PoolAllocatorBlock*>(reinterpret_cast<byte*>(header) + block_size);
        header       = header->next;
      }

      header->next = nullptr;
    }
    else
    {
      result = nullptr;
    }

    return result;
  }

  //-------------------------------------------------------------------------------------//
  // Freelist Allocator
  //-------------------------------------------------------------------------------------//

  struct AllocationHeader
  {
    std::size_t size;  //!< size does not include the size of the header itself, rather it is the size of the writable region of memory.
  };

  struct FreeListNode : public AllocationHeader
  {
    //
    // When size is used here it includes the memory taken up by FreeListNode::next.
    //

    FreeListNode* next;  //!< Next free block.

    unsigned char* begin() const { return reinterpret_cast<unsigned char*>(const_cast<FreeListNode*>(this)); }
    unsigned char* end() const { return begin() + sizeof(AllocationHeader) + size; }
  };

  static AllocationResult freelist_alloc(FreeListAllocator* const self, const std::size_t size)
  {
    FreeListNode* prev_node = nullptr;
    FreeListNode* curr_node = self->freelist;

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
        const std::size_t   offset_from_block = sizeof(AllocationHeader) + size;
        FreeListNode* const new_node          = reinterpret_cast<FreeListNode*>(reinterpret_cast<char*>(curr_node) + offset_from_block);

        new_node->size = block_size - offset_from_block - sizeof(AllocationHeader);
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
        self->freelist = block_next;
      }

      return {reinterpret_cast<char*>(curr_node) + sizeof(AllocationHeader), curr_node->size};
    }

    return AllocationResult::Null();
  }

  static void freelist_dealloc(FreeListAllocator* const self, const AllocationResult mem_block)
  {
    AllocationHeader* const header = reinterpret_cast<AllocationHeader*>(reinterpret_cast<char*>(mem_block.ptr) - sizeof(AllocationHeader));
    FreeListNode* const     node   = static_cast<FreeListNode*>(header);

    bfMemAssert(mem_block.num_bytes <= header->size, "Invalid number of bytes passed in.,");

    FreeListNode*     previous   = nullptr;
    FreeListNode*     current    = self->freelist;
    const void* const node_begin = node->begin();
    const void* const node_end   = node->end();

    while (current)
    {
      //
      // if current is past the end of this current block.
      //                      or
      // The last block we passed by can be merged with us.
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
        self->freelist = node;
      }

      node->size += (current->size + sizeof(AllocationHeader));
      node->next = current->next;
    }
    else if (previous)
    {
      //
      // Merge Prev => Node
      //
      if (previous->end() == node_begin)
      {
        previous->size += (node->size + sizeof(AllocationHeader));
      }
      else  // Add To Freelist
      {
        node->next = std::exchange(previous->next, node);
      }
    }
    else  // Add To Freelist
    {
      node->next = std::exchange(self->freelist, node);
    }
  }

  FreeListAllocator::FreeListAllocator(byte* const memory_block, std::size_t memory_block_size) :
    IAllocator(alloc_wrapper<&freelist_alloc, FreeListAllocator>, dealloc_wrapper<&freelist_dealloc, FreeListAllocator>),
    freelist{reinterpret_cast<FreeListNode*>(memory_block)}
  {
    freelist->size = memory_block_size - sizeof(AllocationHeader);
    freelist->next = nullptr;
  }
}  // namespace bf

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2022 Shareef Abdoul-Raheem

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
