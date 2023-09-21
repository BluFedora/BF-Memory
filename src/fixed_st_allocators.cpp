#include "memory/fixed_st_allocators.hpp"

#include "memory/alignment.hpp"

// LinearAllocatorState //

Memory::LinearAllocatorState::LinearAllocatorState(byte* const memory_block, const MemoryIndex memory_block_size) :
  m_MemoryBgn{memory_block},
  m_MemoryEnd{memory_block + memory_block_size},
  m_Current{memory_block}
{
}

bool Memory::LinearAllocatorState::CanServiceAllocation(const MemoryIndex size, const MemoryIndex alignment) const
{
  const void* const aligned_ptr = Memory::AlignPointer(m_Current, alignment);

  return (reinterpret_cast<const byte*>(aligned_ptr) + size) <= m_MemoryEnd;
}

AllocationResult Memory::LinearAllocatorState::Allocate(const MemoryIndex size, const MemoryIndex alignment)
{
  void* const aligned_ptr     = AlignPointer(m_Current, alignment);
  byte* const aligned_ptr_end = static_cast<byte*>(aligned_ptr) + size;

  if (aligned_ptr_end <= m_MemoryEnd)
  {
    m_Current = aligned_ptr_end;
    return AllocationResult(aligned_ptr, size);
  }

  return AllocationResult::Null();
}

void Memory::LinearAllocatorState::Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment)
{
  const byte* const ptr_end = static_cast<const byte*>(ptr) + size;

  if (ptr_end == m_Current)
  {
    m_Current = static_cast<byte*>(ptr);
  }
}
