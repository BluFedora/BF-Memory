/******************************************************************************/
/*!
 * @file   bf_imemory_manager.hpp
 * @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
 * @date   2019-12-26
 * @brief
 *   Outlines a basic interface for the various types of memory managers.
 *
 * @version 0.0.1
 *
 * @copyright Copyright (c) 2019-2022
 */
/******************************************************************************/
#ifndef BF_IMEMORY_MANAGER_HPP
#define BF_IMEMORY_MANAGER_HPP

#include <cstddef>     /* size_t                      */
#include <cstring>     /* memcpy                      */
#include <new>         /* placement new               */
#include <type_traits> /* is_convertible, enable_if_t */
#include <utility>     /* forward, move               */

#ifndef BF_MEMORY_DEBUG_WIPE_MEMORY
// NOTE(Shareef):
//   Set to 0 if you want faster allocations at the cost of less safety.
#define BF_MEMORY_DEBUG_WIPE_MEMORY 1
#define BF_MEMORY_DEBUG_SIGNATURE 0xCD
#define BF_MEMORY_DEBUG_ALIGNMENT_PAD 0xFE
#endif /* BIFROST_MEMORY_DEBUG_WIPE_MEMORY */

namespace bf
{
  /*!
   * @brief
   *   Interface for a memory manager.
   */
  class IMemoryManager
  {
   private:
    /*!
     * @brief
     *   Header for all array API allocated blocks.
     */
    struct ArrayHeader final
    {
      std::size_t size;
      std::size_t alignment;
    };

   protected:
    //-------------------------------------------------------------------------------------//
    // Only Subclasses should be able to call the constructor and destructor.
    //-------------------------------------------------------------------------------------//

    IMemoryManager()  = default;
    ~IMemoryManager() = default;

   public:
    //-------------------------------------------------------------------------------------//
    // Disallow Copies and Moves
    //-------------------------------------------------------------------------------------//

    IMemoryManager(const IMemoryManager& rhs)     = delete;
    IMemoryManager(IMemoryManager&& rhs) noexcept = delete;
    IMemoryManager& operator=(const IMemoryManager& rhs) = delete;
    IMemoryManager& operator=(IMemoryManager&& rhs) noexcept = delete;

    //-------------------------------------------------------------------------------------//
    // Base Interface
    //-------------------------------------------------------------------------------------//

    /*!
     * @brief
     *   Allocates a block of bytes of \p size.
     *
     * @param[in] size
     *   The size of the block to allocate.
     *
     * @return void*
     *   The begginning to the allocated block.
     *   nullptr if the request could not be fulfilled.
     */
    virtual void* allocate(std::size_t size) = 0;

    /*!
     * @brief
     *   Frees the memory pointer to by \p ptr.
     *
     * @param ptr
     *   A non null pointer to memory allcoated with 'IMemoryManager::allocate'.
     *   The pointer is invalid after this call returns.
     *
     * @param num_bytes
     *   The size the pointer was allocated with.
     */
    virtual void deallocate(void* ptr, std::size_t num_bytes) = 0;

    //
    // Unlike the Array API this does not do any 'magic' in the background,
    // Just a simple wrapper around a multiply and cast.
    //
    // Deallocate using `IMemoryManager::deallocateUnmanagedArray`.
    //
    template<typename T>
    inline T* allocateUnmanagedArray(std::size_t num_elements)
    {
      return static_cast<T*>(allocate(sizeof(T) * num_elements));
    }

    template<typename T>
    inline T* reallocateUnmanagedArray(T* ptr, std::size_t old_elements, std::size_t new_num_elements)
    {
      if (ptr)
      {
        deallocateUnmanagedArray<T>(ptr, old_elements);
      }

      return allocateUnmanagedArray<T>(new_num_elements);
    }

    template<typename T>
    inline void deallocateUnmanagedArray(T* ptr, std::size_t num_elements)
    {
      deallocate(ptr, sizeof(T) * num_elements);
    }

    //-------------------------------------------------------------------------------------//
    // Aligned Allocations API
    //-------------------------------------------------------------------------------------//

    /*!
     * @brief
     *   Allocates a block of bytes of \p size, the returned pointer will be on an address
     *   aligned to \p alignment.
     *
     * @param size
     *   The size of the block to allocate.
     *
     * @param alignment
     *   The alignment of the allocated block.
     *   Must be a power of two.
     *
     * @return void*
     *   The beginning to the allocated block.
     *   nullptr if the request could not be fulfilled.
     */
    void* allocateAligned(std::size_t size, std::size_t alignment);

    /*!
     * @brief
     *  Frees the memory pointer to by \p ptr.
     *
     * @param ptr
     *   The pointer to free memory from.
     *   Freeing a nullptr is safe.
     *   Must have been allocated with 'IMemoryManager::allocateAligned'.
     */
    void deallocateAligned(void* ptr, std::size_t size, std::size_t alignment);

    //-------------------------------------------------------------------------------------//
    // Templated API
    //-------------------------------------------------------------------------------------//

    /*!
     * @brief
     *   Allocates a new T object calling it's constructor.
     *
     * @tparam T
     *   The type of the object to allocate.
     *
     * @tparam Args
     *   Constructor argument type.
     *
     * @param args
     *   Constructor arguments.
     *
     * @return T*
     *   A pointer to the new object.
     *   nullptr if the request could not be fulfilled.
     */
    template<typename T, typename... Args>
    T* allocateT(Args&&... args)
    {
      void* const mem_block = allocate(sizeof(T));

      return mem_block ? new (mem_block) T(std::forward<Args>(args)...) : nullptr;
    }

    /*!
     * @brief
     *   Frees the memory pointer to by \p ptr.
     *
     * @tparam T
     *   The type of the object to deallocate.
     *
     * @param ptr
     *   The pointer to free memory from.
     *   Freeing a nullptr is safe.
     *   Must have been allocated with 'IMemoryManager::allocateT'.
     */
    template<typename T>
    void deallocateT(T* ptr)
    {
      if (ptr)
      {
        ptr->~T();
        deallocate(ptr, sizeof(T));
      }
    }

    //-------------------------------------------------------------------------------------//
    // Array API
    //   The Array api takes up extra memory for header and alignment.
    //-------------------------------------------------------------------------------------//

    /*!
     * @brief
     *   Allocates an array of type T.
     *
     * @tparam T
     *   The type of the array to allocate.
     *   Must be default constructable.
     *
     * @param num_elements
     *   The number of the elements the array contains.
     *
     * @param array_alignment
     *   What to align the array address to.
     *   Must be a power of two.
     *
     * @return T*
     *   The allocated array with num_elements size.
     *   nullptr if the request could not be fulfilled.
     */
    template<typename T>
    T* allocateArray(std::size_t num_elements, std::size_t array_alignment = alignof(T))
    {
      if (num_elements)
      {
        T* const array_data = allocateArrayTrivial<T>(num_elements, array_alignment);

        if (array_data)
        {
          T*       elements     = array_data;
          T* const elements_end = elements + num_elements;

          while (elements != elements_end)
          {
            new (elements++) T();
          }
        }

        return array_data;
      }

      return nullptr;
    }

    /*!
     * @brief
     *   Allocates an array of type T without calling default constructors.
     *
     * @tparam T
     *   The type of the array to allocate.
     *
     * @tparam
     *   SFINAE checking to make sure T is trivial.
     *
     * @param num_elements
     *   The number of the elements the array contains.
     *
     * @param array_alignment
     *   What to align the array address to.
     *   Must be a power of two.
     *
     * @return T*
     *   The allocated array with num_elements size.
     *   nullptr if the request could not be fulfilled.
     */
    template<typename T>
    T* allocateArrayTrivial(std::size_t num_elements, std::size_t array_alignment = alignof(T))
    {
      if (num_elements)
      {
        T* const array_data = static_cast<T*>(allocateAligned(sizeof(ArrayHeader), sizeof(T) * num_elements, array_alignment));

        if (array_data)
        {
          ArrayHeader* const header = static_cast<ArrayHeader*>(grabHeader(sizeof(ArrayHeader), array_data));
          header->size              = num_elements;
          header->alignment         = array_alignment;
        }

        return array_data;
      }

      return nullptr;
    }

    /*!
     * @brief
     *   Returns number of elements the array has.
     *
     * @tparam T
     *   The type of the array.
     *
     * @param ptr
     *   The array to get the size of.
     *   Must have been allocated with
     *     'IMemoryManager::allocateArray'        or
     *     'IMemoryManager::allocateArrayTrivial' or
     *     'IMemoryManager::arrayResize'.
     *
     * @return std::size_t
     *   The number of elements the array has.
     */
    template<typename T>
    static std::size_t arraySize(const T* const ptr) noexcept
    {
      return static_cast<ArrayHeader*>(grabHeader(sizeof(ArrayHeader), const_cast<T*>(ptr)))->size;
    }

    /*!
     * @brief
     *   Returns alignment of the array.
     *
     * @tparam T
     *   The type of the array.
     *
     * @param ptr
     *   The array to get the size of.
     *   Must have been allocated with
     *     'IMemoryManager::allocateArray'        or
     *     'IMemoryManager::allocateArrayTrivial' or
     *     'IMemoryManager::arrayResize'.
     *
     * @return std::size_t
     *   The number of elements the array has.
     */
    template<typename T>
    static std::size_t arrayAlignment(const T* const ptr) noexcept
    {
      return static_cast<ArrayHeader*>(grabHeader(sizeof(ArrayHeader), const_cast<T*>(ptr)))->alignment;
    }

    /*!
     * @brief
     *   Resizes an array potentially giving you a new allocation.
     *   Acts the same as the C stdlib's realloc.
     *   The new array has the old array's elements 'std::move'-d into them.
     *
     * @tparam T
     *   The type of the array.
     *
     * @param old_ptr
     *   The array that needs to be resized.
     *   If this is null then this acts as a malloc / IMemoryManager::allocateArray.
     *
     * @param num_elements
     *   The new size you want the array to be.
     *   If this size is 0 then this acts as free / IMemoryManager::deallocateArray.
     *   Otherwise a resize is attempted.
     *
     * @param array_alignment
     *   The alignment of the array.
     *
     * @return T*
     *   The allocated array with num_elements size.
     *   nullptr if the array was resized to 0 or resize could not happen.
     */
    template<typename T>
    T* arrayResize(T* const old_ptr, std::size_t num_elements, std::size_t array_alignment = alignof(T))
    {
      if (!old_ptr)  // NOTE(Shareef): if old_ptr is null act as malloc
      {
        return allocateArray<T>(num_elements, array_alignment);
      }

      if (num_elements == 0)  // NOTE(Shareef): if new_size is zero act as 'free'
      {
        deallocateArray(old_ptr);
      }
      else  // NOTE(Shareef): attempt to resize the allocation
      {
        ArrayHeader*      header   = static_cast<ArrayHeader*>(grabHeader(sizeof(ArrayHeader), old_ptr));
        const std::size_t old_size = header->size;

        if (num_elements > old_size)
        {
          T* const new_ptr = allocateArrayTrivial<T>(num_elements, array_alignment);

          if (new_ptr)
          {
            for (std::size_t i = 0; i < old_size; ++i)
            {
              new (new_ptr + i) T(std::move(old_ptr[i]));
            }

            deallocateArray(old_ptr);

            for (std::size_t i = old_size; i < num_elements; ++i)
            {
              new (new_ptr + i) T();
            }
          }

          return new_ptr;
        }

        // NOTE(Shareef): The allocation did not need to be resized.
        return old_ptr;
      }

      return nullptr;
    }

    /*!
     * @brief
     *   Frees the memory pointer to by \p ptr.
     *
     * @tparam T
     *   The type of the object to deallocate.
     *
     * @param ptr
     *   The array that needs to be freed.
     *   Freeing a nullptr is safe.
     *   Must have been allocated with
     *     'IMemoryManager::allocateArray'        or
     *     'IMemoryManager::allocateArrayTrivial' or
     *     'IMemoryManager::arrayResize'.
     */
    template<typename T>
    void deallocateArray(T* const ptr)
    {
      if (ptr)
      {
        const ArrayHeader* const header       = static_cast<ArrayHeader*>(grabHeader(sizeof(ArrayHeader), ptr));
        const T* const           elements_end = ptr + header->size;

        for (T* elements = ptr; elements != elements_end; ++elements)
        {
          elements->~T();
        }

        deallocateAligned(sizeof(ArrayHeader), ptr, header->size * sizeof(T), header->alignment);
      }
    }

   private:
    void*        allocateAligned(std::size_t header_size, std::size_t size, std::size_t alignment);
    static void* grabHeader(std::size_t header_size, void* ptr);
    void         deallocateAligned(std::size_t header_size, void* ptr, std::size_t size, std::size_t alignment);
  };

  /*!
   * @brief
   *   This is an abstract base class with some useful bounds checking functionality.
   */
  class MemoryManager : public IMemoryManager
  {
   public:
    template<typename T,
             typename = std::enable_if_t<std::is_convertible<T*, IMemoryManager*>::value>>
    static constexpr std::size_t header_size()
    {
      return T::header_size;
    }

   private:
    char* const m_MemoryBlockBegin;
    char* const m_MemoryBlockEnd;

   protected:
    MemoryManager(char* memory_block, std::size_t memory_block_size);

    ~MemoryManager() = default;

   public:
    MemoryManager(const MemoryManager& rhs) = delete;
    MemoryManager(MemoryManager&& rhs)      = delete;
    MemoryManager& operator=(const MemoryManager& rhs) = delete;
    MemoryManager& operator=(MemoryManager&& rhs) = delete;

    char*       begin() const { return m_MemoryBlockBegin; }
    char*       end() const { return m_MemoryBlockEnd; }
    std::size_t size() const { return end() - begin(); }
    void        checkPointer(const void* ptr) const;
  };

  class ScopedBuffer
  {
   private:
    IMemoryManager* m_Allocator;
    char*           m_Buffer;
    std::size_t     m_Size;

   public:
    // NOTE(SR):
    //   Make sure the passed in 'buffer' was allocated from 'allocator.allocate(...)'.
    ScopedBuffer(IMemoryManager& allocator, char* buffer, std::size_t size) noexcept :
      m_Allocator{&allocator},
      m_Buffer{buffer},
      m_Size{size}
    {
    }

    ScopedBuffer(IMemoryManager& allocator, std::size_t size) :
      ScopedBuffer(allocator, static_cast<char*>(allocator.allocate(size)), size)
    {
    }

    ScopedBuffer(std::nullptr_t) :
      m_Allocator{nullptr},
      m_Buffer{nullptr},
      m_Size{0u}
    {
    }

    // Disable copies

    ScopedBuffer(const ScopedBuffer& rhs) = delete;
    ScopedBuffer& operator=(const ScopedBuffer& rhs) = delete;

    // Move

    ScopedBuffer(ScopedBuffer&& rhs) noexcept :
      m_Allocator{std::exchange(rhs.m_Allocator, nullptr)},
      m_Buffer{std::exchange(rhs.m_Buffer, nullptr)},
      m_Size{std::exchange(rhs.m_Size, 0u)}
    {
    }

    ScopedBuffer& operator=(ScopedBuffer&& rhs) noexcept
    {
      if (this != &rhs)
      {
        m_Allocator = std::exchange(rhs.m_Allocator, nullptr);
        m_Buffer    = std::exchange(rhs.m_Buffer, nullptr);
        m_Size      = std::exchange(rhs.m_Size, 0u);
      }

      return *this;
    }

    // Accessors

    [[nodiscard]] IMemoryManager& allocator() const noexcept { return *m_Allocator; }
    [[nodiscard]] char*           buffer() const noexcept { return m_Buffer; }
    [[nodiscard]] std::size_t     size() const noexcept { return m_Size; }

    ~ScopedBuffer() noexcept
    {
      if (m_Allocator)
      {
        m_Allocator->deallocate(m_Buffer, m_Size);
      }
    }
  };

  template<typename T>
  struct FixedOwnedBuffer
  {
    IMemoryManager& memory;
    T*              buffer;

    FixedOwnedBuffer(IMemoryManager& memory) :
      memory{memory},
      buffer{nullptr}
    {
    }

    // TODO(SR): Implement move and copy. (disabled for now)
    FixedOwnedBuffer(const FixedOwnedBuffer& rhs) = delete;
    FixedOwnedBuffer& operator=(const FixedOwnedBuffer& rhs) = delete;
    FixedOwnedBuffer(FixedOwnedBuffer&& rhs)                 = delete;
    FixedOwnedBuffer& operator=(FixedOwnedBuffer&& rhs) = delete;

    std::size_t size() const { return buffer ? memory.arraySize(buffer) : 0u; }
    void        resize(std::size_t new_size) { buffer = memory.arrayResize(buffer, new_size); }

    ~FixedOwnedBuffer()
    {
      memory.deallocateArray(buffer);
    }
  };
}  // namespace bf

#endif /* BF_IMEMORY_MANAGER_HPP */

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2019-2022 Shareef Abdoul-Raheem

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
