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
 * @copyright Copyright (c) 2022 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef BF_MEMORY_API_HPP
#define BF_MEMORY_API_HPP

#include <cstddef>  // size_t
#include <memory>   // uninitialized_default_construct, uninitialized_value_construct
#include <new>      // placement new
#include <utility>  // forward, move

#ifndef BF_MEMORY_DEBUG_WIPE_MEMORY
#define BF_MEMORY_DEBUG_WIPE_MEMORY 1  //!< Unitialized memory will be wiped to a known value, disable for shipping build.
#endif

#ifndef BF_MEMORY_DEBUG_ASSERTIONS
#define BF_MEMORY_DEBUG_ASSERTIONS 1  //!< Extra checks for function preconditions, disable for shipping build.
#endif

namespace bf
{
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
    static inline constexpr AllocationResult Null()
    {
      return {nullptr, 0u};
    }

    void*       ptr;        //!< Pointer to the starting address of the allocated block.
    std::size_t num_bytes;  //!< Number of bytes allocated, could be greater than the amount of memory requested.

    operator bool() const { return ptr != nullptr && num_bytes != 0u; }
    operator void*() const { return ptr; }
  };

  /*!
   * @brief
   *   Function used to allocate memory.
   *
   * @param self
   *   The allocator to request a memory block from.
   *
   * @param size
   *   The number of bytes requested to allocate.
   *
   * @return
   *    On Success: An AllocationResult with a pointer to the block of memory and number of bytes
   *                available (could be greater than \p size).
   *    On Failure: An AllocationResult with a nullptr and num_bytes == 0u;
   */
  using MemAllocFn = AllocationResult (*)(struct IAllocator* const self, const std::size_t size);

  /*!
   * @brief
   *   Function used to free memory allocated from `MemAllocFn`.
   *
   * @param self
   *   The allocator to request a memory block from.
   *
   * @param mem_block
   *   Allocated region to free, the AllocationResult::num_bytes must be some number
   *   between the requested number of bytes to the number of returned bytes.
   */
  using MemDeallocFn = void (*)(struct IAllocator* const self, const AllocationResult mem_block);

  /*!
   * @brief
   *   Interface for a memory manager.
   */
  struct IAllocator
  {
    const MemAllocFn   alloc;    //!< @copydoc MemAllocFn
    const MemDeallocFn dealloc;  //!< @copydoc MemDeallocFn
    IAllocator*        parent;   //!< private: parent allocator for the allocator stack.

   protected:
    //-------------------------------------------------------------------------------------//
    // Only Subclasses should be able to call the constructor and destructor.
    //-------------------------------------------------------------------------------------//

    IAllocator(MemAllocFn alloc, MemDeallocFn dealloc) :
      alloc{alloc},
      dealloc{dealloc},
      parent{nullptr}
    {
    }

    ~IAllocator() = default;

   public:
    //-------------------------------------------------------------------------------------//
    // Disallow Copies and Moves
    //-------------------------------------------------------------------------------------//

    IAllocator(const IAllocator& rhs)            = delete;
    IAllocator(IAllocator&& rhs)                 = delete;
    IAllocator& operator=(const IAllocator& rhs) = delete;
    IAllocator& operator=(IAllocator&& rhs)      = delete;
  };

  /*!
   * @brief
   *   Describes the way memory should be initialized.
   */
  enum class MemArrayInit
  {
    UNINITIALIZE,       //!< Memory is left alone in unitialized state.
    DEFAULT_CONSTRUCT,  //!< Will default construct the memory to type `T`, if `T` is trivial then it may be left uninitialized.
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

  /*!
   * @brief
   *   Usage:
   *   Declare a AllocatorScope on the stack to reassign the global allocator.
   *
   *   @code
   *   const AllocatorScope mem_ctx{my_new_allocator};
   *
   *   bfMemAllocator() // will now be my_new_allocator.
   *
   *   @endcode
   *
   * @attention
   *   Memory contexts are thread local.
   */
  struct AllocatorScope
  {
    /*!
     * @brief
     *   Calls bfMemAllocatorPush with \p new_allocator
     *
     * @param new_allocator
     *   The new allocator to be made global.
     */
    AllocatorScope(bf::IAllocator& new_allocator);

    /*!
     * @brief
     *   Calls bfMemAllocatorPop.
     */
    ~AllocatorScope();

    AllocatorScope(const AllocatorScope& rhs)            = delete;
    AllocatorScope(AllocatorScope&& rhs)                 = delete;
    AllocatorScope& operator=(const AllocatorScope& rhs) = delete;
    AllocatorScope& operator=(AllocatorScope&& rhs)      = delete;
  };

}  // namespace bf

//-------------------------------------------------------------------------------------//
// Utilities Interface
//-------------------------------------------------------------------------------------//

#if BF_MEMORY_DEBUG_WIPE_MEMORY
#define BF_MEMORY_DEBUG_UNINITIALIZED 0xCD

inline const bf::AllocationResult& bfMemDebugWipeMemory(const bf::AllocationResult& alloc)
{
  if (alloc.ptr)
  {
    std::memset(alloc.ptr, BF_MEMORY_DEBUG_UNINITIALIZED, alloc.num_bytes);
  }

  return alloc;
}

#else
#define bfMemDebugWipeMemory(alloc) alloc
#endif

#define bfKilobytes(n) ((n)*1024)
#define bfMegabytes(n) (bfKilobytes(n) * 1024)
#define bfGigabytes(n) (bfMegabytes(n) * 1024)

#if BF_MEMORY_DEBUG_ASSERTIONS
bool bfMemAssertImpl(const bool expr, const char* const expr_str, const char* const filename, const int line_number, const char* const assert_msg);

#define bfMemAssert(expr, msg) bfMemAssertImpl((expr), #expr, __FILE__, __LINE__, (msg))
#else
#define bfMemAssert(expr, msg) (void)0
#endif

/*!
 * @brief
 *   Aligns size to required_alignment.
 *
 * @param size
 *   The potentially unaligned size of an object.
 *
 * @param required_alignment
 *   Must be a non zero power of two.
 *
 * @return size_t
 *   The size of the object for the required alignment,
 */
inline constexpr std::size_t bfAlignUpSize(const std::size_t size, const std::size_t required_alignment) noexcept
{
  // Doesn't work in constexpr context.
  //   bfMemAssert(required_alignment > 0 && (required_alignment & (required_alignment - 1)) == 0, "bfAlignUpSize:: The alignment must be a non-zero power of two.");

  const size_t required_alignment_mask = required_alignment - 1;

  return size + required_alignment_mask & ~required_alignment_mask;
}

/*!
 * @brief
 *   Moves `ptr` up a certain offset to be aligned.
 *
 * @param ptr
 *   The pointer you want aligned.
 *
 * @param required_alignment
 *   The desired alignment.
 *   Must be a non zero power of two.
 *
 * @return
 *   The newly aligned pointer.
 */
void* bfAlignUpPointer(const void* const ptr, const size_t required_alignment);

/*!
 * @brief
 *   Implements "std::align" but in C.
 *   `size` must be <= *space or this functions will always fail.
 *
 * @param[in] alignment
 *   The alignment that you are wanting the pointer to be at.
 *   Must be a non zero power of two.
 *
 * @param[in] size
 *   The size of the block of that needs to be aligned.
 *
 * @param[in,out] ptr
 *   The pointer ot be aligned, will be aligned
 *   if this function succeeds otherwise unchanged.
 *
 * @param[in,out] space
 *   The size of the block of memory available to align memory to.
 *   if this function succeeds the space left in the allocation
 *   otherwise left unchanged.
 *
 * @return
 *   `ptr` is we can fit an aligned `size` into `space`,
 *   otherwise NULL is returned.
 */
void* bfStdAlign(size_t alignment, size_t size, void** ptr, size_t* space);

/*!
 * @brief
 *  Returns the number of bytes needed to align \p ptr to the next \p alignment
 *  aligned address.
 *
 * @param ptr
 *   The pointer to align.
 *
 * @param alignment
 *   The desired alignment.
 *
 * @return
 *   The number of bytes needed to align \p ptr to the next \p alignment aligned address.
 */
std::size_t bfMemAlignOffset(const void* const ptr, const std::size_t alignment);

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

//-------------------------------------------------------------------------------------//
// Allocator Stack Interface: The stack is thread local.
//-------------------------------------------------------------------------------------//

/*!
 * @brief
 *   Pushes a new global allocator onto the thread local stack.
 *
 * @param new_allocator
 *   The allocator you now want to be returned by bfMemAllocator.
 */
void bfMemAllocatorPush(bf::IAllocator& new_allocator);

/*!
 * @brief
 *   Returns the current global allocator.
 *
 * @return
 *   The current global allocator.
 */
bf::IAllocator& bfMemAllocator(void);

/*!
 * @brief
 *   Removes the last global allocator and restores it to it's previous value.
 */
void bfMemAllocatorPop(void);

//-------------------------------------------------------------------------------------//
// Base Allocation Interface: Raw byte allocation, address has no alignment guarantees.
//-------------------------------------------------------------------------------------//

/*!
 * @brief
 *   Allocates memory from \p self.
 *
 *   The memory may only be freed with bfMemDeallocate.
 *
 * @param self
 *   The allocator to request memory from.
 *
 * @param size
 *   The minimum number of bytes the returned block should contain.
 *
 * @return
 *   A pointer size pair of the usable block of memory, the size may be larger than \p size.
 *   bf::AllocationResult::Null - on failed allocation.
 *
 * @see bfMemDeallocate
 */
bf::AllocationResult bfMemAllocate(bf::IAllocator& self, const std::size_t size);

/*!
 * @brief
 *   Frees the memory pointed to by \p mem_block.
 *
 *   Must only pass in a block from bfMemAllocate.
 *
 * @param self
 *   The allocator to return memory to.
 *
 * @param mem_block
 *   The block of memory to free. mem_block.size may be smaller than what was returned by bfMemAllocate.
 *
 * @see bfMemAllocate
 */
void bfMemDeallocate(bf::IAllocator& self, const bf::AllocationResult mem_block);

//-------------------------------------------------------------------------------------//
// Aligned Allocation API: Takes up extra memory for offset header and alignment.
//-------------------------------------------------------------------------------------//

/*!
 * @brief
 *   Allocates memory from \p self, returns a block of memory with a minimum size of \p size
 *   and an alignment of \p alignment.
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
 * @return
 *    A pointer size pair of the usable block of memory, the size may be larger than \p size.
 *    bf::AllocationResult::Null - on failed allocation.
 *
 * @see bfMemDeallocateAligned
 */
bf::AllocationResult bfMemAllocateAligned(bf::IAllocator& self, const std::size_t size, const std::size_t alignment);

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
void bfMemDeallocateAligned(bf::IAllocator& self, const bf::AllocationResult mem_block, const std::size_t alignment);

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
template<typename T, typename... Args>
T* bfMemAllocate(bf::IAllocator& self, Args&&... args);

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
template<typename T>
void bfMemDeallocate(bf::IAllocator& self, T* const ptr);

/*!
 * @brief
 *   Aligned variant of bfMemAllocate, will be aligned to the value of alignof(T).
 *
 * @tparam T
 *   The type of object.
 *
 * @tparam ...Args
 *   The constructor arguments types to pass to T.
 *
 * @param self
 *   The allocator to request memory from.
 *
 * @param ...args
 *    The constructor arguments to pass to T.
 *
 * @return
 *   On Success: Returns a new object T.
 *   On Failure: nullptr.
 */
template<typename T, typename... Args>
T* bfMemAllocateAligned(bf::IAllocator& self, Args&&... args);

/*!
 * @brief
 *   Aligned variant of bfMemDeallocate.
 *
 * @tparam T
 *   The type of object.
 *
 * @param self
 *   The allocator to return memory to.
 *
 * @param ptr
 *   The pointer allocated with bfMemAllocateAligned.
 */
template<typename T>
void bfMemDeallocateAligned(bf::IAllocator& self, T* const ptr);

//-------------------------------------------------------------------------------------//
// Array API: Aligned versions take up extra memory.
//-------------------------------------------------------------------------------------//

/*!
 * @brief
 *   Initializes a block of memory using the policy from \p init.
 *   The block of memory should be unitialized.
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
T* bfMemArrayInit(const bf::AllocationResult mem_block, const std::size_t num_elements);

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
template<typename T, bf::MemArrayInit init = bf::MemArrayInit::UNINITIALIZE>
T* bfMemAllocateArray(bf::IAllocator& self, const std::size_t num_elements);

/*!
 * @brief
 *   Allocates an array of type \p T of length \p num_elements.
 *
 * @tparam T
 *   The type of array.
 *
 * @param self
 *   The allocator to request memory from.
 *
 * @param num_elements
 *   The number of elements in the array.
 *
 * @param value
 *   The initial value to set the elements of the array to.
 *
 * @return
 *   On Success: An array of \p num_elements length.
 *   On Failure: nullptr
 *
 * @see bfMemDeallocateArray
 */
template<typename T>
T* bfMemAllocateArray(bf::IAllocator& self, const std::size_t num_elements, const T& value);

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
void bfMemDeallocateArray(bf::IAllocator& self, T* const array, const std::size_t num_elements);

/*!
 * @brief
 *   Aligned version of bfMemAllocateArray.
 *
 * @tparam T
 *   The type of array.
 *
 * @param self
 *   The allocator to request memory from.
 *
 * @param num_elements
 *   The number of elements in the array.
 *
 * @param init
 *   Initialization policy to apply to the array.
 *
 * @param alignment
 *   The desired minimum alignment of the beginning of the block of memory.
 *
 * @return
 *   On Success: An array of \p num_elements length with alignment \p alignment.
 *   On Failure: nullptr
 *
 * @see bfMemDeallocateArrayAligned
 */
template<typename T, bf::MemArrayInit init = bf::MemArrayInit::UNINITIALIZE>
T* bfMemAllocateArrayAligned(bf::IAllocator& self, const std::size_t num_elements, const std::size_t alignment = alignof(T));

/*!
 * @brief
 *   Aligned version of bfMemAllocateArray.
 *
 * @tparam T
 *   The type of array.
 *
 * @param self
 *   The allocator to request memory from.
 *
 * @param num_elements
 *   The number of elements in the array.
 *
 * @param value
 *   The initial value to set the elements of the array to.
 *
 * @param alignment
 *   The desired minimum alignment of the beginning of the block of memory.
 *
 * @return
 *   On Success: An array of \p num_elements length with alignment \p alignment.
 *   On Failure: nullptr
 *
 * @see bfMemDeallocateArrayAligned
 */
template<typename T>
T* bfMemAllocateArrayAligned(bf::IAllocator& self, const std::size_t num_elements, const T& value, const std::size_t alignment = alignof(T));

/*!
 * @brief
 *   Free memory allocated from bfMemAllocateArrayAligned.
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
 *   The array to deallocate.
 *
 * @param num_elements
 *   The number of elements in the array.
 *
 * @param alignment
 *   The alignment passed into bfMemAllocateArrayAligned.
 *
 * @see bfMemAllocateArrayAligned
 */
template<bf::MemArrayDestroy destroy = bf::MemArrayDestroy::NONE, typename T>
void bfMemDeallocateArrayAligned(bf::IAllocator& self, T* const array, const std::size_t num_elements, const std::size_t alignment = alignof(T));

//-------------------------------------------------------------------------------------//
// Templated Function Implementations
//-------------------------------------------------------------------------------------//

template<typename T, typename... Args>
T* bfMemAllocate(bf::IAllocator& self, Args&&... args)
{
  const bf::AllocationResult mem_block = bfMemAllocate(self, sizeof(T));

  return mem_block ? new (mem_block.ptr) T(std::forward<Args>(args)...) : nullptr;
}

template<typename T>
void bfMemDeallocate(bf::IAllocator& self, T* const ptr)
{
  if (ptr)
  {
    std::destroy_at(ptr);
    bfMemDeallocate(self, bf::AllocationResult{ptr, sizeof(T)});
  }
}

template<typename T, typename... Args>
T* bfMemAllocateAligned(bf::IAllocator& self, Args&&... args)
{
  const bf::AllocationResult mem_block = bfMemAllocateAligned(self, sizeof(T), alignof(T));

  return mem_block ? new (mem_block.ptr) T(std::forward<Args>(args)...) : nullptr;
}

template<typename T>
void bfMemDeallocateAligned(bf::IAllocator& self, T* const ptr)
{
  if (ptr)
  {
    std::destroy_at(ptr);
    bfMemDeallocateAligned(self, bf::AllocationResult{ptr, sizeof(T)}, alignof(T));
  }
}

template<typename T, bf::MemArrayInit init>
T* bfMemArrayInit(const bf::AllocationResult mem_block, const std::size_t num_elements)
{
  T* const typed_array = static_cast<T*>(mem_block.ptr);

  if (typed_array)
  {
    if constexpr (init == bf::MemArrayInit::UNINITIALIZE)
    {
    }
    else if constexpr (init == bf::MemArrayInit::DEFAULT_CONSTRUCT)
    {
      std::uninitialized_default_construct(typed_array, typed_array + num_elements);
    }
    else if constexpr (init == bf::MemArrayInit::VALUE_CONSTRUCT)
    {
      std::uninitialized_value_construct(typed_array, typed_array + num_elements);
    }
  }

  return typed_array;
}

template<typename T, bf::MemArrayInit init>
T* bfMemAllocateArray(bf::IAllocator& self, const std::size_t num_elements)
{
  const bf::AllocationResult mem_block = bfMemAllocate(self, sizeof(T) * num_elements);

  return bfMemArrayInit<T, init>(mem_block, num_elements);
}

template<typename T>
T* bfMemAllocateArray(bf::IAllocator& self, const std::size_t num_elements, const T& value)
{
  const bf::AllocationResult mem_block = bfMemAllocate(self, sizeof(T) * num_elements);

  if (mem_block)
  {
    T* const typed_array = static_cast<T*>(mem_block.ptr);

    std::uninitialized_fill(typed_array, typed_array + num_elements, value);
  }

  return bfMemArrayInit<T>(mem_block, num_elements, value);
}

template<bf::MemArrayDestroy destroy, typename T>
void bfMemDestructArray(T* const array, const std::size_t num_elements)
{
  if constexpr (destroy == bf::MemArrayDestroy::DESTRUCT)
  {
    std::destroy_n(array, num_elements);
  }
}

template<bf::MemArrayDestroy destroy, typename T>
void bfMemDeallocateArray(bf::IAllocator& self, T* const array, const std::size_t num_elements)
{
  if (array && num_elements)
  {
    bfMemDestructArray<destroy>(array, num_elements);
    bfMemDeallocate(self, bf::AllocationResult{array, sizeof(T) * num_elements});
  }
}

template<typename T, bf::MemArrayInit init>
T* bfMemAllocateArrayAligned(bf::IAllocator& self, const std::size_t num_elements, const std::size_t alignment)
{
  const bf::AllocationResult mem_block = bfMemAllocateAligned(self, sizeof(T) * num_elements, alignment);

  return bfMemArrayInit<T, init>(mem_block, num_elements);
}

template<typename T>
T* bfMemAllocateArrayAligned(bf::IAllocator& self, const std::size_t num_elements, const T& value, const std::size_t alignment)
{
  const bf::AllocationResult mem_block = bfMemAllocateAligned(self, sizeof(T) * num_elements, alignment);

  return bfMemArrayInit<T>(mem_block, num_elements, value);
}

template<bf::MemArrayDestroy destroy, typename T>
void bfMemDeallocateArrayAligned(bf::IAllocator& self, T* const array, const std::size_t num_elements, const std::size_t alignment)
{
  if (array && num_elements)
  {
    bfMemDestructArray<destroy>(array, num_elements);
    bfMemDeallocateAligned(self, bf::AllocationResult{array, sizeof(T) * num_elements}, alignment);
  }
}

#endif /* BF_MEMORY_API_HPP */

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2022 Shareef Abdoul-Raheem

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
