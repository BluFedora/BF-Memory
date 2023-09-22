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

static bf::CRTAllocator s_DefaultAllocator = {};

bf::IAllocator& bfMemAllocator(void)
{
  return s_DefaultAllocator;
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

AllocationResult bfMemAllocate(bf::IAllocator& self, const std::size_t size, const std::size_t alignment)
{
  bfMemAssert(alignment <= std::numeric_limits<AlignmentHeader>::max(), "Alignment too large.");

  const std::size_t      allocation_size = alignedAllocationSize(size, alignment);
  const AllocationResult allocation      = self.alloc(&self, allocation_size);

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

void bfMemDeallocate(bf::IAllocator& self, const AllocationResult mem_block, const std::size_t alignment)
{
  const AlignmentHeader offset           = alignedAllocationOffset(mem_block.ptr);
  const std::size_t     allocation_size  = alignedAllocationSize(mem_block.num_bytes, alignment);
  void* const           allocation_start = reinterpret_cast<void*>(std::uintptr_t(mem_block.ptr) - offset);

  self.dealloc(&self, AllocationResult{allocation_start, allocation_size});
}
