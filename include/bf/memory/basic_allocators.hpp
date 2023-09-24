#ifndef BF_BASIC_ALLOCATORS_HPP
#define BF_BASIC_ALLOCATORS_HPP

#include "memory_api.hpp"  // IAllocator

#include "memory/fixed_st_allocators.hpp"

namespace bf
{
  using namespace Memory;

  //-------------------------------------------------------------------------------------//
  // Chunk Pool Allocator: Like PoolAllocator except that it grows in chunks.
  //-------------------------------------------------------------------------------------//

  template<std::size_t k_BlockSize, std::size_t k_NumBlocksInChunk>
  struct FixedChunkPoolAllocator
  {
    static_assert(k_NumBlocksInChunk > 0u, "Number of items in each chunk must be greater than 0.");

    static constexpr std::size_t BlockSize        = k_BlockSize < sizeof(PoolAllocatorBlock) ? sizeof(PoolAllocatorBlock) : k_BlockSize;
    static constexpr std::size_t NumBlocksInChunk = k_NumBlocksInChunk;
    static constexpr std::size_t ChunkMemSize     = BlockSize * k_NumBlocksInChunk;
    static constexpr std::size_t Alignment        = 16;

    struct Chunk
    {
      alignas(Alignment) byte buffer[ChunkMemSize];
      Chunk* next;
    };

    Chunk*              chunks;
    PoolAllocatorBlock* pool_head;
    IAllocator&         chunk_allocator;

    FixedChunkPoolAllocator(IAllocator& allocator) :
      chunk_allocator{allocator},
      chunks{nullptr},
      pool_head{nullptr}
    {
    }

    AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& /* source_info */) noexcept
    {
      bfMemAssert(size <= BlockSize, "This Allocator is made for Objects of size %u (not %u)!", unsigned(BlockSize), unsigned(size));

    pool_alloc:
      PoolAllocatorBlock* const block = pool_head;

      if (block != nullptr)
      {
        pool_head = block->next;

        return {reinterpret_cast<void*>(block), BlockSize};
      }

      Chunk* const new_chunk = bfMemAllocateObject<Chunk>(chunk_allocator);

      if (new_chunk)
      {
        new_chunk->next = std::exchange(chunks, new_chunk);

        pool_head = PoolAllocator::SetupPool(new_chunk->buffer, ChunkMemSize, BlockSize, Alignment).head;

        goto pool_alloc;
      }

      return AllocationResult::Null();
    }

    void Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept
    {
      bfMemAssert(size <= BlockSize, "That allocation did not come from this allocator.");

      PoolAllocatorBlock* const block = static_cast<PoolAllocatorBlock*>(ptr);

      block->next = std::exchange(pool_head, block);
    }

    void Clear()
    {
      Chunk* chunk = chunks;

      while (chunk)
      {
        Chunk* const next_chunk = chunk->next;

        bfMemDeallocateObject(chunk_allocator, chunk);

        chunk = next_chunk;
      }

      chunks    = nullptr;
      pool_head = nullptr;
    }

    ~FixedChunkPoolAllocator()
    {
      Clear();
    }

    operator IAllocator() { return IAllocator::BasicAllocatorConvert(*this); }
  };
}  // namespace bf

#endif /* LibFoundation_Memory_API_HPP */
