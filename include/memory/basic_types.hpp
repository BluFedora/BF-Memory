/******************************************************************************/
/*!
 * @file   basic_types.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Defines the basic types shared though out the library.
 *
 * @copyright Copyright (c) 2023-2024 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_BASIC_TYPES_HPP
#define LIB_FOUNDATION_MEMORY_BASIC_TYPES_HPP

#ifndef BF_MEMORY_ALLOCATION_INFO
#define BF_MEMORY_ALLOCATION_INFO 1
#endif

using MemoryIndex = decltype(sizeof(int));  //!<

using byte = unsigned char;  //!< Type to represent a single byte of memory.

#define bfKilobytes(n) static_cast<MemoryIndex>((n) * 1024)
#define bfMegabytes(n) static_cast<MemoryIndex>(bfKilobytes(n) * 1024)
#define bfGigabytes(n) static_cast<MemoryIndex>(bfMegabytes(n) * 1024)

constexpr bool WillMulOverflow(const MemoryIndex lhs, const MemoryIndex rhs)
{
  return (rhs > 1) ? (lhs > (MemoryIndex(-1) / rhs)) : false;
}

namespace Memory
{
  constexpr MemoryIndex AlignSize(const MemoryIndex size, const MemoryIndex alignment) noexcept;
}

/*!
 * @brief
 *   Helper type for calculating the size and alignment requirements of a single
 *   buffer that will have a heterogeneous array types contained in it.
 *
 *   Example Usage:
 *   ```cpp
 *   constexpr MemoryIndex simd_sse_alignment = 16u;
 *
 *   MemoryRequirements mem_reqs = {};
 *   const MemoryIndex buffer0_offset = mem_reqs.Append<int>();
 *   const MemoryIndex buffer1_offset = mem_reqs.Append<char>(1999);
 *   const MemoryIndex buffer2_offset = mem_reqs.Append<float>(1, simd_sse_alignment);
 *   ```
 */
struct MemoryRequirements
{
  MemoryIndex size      = 0u;
  MemoryIndex alignment = alignof(byte);

  constexpr MemoryRequirements() noexcept = default;

  constexpr MemoryRequirements(const MemoryIndex size, const MemoryIndex alignment) noexcept :
    size{size},
    alignment{alignment}
  {
  }

  // Setup API

  template<typename T>
  constexpr MemoryIndex Append(const MemoryIndex element_count = 1u, const MemoryIndex element_alignment = alignof(T)) noexcept
  {
    return Append(sizeof(T), element_count, element_alignment);
  }

  // Returns the offset in the buffer that this element(s) would be located at.
  // constexpr MemoryIndex Append(const MemoryIndex element_size, const MemoryIndex element_count, const MemoryIndex element_alignment) noexcept;

  constexpr MemoryIndex Append(const MemoryIndex element_size, const MemoryIndex element_count, const MemoryIndex element_alignment) noexcept
  {
    if (WillMulOverflow(element_size, element_count))
    {
      return size;
    }

    const MemoryIndex allocation_size = element_size * element_count;

    if (!allocation_size)
    {
      return size;
    }

    const MemoryIndex allocation_offset = Memory::AlignSize(size, element_alignment);

    size      = allocation_offset + allocation_size;
    alignment = (element_alignment > alignment) ? element_alignment : alignment;

    return allocation_offset;
  }

  constexpr MemoryIndex Append(const MemoryRequirements mem_reqs, const MemoryIndex element_count = 1u) noexcept
  {
    return Append(mem_reqs.size, element_count, mem_reqs.alignment);
  }

  // Only needed if you want to have multiple `MemoryRequirements` sized buffers consecutively in memory.
  // Call after you are done `MemoryRequirements::Append`ing.
  void AlignSizeToAlignment() noexcept;

  // Query API

  // Checks size and alignment
  bool IsBufferValid(const void* const buffer, const MemoryIndex buffer_size) const noexcept;

  // Only checks alignment.
  bool IsBufferValid(const void* const buffer) const noexcept;

  // Allocation API

  template<typename T, typename BufferType>
  static T* Alloc(BufferType** buffer, const void* const buffer_end, const MemoryIndex element_count = 1u, const MemoryIndex element_alignment = alignof(T)) noexcept
  {
    return static_cast<T*>(Alloc(buffer, buffer_end, sizeof(T), element_count, element_alignment));
  }

  template<typename BufferType>
  static void* Alloc(BufferType** buffer, const void* const buffer_end, const MemoryIndex element_size, const MemoryIndex element_count, const MemoryIndex element_alignment) noexcept
  {
    return Alloc(reinterpret_cast<void**>(buffer), buffer_end, element_size, element_count, element_alignment);
  }

  static void* Alloc(void** buffer, const void* const buffer_end, const MemoryIndex element_size, const MemoryIndex element_count, const MemoryIndex element_alignment) noexcept;
};

template<typename TagType>
struct TaggedMemoryRequirements : public MemoryRequirements
{
  using MemoryRequirements::MemoryRequirements;
};

/*!
 * @brief
 *   The result of an allocation from an IPolymorphicAllocator.
 */
struct AllocationResult
{
  /*!
   * @brief
   *   Returns an empty AllocationResult.
   */
  static constexpr AllocationResult Null()
  {
    return {nullptr, 0u};
  }

  void*       ptr;        //!< Pointer to the starting address of the allocated block.
  MemoryIndex num_bytes;  //!< Number of bytes allocated, could be greater than the amount of memory requested.

  AllocationResult() = default;
  constexpr AllocationResult(void* const ptr, const MemoryIndex num_bytes) :
    ptr{ptr},
    num_bytes{num_bytes}
  {
  }

  constexpr operator bool() const { return ptr != nullptr && num_bytes != 0u; }
  constexpr operator void*() const { return ptr; }
};

struct AllocationSourceInfo
{
#if BF_MEMORY_ALLOCATION_INFO
  const char* file;
  const char* function;
  int         line;
#endif
};

#if BF_MEMORY_ALLOCATION_INFO
#define MemoryMakeAllocationSourceInfo() \
  AllocationSourceInfo { __FILE__, __func__, __LINE__ }
#define MemoryMakeAllocationSourceInfoDefaultArg() \
  AllocationSourceInfo { __builtin_FILE(), __builtin_FUNCTION(), __builtin_LINE() }
#else
#define MemoryMakeAllocationSourceInfo() \
  AllocationSourceInfo {}
#define MemoryMakeAllocationSourceInfoDefaultArg() \
  AllocationSourceInfo {}
#endif

enum class AllocationOp : MemoryIndex
{
  DO_ALLOCATE   = 0,
  DO_DEALLOCATE = 1,
};

/*!
 * @brief
 *   ptr is a AllocationSourceInfo* ptr when op == DO_ALLOCATE.
 */
using PolymorphicAllocatorFn = AllocationResult (*)(MemoryIndex size, MemoryIndex alignment, void* const ptr, const AllocationOp op, void* const self);

/*!
 * @brief
 *   Type erased polymorphic allocator, unlike AllocatorView this in meant for long term storage.
 */
struct IPolymorphicAllocator
{
  PolymorphicAllocatorFn allocate_fn;

  IPolymorphicAllocator(const IPolymorphicAllocator& rhs)            = delete;
  IPolymorphicAllocator(IPolymorphicAllocator&& rhs)                 = delete;
  IPolymorphicAllocator& operator=(const IPolymorphicAllocator& rhs) = delete;
  IPolymorphicAllocator& operator=(IPolymorphicAllocator&& rhs)      = delete;

  IPolymorphicAllocator(const PolymorphicAllocatorFn allocate_fn) :
    allocate_fn{allocate_fn}
  {
  }

  AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& source_info) noexcept
  {
    return allocate_fn(size, alignment, const_cast<void*>(static_cast<const void*>(&source_info)), AllocationOp::DO_ALLOCATE, this);
  }

  void Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept
  {
    allocate_fn(size, alignment, ptr, AllocationOp::DO_DEALLOCATE, this);
  }
};

/*!
 * @brief
 *   Type erased polymorphic view for an allocator to be used as a parameter type.
 */
struct AllocatorView
{
  void*                  self;
  PolymorphicAllocatorFn allocate_fn;

  AllocatorView(IPolymorphicAllocator& allocator) :
    self{&allocator},
    allocate_fn{allocator.allocate_fn}
  {
  }

  template<typename AllocatorConcept>
  AllocatorView(AllocatorConcept& allocator) :
    self{&allocator},
    allocate_fn{+[](MemoryIndex size, MemoryIndex alignment, void* const ptr, const AllocationOp op, void* const self) -> AllocationResult {
      AllocatorConcept& typed_self = *static_cast<AllocatorConcept*>(self);

      if (op == AllocationOp::DO_ALLOCATE)
      {
        return typed_self.Allocate(size, alignment, *static_cast<const AllocationSourceInfo*>(ptr));
      }
      else
      {
        typed_self.Deallocate(ptr, size, alignment);
      }

      return AllocationResult::Null();
    }}
  {
  }

  AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& source_info) const noexcept
  {
    return allocate_fn(size, alignment, const_cast<void*>(static_cast<const void*>(&source_info)), AllocationOp::DO_ALLOCATE, self);
  }

  void Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) const noexcept
  {
    allocate_fn(size, alignment, ptr, AllocationOp::DO_DEALLOCATE, self);
  }
};

/*!
 * @brief
 *   Helper class for allowing both polymorphic and static interface.
 *
 * @tparam AllocatorConcept
 *   The type of allocator this will be implemented with.
 */
template<typename BaseAllocator>
// clang-format off
struct PolymorphicAllocator : public IPolymorphicAllocator, public BaseAllocator
// clang-format on
{
  template<typename... Args>
  PolymorphicAllocator(Args&&... args) :
    IPolymorphicAllocator(+[](MemoryIndex size, MemoryIndex alignment, void* const ptr, const AllocationOp op, void* const self) -> AllocationResult {
      // Polymorphic Interface

      PolymorphicAllocator<BaseAllocator>& typed_self = *static_cast<PolymorphicAllocator<BaseAllocator>*>(static_cast<IPolymorphicAllocator*>(self));

      if (op == AllocationOp::DO_ALLOCATE)
      {
        return typed_self.Allocate(size, alignment, *static_cast<const AllocationSourceInfo*>(ptr));
      }
      else
      {
        typed_self.Deallocate(ptr, size, alignment);
      }

      return AllocationResult::Null();
    }),
    BaseAllocator{static_cast<decltype(args)&&>(args)...}
  {
  }

  // Static Interface

  AllocationResult Allocate(const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& source_info) noexcept
  {
    return static_cast<BaseAllocator*>(this)->Allocate(size, alignment, source_info);
  }

  void Deallocate(void* const ptr, const MemoryIndex size, const MemoryIndex alignment) noexcept
  {
    return static_cast<BaseAllocator*>(this)->Deallocate(ptr, size, alignment);
  }
};

#endif  // LIB_FOUNDATION_MEMORY_BASIC_TYPES_HPP

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2023-2024 Shareef Abdoul-Raheem

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
