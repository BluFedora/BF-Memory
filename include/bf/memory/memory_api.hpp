/******************************************************************************/
/*!
 * @file   memory_api.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @date   2022-09-12
 * @brief
 *   Main API for the library.
 *
 *   Contains functions useful for low level memory manipulations.
 *
 *   Random Memory Notes:
 *     - new and malloc will always return memory aligned to alignof(std::max_align_t).
 *
 * @copyright Copyright (c) 2022-2023 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LibFoundation_Memory_API_HPP
#define LibFoundation_Memory_API_HPP

#include "memory/alignment.hpp"  // AlignPointer
#include "memory/assertion.hpp"  // bfMemAssert

#include <cstddef>  // size_t, max_align_t
#include <memory>   // uninitialized_default_construct, uninitialized_value_construct
#include <new>      // placement new
#include <utility>  // forward, move

namespace bf
{
  static constexpr std::size_t k_DefaultAlignment = alignof(std::max_align_t);

  template<typename T>
  struct is_trivially_relocatable : public std::true_type
  {
  };

  template<typename T>
  inline constexpr bool is_trivially_relocatable_v = is_trivially_relocatable<T>::value;

  /*!
   * @brief
   *   Describes the way memory should be initialized.
   */
  enum class MemArrayInit
  {
    UNINITIALIZE,       //!< Memory is left alone in uninitialized state.
    DEFAULT_CONSTRUCT,  //!< Will default construct the memory to type `T`, if `T` is trivial then same as UNINITIALIZE.
    VALUE_CONSTRUCT,    //!< Will default construct the memory to its default value, typically zeroed for trivial types.
  };

  /*!
   * @brief
   *   Describes the way memory should be destroyed.
   */
  enum class MemArrayDestroy
  {
    NONE,      //!< Memory is left alone.
    DESTRUCT,  //!< Will destruct the memory to type `T`.
  };
}  // namespace bf

//-------------------------------------------------------------------------------------//
// Utilities Interface
//-------------------------------------------------------------------------------------//

// TODO(SR): Add ZeroStruct and ByteCopyStruct.

/*!
 * @brief
 *   Same as std::memcpy.
 *   Byte by byte copy from \p src to \p dst.
 *
 * @param dst
 *   The destination of the byte copy, must be at least \p num_bytes in size.
 *
 * @param src
 *   The source of the byte copy, must be at least \p num_bytes in size.
 *
 * @param num_bytes
 *   The number of bytes to copy from \p src to \p dst.
 */
void bfMemCopy(void* const dst, const void* const src, std::size_t num_bytes);

void bfMemSet(void* const dst, const unsigned char value, std::size_t num_bytes);

template<typename T>
constexpr void bfMemDestruct(T* const ptr)
{
  std::destroy_at(ptr);
}

template<typename Iterator>
constexpr void bfMemDestructRange(const Iterator bgn, const Iterator end)
{
  using Traits     = std::iterator_traits<Iterator>;
  using value_type = typename Traits::value_type;

  if constexpr (!std::is_trivially_destructible_v<value_type>)
  {
    std::destroy(bgn, end);
  }
}

template<typename SrcIterator, typename DstIterator>
DstIterator bfMemUninitializedMove(SrcIterator src_bgn, SrcIterator src_end, DstIterator dst_bgn)
{
  return std::uninitialized_move(src_bgn, src_end, dst_bgn);
}

template<typename SrcIterator, typename DstIterator>
DstIterator bfMemUninitializedMoveRev(SrcIterator src_bgn, SrcIterator src_end, DstIterator dst_end)
{
  return bfMemUninitializedMove(
          std::make_reverse_iterator(src_end),
          std::make_reverse_iterator(src_bgn),
          std::make_reverse_iterator(dst_end))
   .base();
}

//-------------------------------------------------------------------------------------//
// Base Allocation API: The functions that all other allocation functions are built on.
//-------------------------------------------------------------------------------------//

/*!
 * @brief
 *   Allocates memory from \p self, returns a block of memory with a minimum size of \p size and an alignment of \p alignment.
 *
 * @param self
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
inline AllocationResult(bfMemAllocate)(AllocatorConcept&& allocator, const MemoryIndex size, const MemoryIndex alignment, const AllocationSourceInfo& source_info)
{
  using AllocatorConceptRaw = std::decay_t<AllocatorConcept>;

  if constexpr (std::is_same_v<AllocatorConceptRaw, IAllocator>)
  {
    return allocator.allocate(allocator.state, size, alignment, source_info);
  }
  else
  {
    return allocator.Allocate(size, alignment, source_info);
  }
}

#define bfMemAllocate(allocator, size, alignment) (bfMemAllocate)((allocator), (size), (alignment), MemoryMakeAllocationSourceInfo())

/*!
 * @brief
 *   Returns memory allocated from bfMemAllocateAligned back to \p self.
 *
 * @param self
 *   The allocator to return memory to.
 *
 * @param mem_block
 *   The block of memory to free. mem_block.size may be smaller than what was returned by bfMemAllocateAligned.
 *
 * @param alignment
 *   The alignment passed into bfMemAllocateAligned.
 *
 * @see bfMemAllocateAligned
 */
// TODO(SR): Dont take in AllocationResult.
inline void bfMemDeallocate(const IAllocator allocator, const AllocationResult mem_block, const std::size_t alignment)
{
  return allocator.deallocate(allocator.state, mem_block.ptr, mem_block.num_bytes, alignment);
}

// TODO(SR): Dont take in AllocationResult.
template<typename AllocatorConcept>
inline void bfMemDeallocate(AllocatorConcept&& allocator, const AllocationResult mem_block, const std::size_t alignment)
{
  return allocator.Deallocate(mem_block.ptr, mem_block.num_bytes, alignment);
}

template<typename AllocatorConcept>
inline void bfMemDeallocate(AllocatorConcept&& allocator, void* const ptr, const MemoryIndex size, const MemoryIndex alignment)
{
  return allocator.Deallocate(ptr, size, alignment);
}

//-------------------------------------------------------------------------------------//
// Single Object API: Aligned versions take up extra memory.
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
 * @param self
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
T* bfMemAllocateObject(AllocatorConcept&& allocator, Args&&... args);

/*!
 * @brief
 *   Deallocates a single object of type \p T.
 *
 * @tparam T
 *   The type of object.
 *
 * @param self
 *   The allocator to return memory to.
 *
 * @param ptr
 */
template<typename T, typename AllocatorConcept>
void bfMemDeallocateObject(AllocatorConcept&& allocator, T* const ptr);

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
template<typename T, bf::MemArrayInit init>
T* bfMemArrayConstruct(const AllocationResult mem_block, const std::size_t num_elements);

template<bf::MemArrayDestroy destroy, typename T>
void bfMemDestructArray(T* const array, const std::size_t num_elements);

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
 * @param self
 *   The allocator to request memory from.
 *
 * @param num_elements
 *   The number of elements in the array.
 *
 * @return
 *   On Success: An array of \p num_elements length.
 *   On Failure: nullptr
 *
 * @see bf::MemArrayInit
 */
template<typename T, bf::MemArrayInit init = bf::MemArrayInit::UNINITIALIZE, typename AllocatorConcept>
T* bfMemAllocateArray(AllocatorConcept&& allocator, const std::size_t num_elements, const std::size_t alignment = alignof(T), const AllocationSourceInfo& source_info = MemoryMakeAllocationSourceInfoDefaultArg())
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
 * @param self
 *   The allocator to return memory to.
 *
 * @param array
 * @param num_elements
 *   The number of elements in the array.
 *
 * @see bfMemAllocateArray
 */
template<bf::MemArrayDestroy destroy = bf::MemArrayDestroy::NONE, typename T>
void bfMemDeallocateArray(const IAllocator allocator, T* const array, const std::size_t num_elements, const std::size_t alignment = alignof(T));

//-------------------------------------------------------------------------------------//
// Templated Function Implementations
//-------------------------------------------------------------------------------------//

template<bf::MemArrayDestroy destroy, typename T>
void bfMemDeallocateArray(const IAllocator allocator, T* const array, const std::size_t num_elements, const std::size_t alignment)
{
  if (array && num_elements)
  {
    bfMemDestructArray<destroy>(array, num_elements);
    bfMemDeallocate(allocator, AllocationResult{array, sizeof(T) * num_elements}, alignment);
  }
}

//

template<typename T, bf::MemArrayInit init>
T* bfMemArrayConstruct(const AllocationResult mem_block, const std::size_t num_elements)
{
  T* const typed_array = static_cast<T*>(mem_block.ptr);

  if constexpr (init != bf::MemArrayInit::UNINITIALIZE)
  {
    if (typed_array)
    {
      if constexpr (init == bf::MemArrayInit::DEFAULT_CONSTRUCT)
      {
        std::uninitialized_default_construct(typed_array, typed_array + num_elements);
      }
      else if constexpr (init == bf::MemArrayInit::VALUE_CONSTRUCT)
      {
        std::uninitialized_value_construct(typed_array, typed_array + num_elements);
      }
    }
  }

  return typed_array;
}

template<bf::MemArrayDestroy destroy, typename T>
void bfMemDestructArray(T* const array, const std::size_t num_elements)
{
  if constexpr (destroy == bf::MemArrayDestroy::DESTRUCT)
  {
    bfMemDestructRange(array, array + num_elements);
  }
}

//

template<typename T, typename AllocatorConcept, typename... Args>
T* bfMemAllocateObject(AllocatorConcept&& allocator, Args&&... args)
{
  const AllocationResult mem_block = bfMemAllocate(allocator, sizeof(T), alignof(T));

  return mem_block ? new (mem_block.ptr) T(std::forward<Args>(args)...) : nullptr;
}

template<typename T, typename AllocatorConcept>
void bfMemDeallocateObject(AllocatorConcept&& allocator, T* const ptr)
{
  if (ptr)
  {
    std::destroy_at(ptr);
    bfMemDeallocate(allocator, AllocationResult{ptr, sizeof(T)}, alignof(T));
  }
}

#endif /* LibFoundation_Memory_API_HPP */

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2022-2023 Shareef Abdoul-Raheem

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
