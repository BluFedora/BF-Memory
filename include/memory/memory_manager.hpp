/******************************************************************************/
/*!
 * @file   memory_manager.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *
 * @copyright Copyright (c) 2023 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_MANAGER_HPP
#define LIB_FOUNDATION_MEMORY_MANAGER_HPP

#include "assertion.hpp"    // bfMemAssert
#include "basic_types.hpp"  // AllocationResult, AllocationSourceInfo

namespace Memory
{
  static constexpr byte GuardBytePattern     = 0xAB;
  static constexpr byte AllocatedBytePattern = 0xCD;
  static constexpr byte FreeBytePattern      = 0xDD;

  enum class AllocationMarkPolicy : bool
  {
    UNMARKED = false,
    MARK     = true,
  };

  enum class BoundCheckingPolicy : bool
  {
    UNCHECKED = false,
    CHECKED   = true,
  };

  template<BoundCheckingPolicy BoundCheck>
  void GuardBytes(byte* const bytes, const MemoryIndex num_bytes) noexcept
  {
    if constexpr (BoundCheck == BoundCheckingPolicy::CHECKED)
    {
      for (MemoryIndex byte_index = 0u; byte_index < num_bytes; ++byte_index)
      {
        bytes[byte_index] = GuardBytePattern;
      }
    }
  }

  template<BoundCheckingPolicy BoundCheck>
  void CheckGuardBytes(const byte* const bytes, const MemoryIndex num_bytes) noexcept
  {
    if constexpr (BoundCheck == BoundCheckingPolicy::CHECKED)
    {
      for (MemoryIndex byte_index = 0u; byte_index < num_bytes; ++byte_index)
      {
        bfMemAssert(bytes[byte_index] == GuardBytePattern, "Memory guard byte check failure.");
      }
    }
  }

  template<AllocationMarkPolicy MarkPolicy>
  void MarkAllocatedBytes(byte* const bytes, const MemoryIndex num_bytes) noexcept
  {
    if constexpr (MarkPolicy == AllocationMarkPolicy::MARK)
    {
      for (MemoryIndex byte_index = 0u; byte_index < num_bytes; ++byte_index)
      {
        bytes[byte_index] = AllocatedBytePattern;
      }
    }
  }

  template<AllocationMarkPolicy MarkPolicy>
  void MarkFreedBytes(byte* const bytes, const MemoryIndex num_bytes) noexcept
  {
    if constexpr (MarkPolicy == AllocationMarkPolicy::MARK)
    {
      for (MemoryIndex byte_index = 0u; byte_index < num_bytes; ++byte_index)
      {
        bytes[byte_index] = FreeBytePattern;
      }
    }
  }

  struct NoLock
  {
    void Lock() const noexcept {}
    void Unlock() const noexcept {}
  };

  struct MemoryTrackAllocate
  {
    AllocationResult     allocation;
    MemoryIndex          requested_bytes;
    MemoryIndex          alignment;
    AllocationSourceInfo source_info;
  };

  struct MemoryTrackDeallocate
  {
    void*       ptr;
    MemoryIndex num_bytes;
    MemoryIndex alignment;
  };

  struct NoMemoryTracking
  {
    void TrackAllocate(const Memory::MemoryTrackAllocate& allocate_info) const noexcept { (void)allocate_info; }
    void TrackDeallocate(const Memory::MemoryTrackDeallocate& deallocate_info) const noexcept { (void)deallocate_info; }
  };

  template<typename AllocationState,
           AllocationMarkPolicy MarkPolicy,
           BoundCheckingPolicy  BoundCheck,
           typename AllocationTrackingPolicy,
           typename LockPolicy>
  class MemoryManager : public AllocationState
    , public LockPolicy
    , public AllocationTrackingPolicy
  {
   private:
    static constexpr bool MemoryMarkingEnabled = MarkPolicy != AllocationMarkPolicy::UNMARKED;
    static constexpr bool BoundCheckingEnabled = BoundCheck != BoundCheckingPolicy::UNCHECKED;

   public:
    template<typename... Args>
    MemoryManager(Args&&... state_args) :
      AllocationState(static_cast<decltype(state_args)&&>(state_args)...),
      LockPolicy(),
      AllocationTrackingPolicy()
    {
    }

    AllocationResult Allocate(const MemoryIndex size, MemoryIndex alignment, const AllocationSourceInfo& source_info)
    {
      if constexpr (BoundCheckingEnabled)
      {
        if (alignment < alignof(MemoryIndex))
        {
          alignment = alignof(MemoryIndex);
        }
      }

      const MemoryIndex guard_size = BoundCheckingEnabled ? alignment : 0u;
      const MemoryIndex total_size = guard_size + guard_size + size + guard_size;

      LockPolicy::Lock();

      const AllocationResult allocation = static_cast<AllocationState*>(this)->Allocate(total_size, alignment, source_info);

      if (allocation)
      {
        AllocationTrackingPolicy::TrackAllocate(MemoryTrackAllocate{allocation, total_size, alignment, source_info});
      }

      LockPolicy::Unlock();

      if (allocation)
      {
        const MemoryIndex extra_bytes       = allocation.num_bytes - total_size;
        byte* const       bytes             = static_cast<byte*>(allocation.ptr);
        const MemoryIndex user_memory_size  = size + extra_bytes;
        byte* const       size_header       = bytes;
        byte* const       guard_bytes_front = size_header + guard_size;
        byte* const       mark_bytes        = guard_bytes_front + guard_size;
        byte* const       guard_bytes_back  = mark_bytes + user_memory_size;

        if constexpr (BoundCheckingEnabled)
        {
          *reinterpret_cast<MemoryIndex*>(size_header) = user_memory_size;
        }

        GuardBytes<BoundCheck>(guard_bytes_front, guard_size);
        MarkAllocatedBytes<MarkPolicy>(mark_bytes, user_memory_size);
        GuardBytes<BoundCheck>(guard_bytes_back, guard_size);

        return AllocationResult(mark_bytes, user_memory_size);
      }

      return AllocationResult::Null();
    }

    void Deallocate(void* const ptr, const MemoryIndex size, MemoryIndex alignment)
    {
      if (ptr)
      {
        if constexpr (BoundCheckingEnabled)
        {
          if (alignment < alignof(MemoryIndex))
          {
            alignment = alignof(MemoryIndex);
          }
        }

        const MemoryIndex guard_size        = BoundCheckingEnabled ? alignment : 0u;
        const MemoryIndex total_size        = guard_size + guard_size + size + guard_size;
        byte* const       bytes             = static_cast<byte*>(ptr) - guard_size - guard_size;
        byte* const       size_header       = bytes;
        byte* const       guard_bytes_front = size_header + guard_size;
        byte* const       mark_bytes        = guard_bytes_front + guard_size;

        LockPolicy::Lock();

        if constexpr (BoundCheckingEnabled)
        {
          const MemoryIndex user_memory_size = *reinterpret_cast<const MemoryIndex*>(size_header);
          byte* const       guard_bytes_back = mark_bytes + user_memory_size;

          CheckGuardBytes<BoundCheck>(guard_bytes_front, guard_size);
          CheckGuardBytes<BoundCheck>(guard_bytes_back, guard_size);
        }

        MarkFreedBytes<MarkPolicy>(mark_bytes, size);

        AllocationTrackingPolicy::TrackDeallocate(MemoryTrackDeallocate{bytes, total_size, alignment});
        static_cast<AllocationState*>(this)->Deallocate(bytes, total_size, alignment);

        LockPolicy::Unlock();
      }
    }
  };

}  // namespace Memory

#endif  // LIB_FOUNDATION_MEMORY_MANAGER_HPP

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2023 Shareef Abdoul-Raheem

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
