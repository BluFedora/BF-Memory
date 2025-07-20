#include "memory/growing_st_allocators.hpp"

#include "memory/fixed_st_allocators.hpp"  // PoolAllocator, PoolAllocatorBlock

#include "memory/memory_api.hpp"  // MemAllocate, MemDeallocate

Memory::GrowingPoolAllocator::GrowingPoolAllocator(
 IPolymorphicAllocator& parent_allocator,
 const MemoryIndex      block_size,
 const MemoryIndex      block_alignment,
 const MemoryIndex      num_blocks_per_chunk) noexcept :
  m_ParentAllocator{parent_allocator},
  m_BlockSize{block_size < sizeof(PoolAllocatorBlock) ? sizeof(PoolAllocatorBlock) : block_size},
  m_Alignment{block_alignment < alignof(ChunkFooter) ? alignof(ChunkFooter) : block_alignment},
  m_ChunkMemSize{AlignSize(m_BlockSize * num_blocks_per_chunk, alignof(ChunkFooter))},
  m_Chunks{nullptr},
  m_PoolHead{nullptr}
{
  MemAssert(block_size > 0, "Block size must be greater than 0.");
  MemAssert(num_blocks_per_chunk > 0, "Num blocks per chunk must be greater than 0.");
}

void Memory::GrowingPoolAllocator::Clear() noexcept
{
  ChunkFooter* chunk = m_Chunks;

  m_PoolHead = nullptr;

  while (chunk)
  {
    byte* const chunk_bytes = reinterpret_cast<byte*>(chunk) - m_ChunkMemSize;

    const PoolAllocatorSetupResult chunk_pool_setup = PoolAllocator::SetupPool(chunk_bytes, m_ChunkMemSize, m_BlockSize, m_Alignment);

    if (chunk_pool_setup.num_elements)
    {
      chunk_pool_setup.tail->next = m_PoolHead;
      m_PoolHead                  = chunk_pool_setup.head;
    }

    chunk = chunk->next;
  }
}

void Memory::GrowingPoolAllocator::FreeMemory() noexcept
{
  ChunkFooter* chunk = m_Chunks;

  m_Chunks   = nullptr;
  m_PoolHead = nullptr;

  while (chunk)
  {
    ChunkFooter* const next_chunk = chunk->next;

    byte* const chunk_bytes = reinterpret_cast<byte*>(chunk) - m_ChunkMemSize;

    MemDeallocate(m_ParentAllocator, chunk_bytes, m_ChunkMemSize + sizeof(ChunkFooter), m_Alignment);

    chunk = next_chunk;
  }
}

AllocationResult Memory::GrowingPoolAllocator::Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& source_info) noexcept
{
  MemAssert(size <= m_BlockSize, "This Allocator is made for Objects of size %zu (not %zu)!", m_BlockSize, size);
  MemAssert(alignment <= m_Alignment, "This Allocator is made for Objects of alignment %zu (not %zu)!", m_Alignment, alignment);

pool_alloc:
  PoolAllocatorBlock* const block = m_PoolHead;

  if (block != nullptr)
  {
    m_PoolHead = block->next;

    return AllocationResult{reinterpret_cast<void*>(block), m_BlockSize};
  }

  const AllocationResult new_chunk_memory = MemAllocate(m_ParentAllocator, m_ChunkMemSize + sizeof(ChunkFooter), m_Alignment, source_info);

  if (new_chunk_memory)
  {
    byte* const        chunk_bytes = static_cast<byte*>(new_chunk_memory.ptr);
    ChunkFooter* const new_chunk   = reinterpret_cast<ChunkFooter*>(chunk_bytes + m_ChunkMemSize);

    new_chunk->next = m_Chunks;
    m_Chunks        = new_chunk;

    const PoolAllocatorSetupResult chunk_pool_setup = PoolAllocator::SetupPool(chunk_bytes, m_ChunkMemSize, m_BlockSize, m_Alignment);

    if (chunk_pool_setup.head)
    {
      m_PoolHead = chunk_pool_setup.head;
      goto pool_alloc;
    }
  }

  return AllocationResult::Null();
}

void Memory::GrowingPoolAllocator::Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept
{
  MemAssert(size <= m_BlockSize, "That allocation did not come from this allocator (bad size).");
  MemAssert(alignment <= m_Alignment, "That allocation did not come from this allocator (bad alignment).");

  PoolAllocatorBlock* const block = static_cast<PoolAllocatorBlock*>(ptr);

  block->next = m_PoolHead;
  m_PoolHead  = block;
}
