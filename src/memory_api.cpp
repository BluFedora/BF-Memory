#include "bf/memory/memory_api.hpp"

#include "bf/memory/crt_allocator.hpp"

#include <cstring>  // memset
#include <limits>   // numeric_limits

//-------------------------------------------------------------------------------------//
// Utilities Interface
//-------------------------------------------------------------------------------------//

#if BF_MEMORY_DEBUG_ASSERTIONS
bool bfMemAssertImpl(const bool expr, const char* const expr_str, const char* const filename, const int line_number, const char* const assert_msg)
{
  if (!expr)
  {
    std::fprintf(stderr, "Memory[%s:%i] Assertion '%s' failed, %s.\n", filename, line_number, expr_str, assert_msg);
    std::abort();
  }

  return expr;
}
#endif

#define bfCast(e, T) ((T)(e))

void* bfAlignUpPointer(const void* const ptr, const size_t required_alignment)
{
  bfMemAssert(required_alignment > 0 && (required_alignment & (required_alignment - 1)) == 0, "The alignment must be a non-zero power of two.");

  const size_t required_alignment_mask = required_alignment - 1;

  return bfCast(bfCast(ptr, uintptr_t) + required_alignment_mask & ~required_alignment_mask, void*);
}

/*
  Good read on the various implementations and performance characteristics:
    [https://github.com/KabukiStarship/KabukiToolkit/wiki/Fastest-Method-to-Align-Pointers#21-proof-by-example]
    [https://stackoverflow.com/a/51585463]
*/
void* bfStdAlign(size_t alignment, size_t size, void** ptr, size_t* space)
{
  bfMemAssert(alignment > 0 && (alignment & (alignment - 1)) == 0, "The alignment must be a non-zero power of two.");
  bfMemAssert(ptr, "Passed in pointer must not be null.");
  bfMemAssert(space, "Passed in space must not be null.");

  void* const     aligned_ptr = bfAlignUpPointer(*ptr, alignment);
  const uintptr_t offset      = bfCast(aligned_ptr, char*) - bfCast(*ptr, char*);

  if (*space >= (size + offset))
  {
    *ptr = aligned_ptr;
    *space -= offset;

    return aligned_ptr;
  }

  return NULL;
}

std::size_t bfMemAlignOffset(const void* const ptr, const std::size_t alignment)
{
  void* const aligned_ptr = bfAlignUpPointer(ptr, alignment);
  return bfCast(aligned_ptr, char*) - bfCast(ptr, char*);
}

#undef bfCast

void bfMemCopy(void* const dst, const void* const src, std::size_t num_bytes)
{
  std::memcpy(dst, src, num_bytes);
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

bf::AllocationResult bfMemAllocate(bf::IAllocator& self, const std::size_t size)
{
  if (size == 0u)
  {
    return bf::AllocationResult::Null();
  }

  const bf::AllocationResult ptr = self.alloc(&self, size);

  return bfMemDebugWipeMemory(ptr);
}

void bfMemDeallocate(bf::IAllocator& self, const bf::AllocationResult mem_block)
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

bf::AllocationResult bfMemAllocateAligned(bf::IAllocator& self, const std::size_t size, const std::size_t alignment)
{
  bfMemAssert(alignment <= std::numeric_limits<AlignmentHeader>::max(), "Alignment too large.");

  const std::size_t          allocation_size = alignedAllocationSize(size, alignment);
  const bf::AllocationResult allocation      = bfMemAllocate(self, allocation_size);

  if (allocation)
  {
    const std::uint8_t* const header_end  = static_cast<std::uint8_t*>(allocation.ptr) + sizeof(AlignmentHeader);
    std::uint8_t* const       data_start  = static_cast<std::uint8_t*>(bfAlignUpPointer(header_end, alignment));
    const AlignmentHeader     data_offset = AlignmentHeader(std::uintptr_t(data_start) - std::uintptr_t(allocation.ptr));
    data_start[-1]                        = data_offset;

    return bf::AllocationResult{data_start, allocation.num_bytes - data_offset};
  }

  return bf::AllocationResult::Null();
}

void bfMemDeallocateAligned(bf::IAllocator& self, const bf::AllocationResult mem_block, const std::size_t alignment)
{
  const AlignmentHeader offset           = alignedAllocationOffset(mem_block.ptr);
  const std::size_t     allocation_size  = alignedAllocationSize(mem_block.num_bytes, alignment);
  void* const           allocation_start = reinterpret_cast<void*>(std::uintptr_t(mem_block.ptr) - offset);

  bfMemDeallocate(self, bf::AllocationResult{allocation_start, allocation_size});
}
