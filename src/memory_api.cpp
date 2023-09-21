#include "bf/memory/memory_api.hpp"

#include "bf/memory/crt_allocator.hpp"

#include <cstdarg>  // va_list, va_start, va_end
#include <cstdio>   // vsnprintf, stderr,
#include <cstring>  // memset
#include <limits>   // numeric_limits

//-------------------------------------------------------------------------------------//
// Utilities Interface
//-------------------------------------------------------------------------------------//

void bfMemCopy(void* const dst, const void* const src, std::size_t num_bytes)
{
  std::memcpy(dst, src, num_bytes);
}

void bfMemSet(void* const dst, const unsigned char value, std::size_t num_bytes)
{
  std::memset(dst, value, num_bytes);
}

//-------------------------------------------------------------------------------------//
// Allocator Stack Interface: The stack is thread local.
//-------------------------------------------------------------------------------------//

bf::AllocatorScope::AllocatorScope(bf::IAllocator& new_allocator)
{
  bfMemAllocatorPush(new_allocator);
}

bf::AllocatorScope::~AllocatorScope()
{
  bfMemAllocatorPop();
}

namespace
{
  static bf::IAllocator*& CurrentAllocator()
  {
    static bf::CRTAllocator             s_DefaultAllocator = {};
    static thread_local bf::IAllocator* s_CurrentAllocator = &s_DefaultAllocator;

    return s_CurrentAllocator;
  }
}  // namespace

void bfMemAllocatorPush(bf::IAllocator& new_allocator)
{
  bfMemAssert(!new_allocator.parent, "Allocator cannot be pushed onto the stack multiple times.");

  new_allocator.parent = std::exchange(CurrentAllocator(), &new_allocator);
}

bf::IAllocator& bfMemAllocator(void)
{
  return *CurrentAllocator();
}

void bfMemAllocatorPop(void)
{
  bf::IAllocator*& current_allocator = CurrentAllocator();

  current_allocator = std::exchange(current_allocator->parent, nullptr);

  bfMemAssert(current_allocator, "Too many pops for each push.");
}

//-------------------------------------------------------------------------------------//
// Base Allocation Interface: Raw byte allocation, address has no alignment guarantees.
//-------------------------------------------------------------------------------------//

AllocationResult bfMemAllocate(bf::IAllocator& self, const std::size_t size)
{
  if (size != 0u)
  {
    return bfMemDebugWipeMemory(self.alloc(&self, size));
  }

  return AllocationResult::Null();
}

void bfMemDeallocate(bf::IAllocator& self, const AllocationResult mem_block)
{
  if (mem_block)
  {
    self.dealloc(&self, bfMemDebugWipeMemory(mem_block));
  }
}

//-------------------------------------------------------------------------------------//
// Aligned Allocation API: Takes up extra memory for header and alignment.
//-------------------------------------------------------------------------------------//

using AlignmentHeader = std::uint8_t;

static std::size_t alignedAllocationSize(std::size_t size, std::size_t alignment)
{
  return size != 0u ? sizeof(AlignmentHeader) + size + (alignment - 1) : 0u;
}
static AlignmentHeader alignedAllocationOffset(const void* ptr)
{
  return static_cast<const std::uint8_t*>(ptr)[-1];
}

AllocationResult bfMemAllocateAligned(bf::IAllocator& self, const std::size_t size, const std::size_t alignment)
{
  if (alignment <= self.default_min_alignment)
  {
    return bfMemAllocate(self, size);
  }
  else if (self.aligned_alloc)
  {
    return self.aligned_alloc(&self, size, alignment);
  }
  else
  {
    bfMemAssert(alignment <= std::numeric_limits<AlignmentHeader>::max(), "Alignment too large.");

    const std::size_t          allocation_size = alignedAllocationSize(size, alignment);
    const AllocationResult allocation      = bfMemAllocate(self, allocation_size);

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
}

void bfMemDeallocateAligned(bf::IAllocator& self, const AllocationResult mem_block, const std::size_t alignment)
{
  if (alignment <= self.default_min_alignment || self.aligned_alloc)
  {
    bfMemDeallocate(self, mem_block);
  }
  else
  {
    const AlignmentHeader offset           = alignedAllocationOffset(mem_block.ptr);
    const std::size_t     allocation_size  = alignedAllocationSize(mem_block.num_bytes, alignment);
    void* const           allocation_start = reinterpret_cast<void*>(std::uintptr_t(mem_block.ptr) - offset);

    bfMemDeallocate(self, AllocationResult{allocation_start, allocation_size});
  }
}
