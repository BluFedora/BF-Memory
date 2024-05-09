#include "memory/fixed_st_allocators.hpp"

#include "memory/alignment.hpp"

//-------------------------------------------------------------------------------------//
// Linear Allocator
//-------------------------------------------------------------------------------------//

Memory::LinearAllocator::LinearAllocator(byte* const memory_block, const MemoryIndex memory_block_size) noexcept :
  m_MemoryBgn{memory_block},
  m_MemoryEnd{memory_block + memory_block_size},
  m_Current{memory_block}
{
}

void Memory::LinearAllocator::Init(byte* const memory_block, const MemoryIndex memory_block_size)
{
  m_MemoryBgn = memory_block;
  m_MemoryEnd = memory_block + memory_block_size;
  m_Current   = memory_block;
}

bool Memory::LinearAllocator::CanServiceAllocation(const MemoryIndex size, const MemoryIndex alignment) const noexcept
{
  const void* const aligned_ptr = AlignPointer(m_Current, alignment);

  return (reinterpret_cast<const byte*>(aligned_ptr) + size) <= m_MemoryEnd;
}

AllocationResult Memory::LinearAllocator::Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo&) noexcept
{
  if (size != 0u)
  {
    void* const aligned_ptr     = AlignPointer(m_Current, alignment);
    byte* const aligned_ptr_end = static_cast<byte*>(aligned_ptr) + size;

    if (aligned_ptr_end <= m_MemoryEnd)
    {
      m_Current = aligned_ptr_end;
      return AllocationResult(aligned_ptr, size);
    }
  }

  return AllocationResult::Null();
}

void Memory::LinearAllocator::Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept
{
  (void)alignment;  // TODO(SR): Assert that the pointer is in the memory region.

  const byte* const ptr_end = static_cast<const byte*>(ptr) + size;

  if (ptr_end == m_Current)
  {
    m_Current = static_cast<byte*>(ptr);
  }
}

void Memory::LinearAllocatorSavePoint::Save(LinearAllocator& allocator) noexcept
{
  m_Allocator    = &allocator;
  m_RestorePoint = allocator.m_Current;
}

void Memory::LinearAllocatorSavePoint::Restore() noexcept
{
  bfMemAssert(m_Allocator != nullptr, "Savepoint must be active before restore can be called.");

  m_Allocator->m_Current = m_RestorePoint;
  m_Allocator            = nullptr;
}

//-------------------------------------------------------------------------------------//
// Stack Allocator
//-------------------------------------------------------------------------------------//

#include <cstring>  // memcpy

struct StackAllocatorHeader
{
  byte*       restore;
  MemoryIndex num_bytes;
};

namespace Stack
{
  static void WriteHeader(void* const dst, const StackAllocatorHeader& header)
  {
    std::memcpy(static_cast<byte*>(dst) + offsetof(StackAllocatorHeader, restore), &header.restore, sizeof(StackAllocatorHeader::restore));
    std::memcpy(static_cast<byte*>(dst) + offsetof(StackAllocatorHeader, num_bytes), &header.num_bytes, sizeof(StackAllocatorHeader::num_bytes));
  }

  static StackAllocatorHeader ReadHeader(const void* const src)
  {
    StackAllocatorHeader result;
    std::memcpy(&result.restore, static_cast<const byte*>(src) + offsetof(StackAllocatorHeader, restore), sizeof(StackAllocatorHeader::restore));
    std::memcpy(&result.num_bytes, static_cast<const byte*>(src) + offsetof(StackAllocatorHeader, num_bytes), sizeof(StackAllocatorHeader::num_bytes));

    return result;
  }
}  // namespace Stack

Memory::StackAllocator::StackAllocator(byte* const memory_block, MemoryIndex memory_block_size) noexcept :
  m_StackPtr{memory_block},
  m_MemoryEnd{memory_block + memory_block_size}
{
}

AllocationResult Memory::StackAllocator::Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo&) noexcept
{
  const std::size_t needed_memory = size + sizeof(StackAllocatorHeader);
  byte* const       restore_point = m_StackPtr;
  byte* const       aligned_ptr   = reinterpret_cast<byte*>(AlignPointer(restore_point + sizeof(StackAllocatorHeader), alignment));

  if ((aligned_ptr + needed_memory) <= m_MemoryEnd)
  {
    m_StackPtr += needed_memory;

    Stack::WriteHeader(aligned_ptr - sizeof(StackAllocatorHeader), StackAllocatorHeader{restore_point, needed_memory});

    return AllocationResult{aligned_ptr, size};
  }

  return AllocationResult::Null();
}

void Memory::StackAllocator::Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept
{
  (void)alignment;

  const StackAllocatorHeader header = Stack::ReadHeader(reinterpret_cast<byte*>(ptr) - sizeof(StackAllocatorHeader));

  bfMemAssert(header.num_bytes == size, "Incorrect number of bytes passed in.");
  bfMemAssert(header.restore < m_MemoryEnd, "Invalid pointer passed in (Restore point invalid).");
  bfMemAssert(header.restore < m_StackPtr, "Invalid pointer passed in (Stack pointer should be ahead of restore point).");

  m_StackPtr = header.restore;
}

//-------------------------------------------------------------------------------------//
// Pool Allocator
//-------------------------------------------------------------------------------------//

Memory::PoolAllocator::PoolAllocator(byte* const memory_block, const MemoryIndex memory_size, const MemoryIndex block_size, const MemoryIndex alignment) noexcept :
  m_MemoryBgn{memory_block},
  m_MemoryEnd{memory_block + memory_size},
  m_BlockSize{block_size},
  m_Alignment{alignment},
  m_PoolHead{nullptr},
  m_NumElements{0u}
{
  Reset();
}

void Memory::PoolAllocator::Reset() noexcept
{
  const PoolAllocatorSetupResult setup = SetupPool(m_MemoryBgn, m_MemoryEnd - m_MemoryBgn, m_BlockSize, m_Alignment);

  m_PoolHead    = setup.head;
  m_NumElements = setup.num_elements;
}

MemoryIndex Memory::PoolAllocator::IndexOf(const void* ptr) const noexcept
{
  bfMemAssert(ptr < reinterpret_cast<const byte*>(m_MemoryBgn) + m_BlockSize * m_NumElements, "Pointer does not belong to this pool.");
  return (reinterpret_cast<const byte*>(ptr) - m_MemoryBgn) / m_BlockSize;
}

void* Memory::PoolAllocator::FromIndex(const MemoryIndex index) noexcept
{
  bfMemAssert(index < m_NumElements, "Invalid index");

  return m_MemoryBgn + m_BlockSize * index;
}

AllocationResult Memory::PoolAllocator::Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo&) noexcept
{
  bfMemAssert(size <= m_BlockSize, "This Allocator is made for Objects of a certain size!");
  bfMemAssert(alignment <= m_Alignment, "This Allocator is made for Objects of a certain alignment!");

  PoolAllocatorBlock* const block = m_PoolHead;

  if (block != nullptr)
  {
    m_PoolHead = block->next;

    return AllocationResult{block, m_BlockSize};
  }

  return AllocationResult::Null();
}

void Memory::PoolAllocator::Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept
{
  bfMemAssert(size <= m_BlockSize, "That allocation did not come from this allocator (bad size).");
  bfMemAssert(alignment <= m_Alignment, "That allocation did not come from this allocator (bad alignment).");
  bfMemAssert(m_MemoryBgn <= ptr && ptr < m_MemoryEnd, "That allocation did not come from this allocator.");
  bfMemAssert(IsPointerAligned(ptr, alignment), "All memory from this allocator is aligned.");

  PoolAllocatorBlock* const block = static_cast<PoolAllocatorBlock*>(ptr);

  block->next = m_PoolHead;
  m_PoolHead  = block;
}

Memory::PoolAllocatorSetupResult Memory::PoolAllocator::SetupPool(byte* const memory_block, const MemoryIndex memory_size, const MemoryIndex block_size, const MemoryIndex alignment) noexcept
{
  bfMemAssert(block_size >= sizeof(PoolAllocatorBlock), "Each block must be at least PoolAllocatorBlock in size.");

  const MemoryIndex aligned_block_size = AlignSize(block_size, alignment);
  byte* const       base_address       = static_cast<byte*>(AlignPointer(memory_block, alignment));
  const byte* const memory_end         = memory_block + memory_size;
  const MemoryIndex memory_available   = (memory_end - base_address);
  const MemoryIndex num_elements       = memory_available / aligned_block_size;

  if (num_elements != 0u)
  {
    const auto NodeAt = [base_address, aligned_block_size](const MemoryIndex index) -> PoolAllocatorBlock* {
      return reinterpret_cast<PoolAllocatorBlock*>(base_address + aligned_block_size * index);
    };

    const MemoryIndex last_index = num_elements - 1u;

    for (MemoryIndex index = 0; index < last_index; ++index)
    {
      NodeAt(index + 0u)->next = NodeAt(index + 1u);
    }
    NodeAt(last_index)->next = nullptr;

    return {NodeAt(0u), NodeAt(last_index), num_elements};
  }

  return {nullptr, nullptr, 0u};
}

//-------------------------------------------------------------------------------------//
// Freelist Allocator
//-------------------------------------------------------------------------------------//

#include <cstdint>  // uint8_t
#include <limits>   // numeric_limits

using AlignmentHeader = std::uint8_t;

namespace FreeList
{
  static constexpr AlignmentHeader MaxAlignment = std::numeric_limits<AlignmentHeader>::max();

  static std::size_t AlignedAllocationSize(std::size_t size, std::size_t alignment)
  {
    return size != 0u ? sizeof(AlignmentHeader) + size + (alignment - 1) : 0u;
  }

  static AlignmentHeader AlignedAllocationOffset(const void* ptr)
  {
    return static_cast<const std::uint8_t*>(ptr)[-1];
  }
}  // namespace FreeList

AllocationResult Memory::FreeListAllocator::Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo&) noexcept
{
  bfMemAssert(alignment <= FreeList::MaxAlignment, "Alignment too large.");

  const MemoryIndex      allocation_size = FreeList::AlignedAllocationSize(size, alignment);
  const AllocationResult allocation      = AllocateInternal(allocation_size);

  if (allocation)
  {
    const std::uint8_t* const header_end  = static_cast<std::uint8_t*>(allocation.ptr) + sizeof(AlignmentHeader);
    std::uint8_t* const       data_start  = static_cast<std::uint8_t*>(Memory::AlignPointer(header_end, alignment));
    const AlignmentHeader     data_offset = AlignmentHeader(std::uintptr_t(data_start) - std::uintptr_t(allocation.ptr));
    data_start[-1]                        = data_offset;

    return AllocationResult{data_start, allocation.num_bytes - data_offset};
  }

  return AllocationResult::Null();
}

void Memory::FreeListAllocator::Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept
{
  const AlignmentHeader offset           = FreeList::AlignedAllocationOffset(ptr);
  const std::size_t     allocation_size  = FreeList::AlignedAllocationSize(size, alignment);
  void* const           allocation_start = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(ptr) - offset);

  DeallocateInternal(allocation_start, allocation_size);
}

struct AllocationHeader
{
  MemoryIndex size;  //!< size does not include the size of the header itself, rather it is the size of the writable region of memory.
};

struct Memory::FreeListNode : public AllocationHeader
{
  //
  // When size is used here it includes the memory taken up by FreeListNode::next.
  //

  FreeListNode* next;  //!< Next free block.

  unsigned char* begin() const { return reinterpret_cast<unsigned char*>(const_cast<FreeListNode*>(this)); }
  unsigned char* end() const { return begin() + sizeof(AllocationHeader) + size; }
};

AllocationResult Memory::FreeListAllocator::AllocateInternal(const MemoryIndex size) noexcept
{
  FreeListNode* prev_node = nullptr;
  FreeListNode* curr_node = m_Freelist;

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
      m_Freelist = block_next;
    }

    return {reinterpret_cast<char*>(curr_node) + sizeof(AllocationHeader), curr_node->size};
  }

  return AllocationResult::Null();
}

void Memory::FreeListAllocator::DeallocateInternal(void* const ptr, const MemoryIndex size) noexcept
{
  AllocationHeader* const header = reinterpret_cast<AllocationHeader*>(reinterpret_cast<char*>(ptr) - sizeof(AllocationHeader));
  FreeListNode* const     node   = static_cast<FreeListNode*>(header);

  bfMemAssert(size <= header->size, "Invalid number of bytes passed in.,");

  FreeListNode*     previous   = nullptr;
  FreeListNode*     current    = m_Freelist;
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
      m_Freelist = node;
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
      node->next     = previous->next;
      previous->next = node;
    }
  }
  else  // Add To Freelist
  {
    node->next = m_Freelist;
    m_Freelist = node;
  }
}
