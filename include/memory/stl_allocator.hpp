/******************************************************************************/
/*!
 * @file   stl_allocator.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   > This allocator is a designed for use with stl containers.
 *   > This must only be used in C++11 or later because C++03 required all allocators of a certain type to be compatible but this allocator is stateful.
 *
 *  References:
 *    - [Allocator Boilerplate](https://howardhinnant.github.io/allocator_boilerplate.html)
 *    - [MSVC Allocators](https://docs.microsoft.com/en-us/cpp/standard-library/allocators?view=msvc-170)
 *
 * @copyright Copyright (c) 2019-2023 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_STL_ALLOCATOR_HPP
#define LIB_FOUNDATION_MEMORY_STL_ALLOCATOR_HPP

#include "allocation.hpp"           // IAllocator, bfMemAllocateArray, bfMemDeallocateArray
#include "memory/default_heap.hpp"  // Memory::DefaultHeap

#include <utility>  // forward

namespace Memory
{
  /*
     C++11/14 Allocator 'Concept'

     Traits:
       value_type                             T
       pointer                                T*
       const_pointer                          const T*
       reference                              T&
       const_reference                        const T&
       size_type                              std::size_t
       difference_type                        std::ptrdiff_t
       propagate_on_container_move_assignment std::true_ty
       rebind                                 template< class U > struct rebind { typedef allocator<U> other; };
       is_always_equal                        std::true_type

     Methods:
       ctor / dtor
       address
       allocate
       deallocate
       max_size
       construct
       destroy

     Operators:
       operator==
       operator!=

    ********************************************************************************

     C++17 Allocator 'Concept'

     Traits:
       value_type                             T
       size_type                              std::size_t
       difference_type                        std::ptrdiff_t
       propagate_on_container_move_assignment std::true_type
       is_always_equal                        std::true_type

     Methods:
       ctor / dtor
       allocate
       deallocate

     Operators:
       operator==
       operator!=

     ********************************************************************************

     C++20/23 Allocator 'Concept'

     Traits:
       value_type                             T
       size_type                              std::size_t
       difference_type                        std::ptrdiff_t
       propagate_on_container_move_assignment std::true_type
       is_always_equal                        std::true_type (deprecated in C++23)

     Methods:
       ctor / dtor
       allocate
       allocate_at_least (added in C++23)
       deallocate

     Operators:
       constexpr operator==
  */

  /*!
   * @brief
   *   Provides an STL compliant proxy for the IAllocator API.
   *
   * @tparam T
   *   The type of object this allocated expects to make memory for.
   */
  template<typename T, typename AllocatorConcept = IAllocator>
  class StlAllocator
  {
   public:
    using pointer                                = T *;
    using const_pointer                          = const T *;
    using void_pointer                           = void *;
    using const_void_pointer                     = const void *;
    using difference_type                        = std::ptrdiff_t;
    using size_type                              = std::size_t;
    using reference                              = T &;
    using const_reference                        = const T &;
    using value_type                             = T;
    using type                                   = T;
    using is_always_equal                        = std::false_type;
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap            = std::false_type;

    template<class U>
    struct rebind
    {
      using other = StlAllocator<U>;
    };

   private:
    AllocatorConcept &m_MemoryBackend;

   public:
    StlAllocator(AllocatorConcept &backend = Memory::DefaultHeap()) noexcept :
      m_MemoryBackend{backend}
    {
    }

    template<class U, typename RhsAllocatorConcept>
    StlAllocator(const StlAllocator<U, RhsAllocatorConcept> &rhs) noexcept :
      StlAllocator(rhs.backend())
    {
      static_assert(std::is_convertible_v<RhsAllocatorConcept *, AllocatorConcept *>, "Allocator types not convertable.");
    }

    static size_type max_size() noexcept { return static_cast<size_type>(-1) / sizeof(value_type); }

    pointer                         address(reference x) const noexcept { return &x; }
    const_pointer                   address(const_reference x) const noexcept { return &x; }
    [[nodiscard]] constexpr pointer allocate(size_type s) { return s ? bfMemAllocateArray<T>(m_MemoryBackend, s) : nullptr; }
    constexpr void                  deallocate(pointer p, size_type s) { bfMemDeallocateArray(m_MemoryBackend, p, s); }

    template<class U, class... Args>
    void construct(U *p, Args &&...args)
    {
      ::new (p) U(std::forward<Args>(args)...);
    }

    template<class U>
    void destroy(U *p)
    {
      p->~U();
    }

    StlAllocator select_on_container_copy_construction() const noexcept { return *this; }

    template<class U>
    constexpr bool operator==(const StlAllocator<U> &rhs) const noexcept
    {
      return &backend() == &rhs.backend();
    }

    template<class U>
    constexpr bool operator!=(const StlAllocator<U> &rhs) const noexcept
    {
      return &backend() != &rhs.backend();
    }

    AllocatorConcept &backend() const { return m_MemoryBackend; }
  };

}  // namespace Memory

#endif  // LIB_FOUNDATION_MEMORY_STL_ALLOCATOR_HPP

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2019-2023 Shareef Abdoul-Raheem

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
