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

  enum class BoundCheckingPolicy : bool
  {
    UNCHECKED = false,
    CHECKED   = true,
  };

  enum class AllocationMarkPolicy : bool
  {
    UNMARKED,
    MARK,
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

  struct NoMemoryTracking
  {
    void TrackAllocate(const AllocationResult& allocation, const MemoryIndex alignment, const AllocationSourceInfo& source_info) const noexcept {}
    void TrackDeallocate(void* const ptr, const MemoryIndex size, MemoryIndex alignment) const noexcept {}
  };

  template<typename AllocationState,
           AllocationMarkPolicy MarkPolicy,
           BoundCheckingPolicy  BoundCheck,
           typename AllocationTrackingPolicy,
           typename LockPolicy>
  class MemoryManager : public AllocationState
    , private LockPolicy
    , private AllocationTrackingPolicy
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

      LockPolicy::Lock();

      const MemoryIndex total_size = guard_size + guard_size + size + guard_size;

      AllocationResult result = static_cast<AllocationState*>(this)->Allocate(total_size, alignment);

      if (result)
      {
        const MemoryIndex extra_bytes       = result.num_bytes - total_size;
        byte* const       bytes             = static_cast<byte*>(result.ptr);
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

        AllocationTrackingPolicy::TrackAllocate(result, alignment, source_info);

        result = AllocationResult(mark_bytes, user_memory_size);
      }

      LockPolicy::Unlock();

      return result;
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

        AllocationTrackingPolicy::TrackDeallocate(bytes, total_size, alignment);
        static_cast<AllocationState*>(this)->Deallocate(bytes, total_size, alignment);

        LockPolicy::Unlock();
      }
    }

    operator Allocator()
    {
      return Allocator{
       this,
       [](void* const                 allocator_state,
          const MemoryIndex           size,
          const MemoryIndex           alignment,
          const AllocationSourceInfo& source_info) -> AllocationResult {
           return static_cast<MemoryManager*>(allocator_state)->Allocate(size, alignment, source_info);
       },
       [](void* const allocator_state, void* const ptr, const MemoryIndex size, const MemoryIndex alignment) -> void {
         return static_cast<MemoryManager*>(allocator_state)->Deallocate(ptr, size, alignment);
       }
      };
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
