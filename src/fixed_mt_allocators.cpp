#include "memory/fixed_mt_allocators.hpp"

#include "memory/alignment.hpp"  // AlignPointer

Memory::ConcurrentLinearAllocator::ConcurrentLinearAllocator(byte* const memory_block, const MemoryIndex memory_block_size) noexcept :
  m_MemoryBgn{memory_block},
  m_MemoryEnd{memory_block + memory_block_size},
  m_Current{memory_block}
{
}

AllocationResult Memory::ConcurrentLinearAllocator::Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo&) noexcept
{
  const MemoryIndex required_size = size + (alignment - 1);
  byte* const       ptr           = m_Current.fetch_add(required_size);
  byte* const       aligned_ptr   = static_cast<byte*>(AlignPointer(ptr, alignment));

  if ((aligned_ptr + size) <= m_MemoryEnd)
  {
    const byte* const ptr_end         = ptr + required_size;
    const byte* const clamped_ptr_end = ptr_end < m_MemoryEnd ? ptr_end : m_MemoryEnd;
    const MemoryIndex allocated_size  = clamped_ptr_end - aligned_ptr;

    return AllocationResult{aligned_ptr, allocated_size};
  }

  // Don't want to wrap around after many failed allocations.
  m_Current.store(m_MemoryEnd);
  return AllocationResult::Null();
}

void Memory::ConcurrentLinearAllocator::Deallocate(void* const /* ptr */, const MemoryIndex /* size */, const MemoryIndex /* alignment */) noexcept
{
  /* NO-OP */
}
