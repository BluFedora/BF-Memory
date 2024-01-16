/******************************************************************************/
/*!
 * @file   smart_pointer.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Helpers for creating smart pointers with custom allocators.
 *
 * @copyright Copyright (c) 2023 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_SMART_POINTER_HPP
#define LIB_FOUNDATION_MEMORY_SMART_POINTER_HPP

#include "memory/allocation.hpp"     // IAllocator, bfMemAllocateArray, bfMemDeallocateArray, bfMemAllocateObject, bfMemDeallocateObject
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

  template<typename AllocatorConcept>
  class UniquePtrDeleter
  {
    template<typename AllocatorConcept>
    friend class UniquePtrDeleter;

   private:
    AllocatorConcept* m_Allocator;

   public:
    constexpr UniquePtrDeleter(AllocatorConcept* const allocator = nullptr) :
      m_Allocator{allocator}
    {
    }

    template<typename U>
    constexpr UniquePtrDeleter(UniquePtrDeleter<U>&& rhs) :
      m_Allocator{std::exchange(rhs.m_Allocator, nullptr)}
    {
    }

    constexpr AllocatorConcept* allocator() const { return m_Allocator; }
    constexpr MemoryIndex       length() const { return m_Allocator != nullptr ? 1u : 0u; }

    template<typename T>
    void operator()(T* const ptr) noexcept
    {
      bfMemDeallocateObject(*m_Allocator, ptr);
    }
  };

  template<typename T, typename AllocatorConcept, MemoryIndex N>
  class UniquePtrArrayDeleter
  {
    template<typename T, typename AllocatorConcept, MemoryIndex N>
    friend class UniquePtrArrayDeleter;

   public:
    using pointer = std::remove_extent_t<T>*;  //!< Bounded array needs to redefine pointer type from *T[N] => T*.

   private:
    AllocatorConcept* m_Allocator;

   public:
    constexpr UniquePtrArrayDeleter(AllocatorConcept* const allocator = nullptr) :
      m_Allocator{allocator}
    {
    }

    template<typename RhsAllocatorConcept>
    constexpr UniquePtrArrayDeleter(UniquePtrArrayDeleter<T, RhsAllocatorConcept, N>&& rhs) :
      m_Allocator{std::exchange(rhs.m_Allocator, nullptr)}
    {
    }

    constexpr AllocatorConcept* allocator() const { return m_Allocator; }
    constexpr MemoryIndex       length() const { return m_Allocator != nullptr ? N : 0u; }

    template<typename T>
    void operator()(T* const ptr) noexcept
    {
      bfMemDeallocateArray<Memory::ArrayDestruct::DESTRUCT>(*m_Allocator, ptr, N);
    }
  };

  template<typename T, typename AllocatorConcept>
  class UniquePtrArrayDeleter<T, AllocatorConcept, 0u>
  {
    template<typename T, typename AllocatorConcept, MemoryIndex N>
    friend class UniquePtrArrayDeleter;

   private:
    AllocatorConcept* m_Allocator;
    MemoryIndex       m_NumElements;

   public:
    constexpr UniquePtrArrayDeleter(AllocatorConcept* const allocator = nullptr, const MemoryIndex num_elements = 0u) :
      m_Allocator{allocator},
      m_NumElements{num_elements}
    {
    }

    template<typename RhsAllocatorConcept>
    constexpr UniquePtrArrayDeleter(UniquePtrArrayDeleter<T, RhsAllocatorConcept, 0u>&& rhs) :
      m_Allocator{std::exchange(rhs.m_Allocator, nullptr)},
      m_NumElements{std::exchange(rhs.m_NumElements, 0)}
    {
    }

    constexpr AllocatorConcept* allocator() const { return m_Allocator; }
    constexpr MemoryIndex       length() const { return m_Allocator != nullptr ? m_NumElements : 0u; }

    template<typename T>
    void operator()(T* const ptr) noexcept
    {
      bfMemDeallocateArray<Memory::ArrayDestruct::DESTRUCT>(*m_Allocator, ptr, m_NumElements);
    }
  };
}  // namespace Memory

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
    bfMemDeallocateArray<Memory::ArrayDestruct::DESTRUCT>(*allocator, ptr, num_elements, alignof(T));
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
    bfMemDeallocateArray<Memory::ArrayDestruct::DESTRUCT>(*allocator, ptr, num_elements, element_alignment);
  };

  return memory ? SharedPtr<T>(memory, std::move(deleter), Memory::StlAllocator<element_type, AllocatorConcept>(*allocator)) : nullptr;
}

// https://devblogs.microsoft.com/oldnewthing/20230818-00/?p=108619

template<typename T, typename U>
SharedPtr<T> bfMemMakeSharedAlias(SharedPtr<U> owner, T* const ptr)
{
  return owner.use_count() != 0 ? SharedPtr<T>(std::move(owner), ptr) : nullptr;
}

template<typename T, typename U>
SharedPtr<T[]> bfMemMakeSharedAliasArray(SharedPtr<U> owner, T* const ptr)
{
  return owner.use_count() != 0 ? SharedPtr<T[]>(std::move(owner), ptr) : nullptr;
}

namespace detail
{
  template<typename T, typename AllocatorConcept>
  using BaseUniquePtr = std::unique_ptr<T, std::conditional_t<std::is_array_v<T>, Memory::UniquePtrArrayDeleter<T, AllocatorConcept, std::extent_v<T>>, Memory::UniquePtrDeleter<AllocatorConcept>>>;
}

/*!
 * @brief
 *   The default std::unique_ptr does not support std::unique_ptr<T[N]> with a bounded array
 *   but this modified version does the correct thing when containing a bounded array.
 *
 *   The normal unique_ptr doesn't have an specialization for a bounded array and will try to use
 *   the singular object specialization which calls the incorrect delete.
 *
 *   One additional feature if the ability to query the number of elements the UniquePtr has
 *   through the deleter.
 *
 *   ```
 *   UniquePtr<byte[]> ptr = ...;
 *   ptr.length(); // This is length of the contained array.
 *   ```
 *
 * @tparam T
 *   The type of object stored in this pointer.
 *
 * @tparam AllocatorConcept
 *   The allocator type for this UniquePtr.
 */
template<typename T, typename AllocatorConcept = IAllocator>
struct UniquePtr : public detail::BaseUniquePtr<T, AllocatorConcept>
{
  using element_type = std::remove_extent_t<T>;

  using detail::BaseUniquePtr<T, AllocatorConcept>::unique_ptr;

  template<typename RhsAllocator>
  UniquePtr(UniquePtr<T, RhsAllocator>&& rhs) :
    detail::BaseUniquePtr<T, AllocatorConcept>(rhs.release(), std::move(rhs.get_deleter()))
  {
    static_assert(std::is_convertible_v<RhsAllocator*, AllocatorConcept*>, "Allocator types not convertable.");
  }

  UniquePtr(T* const ptr) = delete;

  constexpr element_type& operator[](const MemoryIndex index) const noexcept { return this->get()[index]; }

  constexpr element_type*       begin() { return this->get(); }
  constexpr element_type*       end() { return this->get() + length(); }
  constexpr const element_type* begin() const { return this->get(); }
  constexpr const element_type* end() const { return this->get() + length(); }
  constexpr MemoryIndex         length() const { return this->get_deleter().length(); }
};

template<typename T, typename AllocatorConcept, typename = std::enable_if_t<!std::is_array_v<T>>, typename... Args>
UniquePtr<T, AllocatorConcept> bfMemMakeUnique(AllocatorConcept* const allocator, Args&&... args)
{
  return UniquePtr<T, AllocatorConcept>(bfMemAllocateObject<T>(*allocator, std::forward<Args>(args)...), Memory::UniquePtrDeleter<AllocatorConcept>(allocator));
}

// TODO(SR): Add overloads with custom alignment.

template<typename T, typename AllocatorConcept, typename = std::enable_if_t<Memory::is_unbounded_array_v<T>>>
UniquePtr<T, AllocatorConcept> bfMemMakeUnique(AllocatorConcept* const allocator, const MemoryIndex num_elements)
{
  std::remove_extent_t<T>* const                                       allocation = bfMemAllocateArray<std::remove_extent_t<T>, Memory::ArrayConstruct::DEFAULT_CONSTRUCT>(*allocator, num_elements);
  Memory::UniquePtrArrayDeleter<T, AllocatorConcept, std::extent_v<T>> deleter{allocator, num_elements};

  return UniquePtr<T, AllocatorConcept>(allocation, std::move(deleter));
}

template<typename T, typename AllocatorConcept, typename = std::enable_if_t<Memory::is_bounded_array_v<T>>>
UniquePtr<T, AllocatorConcept> bfMemMakeUnique(AllocatorConcept* const allocator)
{
  constexpr MemoryIndex                                                num_elements = std::extent_v<T>;
  std::remove_extent_t<T>* const                                       allocation   = bfMemAllocateArray<std::remove_extent_t<T>, Memory::ArrayConstruct::DEFAULT_CONSTRUCT>(*allocator, num_elements);
  Memory::UniquePtrArrayDeleter<T, AllocatorConcept, std::extent_v<T>> deleter{allocator};

  return UniquePtr<T, AllocatorConcept>(allocation, std::move(deleter));
}

#undef IS_CXX20

#endif  // LIB_FOUNDATION_MEMORY_SMART_POINTER_HPP

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
