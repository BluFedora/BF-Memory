/******************************************************************************/
/*!
 * @file   allocation.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Main allocation interface.
 *
 * @copyright Copyright (c) 2023-2024 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_ALLOCATION_HPP
#define LIB_FOUNDATION_MEMORY_ALLOCATION_HPP

#include "basic_types.hpp"  // AllocationResult, MemoryIndex, AllocationSourceInfo, MemoryMakeAllocationSourceInfo

#include <new>      // placement new
#include <utility>  // forward

namespace Memory
{
  /*!
   * @brief
   *   Describes the way memory should be initialized.
   */
  enum class ArrayConstruct
  {
    UNINITIALIZE,       //!< Memory is left alone in uninitialized state.
    DEFAULT_CONSTRUCT,  //!< Will default construct the memory to type `T`, if `T` is trivial then same as UNINITIALIZE.
    VALUE_CONSTRUCT,    //!< Will default construct the memory to its default value, typically zeroed for trivial types.
  };

  template<typename T>
  constexpr void DefaultConstructRange(T* const range_bgn, const T* const range_end)
  {
    for (T* element = range_bgn; element != range_end; ++element)
    {
      new (element) T;
    }
  }

  template<typename T>
  constexpr void ValueConstructRange(T* const range_bgn, const T* const range_end)
  {
    for (T* element = range_bgn; element != range_end; ++element)
    {
      new (element) T();
    }
  }

  template<typename T, typename... Args>
  T* Construct(void* const memory, Args&&... args)
  {
    return memory ? ::new (memory) T(std::forward<Args>(args)...) : nullptr;
  }

  template<typename T>
  constexpr void Destruct(T* const ptr)
  {
    ptr->~T();
  }

  template<typename T>
  constexpr void DestructAs(void* const ptr)
  {
    Destruct(static_cast<T*>(ptr));
  }

  template<typename T>
  constexpr void DestructRange(T* const range_bgn, const T* const range_end)
  {
    for (T* element = range_bgn; element != range_end; ++element)
    {
      Destruct(element);
    }
  }

}  // namespace Memory

//-------------------------------------------------------------------------------------//
// Base Allocation API: The functions that all other allocation functions are built on.
//-------------------------------------------------------------------------------------//

/*!
 * @brief
 *   Allocates memory from \p allocator, returns a block of memory with a minimum size of \p size and an alignment of \p alignment.
 *
 * @param allocator
 *   The allocator to request memory from.
 *
 * @param size
 *   The minimum number of bytes the returned block should contain.
 *
 * @param alignment
 *   The desired minimum alignment of the beginning of the block of memory.
 *
 * @param source_info
 *   Optional information on where the allocation came from.
 *
 * @return
 *    A pointer size pair of the usable block of memory, the size may be larger than \p size.
 *    AllocationResult::Null - on failed allocation.
 *
 * @see bfMemDeallocate
 */
template<typename AllocatorConcept>
AllocationResult bfMemAllocate(AllocatorConcept&& allocator, const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& source_info)
{
  if (size != 0)
  {
    return allocator.Allocate(size, alignment, source_info);
  }

  return AllocationResult::Null();
}
#define bfMemAllocate(allocator, size, alignment) (bfMemAllocate)((allocator), (size), (alignment), MemoryMakeAllocationSourceInfo())

/*!
 * @brief
 *   Returns memory allocated from bfMemAllocate back to \p allocator.
 *
 * @tparam AllocatorConcept
 *   Type of the allocator that follows the Allocator interface.
 *
 * @param allocator
 *   The allocator to return memory to.
 *
 * @param ptr
 *   The block of memory to free.
 *
 * @param size
 *   The size passed into bfMemAllocate.
 *
 * @param alignment
 *   The alignment passed into bfMemAllocate.
 *
 * @see bfMemAllocate
 */
template<typename AllocatorConcept>
void bfMemDeallocate(AllocatorConcept&& allocator, void* const ptr, const MemoryIndex size, const MemoryIndex alignment)
{
  return allocator.Deallocate(ptr, size, alignment);
}

//-------------------------------------------------------------------------------------//
// Single Object API: Calls constructor and destructors on the allocated memory.
//-------------------------------------------------------------------------------------//

/*!
 * @brief
 *   Allocates a single object of type \p T.
 *
 * @tparam T
 *   The type of object to allocate.
 *
 * @tparam ...Args
 *   The constructor arguments types to pass to T.
 *
 * @param allocator
 *   The allocator to request memory from.
 *
 * @param ...args
 *   The constructor arguments to pass to T.
 *
 * @return
 *   On Success: Returns a new object T.
 *   On Failure: nullptr.
 */
template<typename T, typename AllocatorConcept, typename... Args>
T* bfMemAllocateObject(AllocatorConcept&& allocator, Args&&... args)
{
  const AllocationResult mem_block = bfMemAllocate(allocator, sizeof(T), alignof(T));

  return Memory::Construct<T>(mem_block.ptr, std::forward<Args>(args)...);
}

/*!
 * @brief
 *   Deallocates a single object of type \p T.
 *
 * @tparam T
 *   The type of object.
 *
 * @param allocator
 *   The allocator to return memory to.
 *
 * @param ptr
 */
template<typename T, typename AllocatorConcept>
void bfMemDeallocateObject(AllocatorConcept&& allocator, T* const ptr)
{
  if (ptr)
  {
    ptr->~T();
    bfMemDeallocate(allocator, ptr, sizeof(T), alignof(T));
  }
}

//-------------------------------------------------------------------------------------//
// Array API:
//-------------------------------------------------------------------------------------//

/*!
 * @brief
 *   Initializes a block of memory using the policy from \p init.
 *   The block of memory should be uninitialized.
 *
 * @tparam T
 *   The type of array.
 *
 * @tparam init
 *   The policy to used to initialize the bytes.
 *
 * @param mem_block
 *   The memory block ot initialize treated as an array of \p T.
 *
 * @param num_elements
 *   The number of elements in the array.
 *
 * @return
 *   The type casted block of memory into it's correct array type.
 */
template<typename T, Memory::ArrayConstruct init>
T* bfMemArrayConstruct(const AllocationResult mem_block, const MemoryIndex num_elements);

/*!
 * @brief
 *   Allocates an array of type \p T of length \p num_elements.
 *
 * @tparam T
 *   The type of array.
 *
 * @tparam init
 *   Initialization policy to apply to the array.
 *
 * @param allocator
 *   The allocator to request memory from.
 *
 * @param num_elements
 *   The number of elements in the array.
 *
 * @param alignment
 *   Alignment of the first element in the array.
 *
 * @param source_info
 *   Auto filled in source info data.
 *
 * @return
 *   On Success: An array of \p num_elements length.
 *   On Failure: nullptr
 *
 * @see Memory::ArrayConstruct
 */
template<typename T, Memory::ArrayConstruct init = Memory::ArrayConstruct::UNINITIALIZE, typename AllocatorConcept>
T* bfMemAllocateArray(AllocatorConcept&& allocator, const MemoryIndex num_elements, const MemoryIndex alignment = alignof(T), const AllocationSourceInfo& source_info = MemoryMakeAllocationSourceInfoDefaultArg())
{
  const AllocationResult mem_block = (bfMemAllocate)(allocator, sizeof(T) * num_elements, alignment, source_info);

  return bfMemArrayConstruct<T, init>(mem_block, num_elements);
}

/*!
 * @brief
 *   Free memory allocated from bfMemAllocateArray.
 *
 * @tparam T
 *   The type of array.
 *
 * @tparam destroy
 *   Destruction policy.
 *
 * @param allocator
 *   The allocator to return memory to.
 *
 * @param array
 *   Pointer to the first element of the array.
 *
 * @param num_elements
 *   The number of elements in the array.
 *
 * @param alignment
 *   Alignment of the first element in the array.
 *
 * @see bfMemAllocateArray
 */
template<typename T, typename AllocatorConcept>
void bfMemDeallocateArray(AllocatorConcept&& allocator, T* const array, const MemoryIndex num_elements, const MemoryIndex alignment = alignof(T))
{
  if (array && num_elements)
  {
    bfMemDeallocate(allocator, array, sizeof(T) * num_elements, alignment);
  }
}

//-------------------------------------------------------------------------------------//
// Templated Function Implementations
//-------------------------------------------------------------------------------------//

template<typename T, Memory::ArrayConstruct init>
T* bfMemArrayConstruct(const AllocationResult mem_block, const MemoryIndex num_elements)
{
  T* const typed_array = static_cast<T*>(mem_block.ptr);

  if (typed_array)
  {
    if constexpr (init == Memory::ArrayConstruct::DEFAULT_CONSTRUCT)
    {
      Memory::DefaultConstructRange(typed_array, typed_array + num_elements);
    }
    else if constexpr (init == Memory::ArrayConstruct::VALUE_CONSTRUCT)
    {
      Memory::ValueConstructRange(typed_array, typed_array + num_elements);
    }
  }

  return typed_array;
}

#endif  // LIB_FOUNDATION_MEMORY_ALLOCATION_HPP

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
