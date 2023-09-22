/******************************************************************************/
/*!
 * @file   basic_types.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Defines the basic types shared though out the library.
 *
 * @copyright Copyright (c) 2023 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_BASIC_TYPES_HPP
#define LIB_FOUNDATION_MEMORY_BASIC_TYPES_HPP

#ifndef BF_MEMORY_ALLOCATION_INFO
#define BF_MEMORY_ALLOCATION_INFO 1
#endif

using MemoryIndex = decltype(sizeof(int));  //!<

using byte = unsigned char;  //!< Type to represent a single byte of memory.

/*!
 * @brief
 *   The result of an allocation from an IAllocator.
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
  AllocationSourceInfo { __FILE__, __FUNCTION__, __LINE__ }
#else
#define MemoryMakeAllocationSourceInfo() \
  AllocationSourceInfo {}
#endif

/*!
 * @brief
 *   Function used to allocate memory.
 *
 * @param allocator_state
 *   The allocator to request a memory block from.
 *
 * @param size
 *   The number of bytes requested to allocate.
 *
 * @param alignment
 *   Required alignment of the returned block.
 *
 * @return
 *    On Success: An AllocationResult with a pointer to the block of memory and number of bytes
 *                available (could be greater than \p size).
 *    On Failure: An AllocationResult with a nullptr and num_bytes == 0u;
 */
using AllocateFn = AllocationResult (*)(
 void* const                 allocator_state,
 const MemoryIndex           size,
 const MemoryIndex           alignment,
 const AllocationSourceInfo& source_info);

/*!
 * @brief
 *   Function used to free memory allocated from `MemAllocFn`.
 *
 * @param allocator_state
 *   The allocator to return the memory block to.
 *
 * @param ptr
 *   Allocated region to free.
 *
 * @param size
 *   Must be some number between the requested number of bytes and the number of returned bytes.
 *
 * @param alignment
 *   Must be the same alignment passed into the allocation function.
 */
using DeallocateFn = void (*)(void* const allocator_state, void* const ptr, const MemoryIndex size, const MemoryIndex alignment);

struct Allocator
{
  AllocateFn   allocate;
  DeallocateFn deallocate;
  void*        state;

  Allocator()                                = default;
  Allocator(const Allocator& rhs)            = default;
  Allocator(Allocator&& rhs)                 = default;
  Allocator& operator=(const Allocator& rhs) = default;
  Allocator& operator=(Allocator&& rhs)      = default;

  Allocator(void* const        state,
            const AllocateFn   allocate_fn,
            const DeallocateFn deallocate_fn) :
    allocate{allocate_fn},
    deallocate{deallocate_fn},
    state{state}
  {
  }
};

#endif  // LIB_FOUNDATION_MEMORY_BASIC_TYPES_HPP

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
