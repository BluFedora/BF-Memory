/******************************************************************************/
/*!
 * @file   smart_pointer.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Helpers for creating smart pointers with custom allocators.
 *
 * @copyright Copyright (c) 2023-2024 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_SMART_POINTER_HPP
#define LIB_FOUNDATION_MEMORY_SMART_POINTER_HPP

#include "memory/allocation.hpp"     // IPolymorphicAllocator, bfMemAllocateArray, bfMemDeallocateArray, bfMemAllocateObject, bfMemDeallocateObject
#include "memory/stl_allocator.hpp"  // StlAllocator

#include <memory>       // shared_ptr, allocate_shared, unique_ptr
#include <type_traits>  // is_array_v, is_bounded_array_v, is_unbounded_array_v, enable_if_t
#include <utility>      // move, exchange

#if defined(_MSC_VER)
#define IS_CXX20 (_MSVC_LANG >= 202002L)  // Unless `/Zc:__cplusplus` is specified `__cplusplus` has an incorrect value of `199711L` on msvc. (_MSVC_LANG is vs2015 and up)
#else
#define IS_CXX20 (__cplusplus >= 202002L)
#endif

namespace Memory
{
#if IS_CXX20

  using std::is_bounded_array_v;
  using std::is_unbounded_array_v;

#else

  template<typename>
  inline constexpr bool is_bounded_array_v = false;

  template<typename T, size_t N>
  inline constexpr bool is_bounded_array_v<T[N]> = true;

  template<typename>
  inline constexpr bool is_unbounded_array_v = false;

  template<typename T>
  inline constexpr bool is_unbounded_array_v<T[]> = true;

#endif

  /*!
   * @brief
   *   This prefixes all allocations made from `bfMemMakeUnique`.
   */
  struct UniquePtrHeader
  {
    using UniquePtrDeleterFn = void (*)(UniquePtrHeader* const header, void* const ptr);

    void*              allocator;
    MemoryIndex        num_objects;
    UniquePtrDeleterFn deleter;
  };

  /*!
   * @brief
   *   Polymorphic deleter for UniquePtr.
   */
  class BaseUniquePtrDeleter
  {
   private:
    UniquePtrHeader* m_Header;

   public:
    constexpr BaseUniquePtrDeleter(UniquePtrHeader* const header = nullptr) noexcept :
      m_Header{header}
    {
    }

    constexpr MemoryIndex length() const noexcept { return m_Header->num_objects; }

    void operator()(void* const ptr) const noexcept
    {
      m_Header->deleter(m_Header, ptr);
    }
  };

  template<typename T>
  struct UniquePtrDeleter : public BaseUniquePtrDeleter
  {
    using pointer = std::remove_extent_t<T>*;  //!< Bounded array needs to redefine pointer type from T[N]* => T*.

    using BaseUniquePtrDeleter::BaseUniquePtrDeleter;

    template<typename U>
    UniquePtrDeleter(UniquePtrDeleter<U>&& rhs) :
      BaseUniquePtrDeleter(rhs)
    {
    }
  };
}  // namespace Memory

// Articles on std::shared_ptr
//   - [What it means when you convert between different shared_ptrs](https://devblogs.microsoft.com/oldnewthing/20230817-00/?p=108611)
//   - [Inside STL: The shared_ptr constructor and enable_shared_from_this](https://devblogs.microsoft.com/oldnewthing/20230816-00/?p=108608)

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename T, typename AllocatorConcept, typename = std::enable_if_t<!std::is_array_v<T>>, typename... Args>
SharedPtr<T> bfMemMakeShared(AllocatorConcept* allocator, Args&&... args)
{
  return std::allocate_shared<T>(Memory::StlAllocator<T, AllocatorConcept>(*allocator), std::forward<Args>(args)...);
}

template<typename T, typename AllocatorConcept, typename = std::enable_if_t<Memory::is_bounded_array_v<T>>>
SharedPtr<T> bfMemMakeShared(AllocatorConcept* allocator)
{
#if IS_CXX20
  return std::allocate_shared<T>(Memory::StlAllocator<T, AllocatorConcept>(*allocator));
#else
  struct ArrayWrapper
  {
    T value;
  };

  SharedPtr<ArrayWrapper> real_shared_ptr = std::allocate_shared<ArrayWrapper>(Memory::StlAllocator<ArrayWrapper, AllocatorConcept>(*allocator));
  ArrayWrapper* const     array_ptr       = real_shared_ptr.get();

  return array_ptr ? SharedPtr<T>(std::move(real_shared_ptr), array_ptr->value) : nullptr;
#endif
}

template<typename T, typename AllocatorConcept, typename = std::enable_if_t<Memory::is_unbounded_array_v<T>>>
SharedPtr<T> bfMemMakeShared(AllocatorConcept* const allocator, const MemoryIndex num_elements)
{
#if IS_CXX20
  return std::allocate_shared<T>(Memory::StlAllocator<T, AllocatorConcept>(*allocator), num_elements);
#else
  using element_type = std::remove_extent_t<T>;

  element_type* const memory = bfMemAllocateArray<element_type, Memory::ArrayConstruct::DEFAULT_CONSTRUCT>(*allocator, num_elements, alignof(T));

  auto deleter = [allocator, num_elements](element_type* const ptr) {
    Memory::DestructRange(ptr, ptr + num_elements);
    bfMemDeallocateArray(*allocator, ptr, num_elements, alignof(T));
  };

  return memory ? SharedPtr<T>(memory, std::move(deleter), Memory::StlAllocator<element_type, AllocatorConcept>(*allocator)) : nullptr;
#endif
}

template<typename T, typename AllocatorConcept, typename = std::enable_if_t<Memory::is_unbounded_array_v<T>>>
SharedPtr<T> bfMemMakeShared(AllocatorConcept* const allocator, const MemoryIndex num_elements, const MemoryIndex element_alignment)
{
  using element_type = std::remove_extent_t<T>;

  element_type* const memory = bfMemAllocateArray<element_type, Memory::ArrayConstruct::DEFAULT_CONSTRUCT>(*allocator, num_elements, element_alignment);

  auto deleter = [allocator, num_elements, element_alignment](element_type* const ptr) {
    Memory::DestructRange(ptr, ptr + num_elements);
    bfMemDeallocateArray(*allocator, ptr, num_elements, element_alignment);
  };

  return memory ? SharedPtr<T>(memory, std::move(deleter), Memory::StlAllocator<element_type, AllocatorConcept>(*allocator)) : nullptr;
}

// https://devblogs.microsoft.com/oldnewthing/20230818-00/?p=108619

template<typename T, typename U>
SharedPtr<T> bfMemMakeSharedAlias(SharedPtr<U> owner, T* const ptr)
{
  return owner.use_count() != 0 ? SharedPtr<T>(std::move(owner), ptr) : nullptr;
}

//   std::remove_extent_t<T> ??
template<typename T, typename U>
SharedPtr<T[]> bfMemMakeSharedAliasArray(SharedPtr<U> owner, T* const ptr)
{
  return owner.use_count() != 0 ? SharedPtr<T[]>(std::move(owner), ptr) : nullptr;
}

/*!
 * @brief
 *   The default std::unique_ptr does not support std::unique_ptr<T[N]> with a bounded array
 *   but this modified version does the correct thing when containing a bounded array.
 *
 *   The normal unique_ptr doesn't have an specialization for a bounded array and will try to use
 *   the singular object specialization which calls the incorrect delete.
 *
 *   Additional Features:
 *     - The ability to query the number of elements the UniquePtr<T[]> has.
 *     - No virtual destructor needed if this is a UniquePtr<Base> is created from a UniquePtr<Derived>.
 *
 *   ```
 *   UniquePtr<byte[]> ptr = ...;
 *   ptr.length(); // This is length of the contained array.
 *   ```
 *
 * @tparam T
 *   The type of object stored in this pointer.
 */
template<typename T>
struct UniquePtr : public std::unique_ptr<T, Memory::UniquePtrDeleter<T>>
{
  using element_type = std::remove_extent_t<T>;

  using std::unique_ptr<T, Memory::UniquePtrDeleter<T>>::unique_ptr;

  UniquePtr(T* const ptr) = delete;

  constexpr element_type& operator[](const MemoryIndex index) const noexcept { return this->get()[index]; }
  constexpr element_type* begin() const noexcept { return this->get(); }
  constexpr element_type* end() const noexcept { return this->get() + length(); }
  constexpr MemoryIndex   length() const noexcept { return this->get_deleter().length(); }
};

namespace Memory
{
  template<typename T, typename AllocatorConcept>
  UniquePtr<T> MakeUniqueImpl(AllocatorConcept* const allocator, const MemoryIndex num_objects)
  {
    using pointer     = typename UniquePtrDeleter<T>::pointer;
    using object_type = std::remove_pointer_t<pointer>;

    static constexpr MemoryIndex alignment   = alignof(T) < alignof(UniquePtrHeader) ? alignof(UniquePtrHeader) : alignof(T);
    static constexpr MemoryIndex header_size = Memory::AlignSize(sizeof(UniquePtrHeader), alignment);

    const MemoryIndex objects_size = num_objects * sizeof(object_type);
    const MemoryIndex total_size   = header_size + objects_size;

    if (void* const allocation = bfMemAllocate(*allocator, total_size, alignment).ptr; allocation != nullptr)
    {
      UniquePtrHeader* const header = static_cast<UniquePtrHeader*>(allocation);

      header->allocator   = allocator;
      header->num_objects = num_objects;
      header->deleter     = +[](UniquePtrHeader* const header, void* const ptr) {
        const MemoryIndex num_objects  = header->num_objects;
        const MemoryIndex objects_size = num_objects * sizeof(object_type);
        const MemoryIndex total_size   = header_size + objects_size;

        Memory::DestructRange(static_cast<pointer>(ptr), static_cast<pointer>(ptr) + num_objects);
        bfMemDeallocate(*static_cast<AllocatorConcept*>(header->allocator), header, total_size, alignment);
      };

      return UniquePtr<T>(reinterpret_cast<pointer>(static_cast<byte*>(allocation) + header_size), UniquePtrDeleter<T>(header));
    }

    return nullptr;
  }
}  // namespace Memory

template<typename T, typename AllocatorConcept, typename = std::enable_if_t<!std::is_array_v<T>>, typename... Args>
UniquePtr<T> bfMemMakeUnique(AllocatorConcept* const allocator, Args&&... args)
{
  UniquePtr<T> result = Memory::MakeUniqueImpl<T>(allocator, 1u);

  if (result)
  {
    Memory::Construct<T>(result.get(), std::forward<Args>(args)...);
  }

  return result;
}

// TODO(SR): Add overloads with custom alignment.

template<typename T, typename AllocatorConcept, typename = std::enable_if_t<Memory::is_unbounded_array_v<T>>>
UniquePtr<T> bfMemMakeUnique(AllocatorConcept* const allocator, const MemoryIndex num_elements)
{
  UniquePtr<T> result = Memory::MakeUniqueImpl<T>(allocator, num_elements);

  if (result)
  {
    Memory::DefaultConstructRange(result.get(), result.get() + num_elements);
  }

  return result;
}

template<typename T, typename AllocatorConcept, typename = std::enable_if_t<Memory::is_bounded_array_v<T>>>
UniquePtr<T> bfMemMakeUnique(AllocatorConcept* const allocator)
{
  constexpr MemoryIndex num_elements = std::extent_v<T>;
  UniquePtr<T>          result       = Memory::MakeUniqueImpl<T>(allocator, num_elements);

  if (result)
  {
    Memory::DefaultConstructRange(result.get(), result.get() + num_elements);
  }

  return result;
}

#undef IS_CXX20

namespace Memory
{
  template<typename T>
  UniquePtrDeleter<T>* GetDeleter(const UniquePtr<T>& ptr) noexcept
  {
    return ptr != nullptr ? &ptr->get_deleter() : nullptr;
  }

  template<typename DeleterType, typename T>
  DeleterType* GetDeleter(const SharedPtr<T>& ptr) noexcept
  {
    return std::get_deleter<DeleterType>(ptr);
  }
}  // namespace Memory

#endif  // LIB_FOUNDATION_MEMORY_SMART_POINTER_HPP

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
