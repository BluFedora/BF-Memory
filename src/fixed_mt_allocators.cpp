#include "memory/fixed_mt_allocators.hpp"

#include "memory/alignment.hpp"  // AlignPointer
#include "memory/allocation.hpp"

Memory::MT_LinearAllocator::MT_LinearAllocator(byte* const memory_block, const MemoryIndex memory_block_size) noexcept :
  m_MemoryBgn{memory_block},
  m_MemoryEnd{memory_block + memory_block_size},
  m_Current{memory_block}
{
}

AllocationResult Memory::MT_LinearAllocator::Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo&) noexcept
{
  const MemoryIndex required_size   = size + (alignment - 1);
  byte* const       ptr             = m_Current.fetch_add(required_size);
  byte* const       aligned_ptr     = static_cast<byte*>(AlignPointer(ptr, alignment));
  const byte* const aligned_ptr_end = aligned_ptr + size;

  if (aligned_ptr_end <= m_MemoryEnd)
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

void Memory::MT_LinearAllocator::Deallocate(void* const /* ptr */, const MemoryIndex /* size */, const MemoryIndex /* alignment */) noexcept
{
  /* NO-OP */
}

// MT_PoolSource

static constexpr MemoryIndex Min(const MemoryIndex lhs, const MemoryIndex rhs) { return lhs < rhs ? lhs : rhs; }
static constexpr MemoryIndex Max(const MemoryIndex lhs, const MemoryIndex rhs) { return lhs > rhs ? lhs : rhs; }

struct Memory::MT_PoolSource::Slab
{
  std::atomic<MemoryIndex> next_index = 0u;
  byte*                    objects    = nullptr;
  Slab*                    next       = nullptr;
};

struct Memory::MT_PoolSource::FreeNode
{
  FreeNode* next = nullptr;
};

Memory::MT_PoolSource::MT_PoolSource(IPolymorphicAllocator& parent_allocator, const MemoryIndex object_size, const MemoryIndex object_align, const MemoryIndex slab_capacity) :
  m_ParentAllocator{parent_allocator},
  m_ObjectSizeAlign{Max(object_size, sizeof(FreeNode)), Max(object_align, alignof(FreeNode))},
  m_SlabCapacity{slab_capacity},
  m_SlabSizeAlign{sizeof(Slab), alignof(Slab)},
  m_ObjectOffset{m_SlabSizeAlign.Append(m_ObjectSizeAlign, m_SlabCapacity)},
  m_ActiveSlab{nullptr},
  m_AllSlabs{nullptr},
  m_GrowMutex{},
  m_FreeNodesHead{nullptr}
{
}

Memory::MT_PoolSource::~MT_PoolSource() noexcept
{
  Slab* slab = m_AllSlabs;

  while (slab != nullptr)
  {
    Slab* const next_slab = slab->next;

    Memory::Destruct(slab);
    MemDeallocate(m_ParentAllocator, slab, m_SlabSizeAlign.size, m_SlabSizeAlign.alignment);

    slab = next_slab;
  }
}

void Memory::MT_PoolSource::EnsurePoolHasItems(MT_Pool* const pool)
{
  const MemoryIndex batch_size = pool->m_BatchSize;

  // Fast Path is only having to souch this allocator every `batch_size` items.
  while (pool->m_BgnIndex == pool->m_EndIndex)
  {
    Slab* const active_slab = m_ActiveSlab.load(std::memory_order_acquire);

    // Try refill from active slab
    {
      if (active_slab != nullptr)
      {
        const MemoryIndex begin = active_slab->next_index.fetch_add(batch_size, std::memory_order_relaxed);

        if (begin < m_SlabCapacity)
        {
          const MemoryIndex raw_end     = begin + batch_size;
          const MemoryIndex clamped_end = Min(raw_end, m_SlabCapacity);

          pool->m_Slab     = active_slab;
          pool->m_BgnIndex = begin;
          pool->m_EndIndex = clamped_end;
          return;
        }
      }
    }
    // Slow Path
    {
      const std::lock_guard lock{m_GrowMutex};

      Slab* const current_slab = m_ActiveSlab.load(std::memory_order_acquire);

      if (current_slab != active_slab)
      {
        continue;  // Another thread beat us to grow.
      }

      // Append New Slab
      {
        void* const allocation = MemAllocate(m_ParentAllocator, m_SlabSizeAlign.size, m_SlabSizeAlign.alignment).ptr;

        MemAssert(allocation != nullptr, "Failed to allocate new slab.");

        Slab* const slab = Memory::Construct<Slab>(allocation);

        slab->objects = reinterpret_cast<byte*>(allocation) + m_ObjectOffset;
        slab->next    = m_AllSlabs;
        m_AllSlabs    = slab;

        m_ActiveSlab.store(slab, std::memory_order_release);
      }
    }
  }
}

void Memory::MT_PoolSource::PushFreeNode(FreeNode* const node)
{
  FreeNode* old_head = m_FreeNodesHead.load(std::memory_order_relaxed);

  do
  {
    node->next = old_head;

  } while (!m_FreeNodesHead.compare_exchange_weak(old_head, node, std::memory_order_release, std::memory_order_relaxed));
}

Memory::MT_PoolSource::FreeNode* Memory::MT_PoolSource::PopFreeNode()
{
  FreeNode* old_head = m_FreeNodesHead.load(std::memory_order_acquire);

  while (old_head != nullptr && !m_FreeNodesHead.compare_exchange_weak(old_head, old_head->next, std::memory_order_acquire, std::memory_order_relaxed))
  {
  }

  return old_head;
}

// MT_Pool

Memory::MT_Pool::MT_Pool(MT_PoolSource* const source_pool, const MemoryIndex batch_size) :
  m_Source{source_pool},
  m_BatchSize{batch_size},
  m_Slab{nullptr},
  m_BgnIndex{0},
  m_EndIndex{0}
{
}

AllocationResult Memory::MT_Pool::Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& /* source_info */) noexcept
{
  const MemoryIndex object_size = m_Source->m_ObjectSizeAlign.size;

  MemAssert(size <= object_size, "Invalid object size");
  MemAssert(alignment <= m_Source->m_ObjectSizeAlign.alignment, "Invalid object alignment");

  auto* const free_node = m_Source->PopFreeNode();

  if (free_node != nullptr)
  {
    return AllocationResult{free_node, object_size};
  }
  else
  {
    m_Source->EnsurePoolHasItems(this);

    const MemoryIndex slot_index = m_BgnIndex++;
    byte* const       result     = m_Slab->objects + slot_index * object_size;

    return AllocationResult{result, object_size};
  }
}

void Memory::MT_Pool::Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept
{
  const MemoryIndex object_size = m_Source->m_ObjectSizeAlign.size;

  MemAssert(size <= object_size, "Invalid object size");
  MemAssert(alignment <= m_Source->m_ObjectSizeAlign.alignment, "Invalid object alignment");

  m_Source->PushFreeNode(reinterpret_cast<Memory::MT_PoolSource::FreeNode*>(ptr));
}
