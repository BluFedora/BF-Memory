/******************************************************************************/
/*!
 * @file   scoped_buffer.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   RAII Memory managed buffer.
 *
 * @todo ScopedBuffer Implement move and copy constructors and assignment.
 * @todo ScopedBuffer define destruction sematics.
 *
 * @copyright Copyright (c) 2022-2023 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef BF_SCOPED_BUFFER_HPP
#define BF_SCOPED_BUFFER_HPP

#include "memory_api.hpp"

#include "memory/default_heap.hpp" // DefaultHeap

namespace bf
{
  /*!
   * @brief
   *   RAII Managed buffer of memory.
   */
  template<typename T>
  struct ScopedBuffer
  {
    IPolymorphicAllocator& memory;
    T*          buffer;
    std::size_t num_elements;

    ScopedBuffer(IPolymorphicAllocator& memory = Memory::DefaultHeap(), std::size_t in_num_elements = 0u) :
      memory{memory},
      buffer{nullptr},
      num_elements{0u}
    {
      resize(in_num_elements);
    }

    // TODO(SR): Implement move and copy. (disabled for now)
    ScopedBuffer(const ScopedBuffer& rhs)            = delete;
    ScopedBuffer& operator=(const ScopedBuffer& rhs) = delete;
    ScopedBuffer(ScopedBuffer&& rhs)                 = delete;
    ScopedBuffer& operator=(ScopedBuffer&& rhs)      = delete;

    std::size_t size() const { return num_elements; }
    T*          begin() { return buffer; }
    T*          end() { return buffer + num_elements; }
    const T*    begin() const { return buffer; }
    const T*    end() const { return buffer + num_elements; }

    T& operator[](const std::size_t index)
    {
      bfMemAssert(index < num_elements, "Out of bounds index (%i >= %i).\n", int(index), int(num_elements));
      return buffer[index];
    }

    const T& operator[](const std::size_t index) const
    {
      bfMemAssert(index < num_elements, "Out of bounds index (%i >= %i).\n", int(index), int(num_elements));
      return buffer[index];
    }

    // Returns true on a successful resize.
    template<Memory::ArrayConstruct new_element_init = Memory::ArrayConstruct::UNINITIALIZE>
    bool resize(const std::size_t new_size)
    {
      if (num_elements != new_size)
      {
        T* const new_buffer = bfMemAllocateArray<T, Memory::ArrayConstruct::UNINITIALIZE>(memory, new_size);

        if (new_buffer || new_size == 0u)
        {
          const std::size_t num_move_elements   = num_elements < new_size ? num_elements : new_size;
          T* const          new_buffer_move_end = std::uninitialized_move(buffer, buffer + num_move_elements, new_buffer);
          bfMemDeallocateArray(memory, buffer, num_elements);

          const T* const    new_buffer_end   = new_buffer + new_size;
          const std::size_t num_new_elements = (new_buffer_end - new_buffer_move_end);

          bfMemArrayConstruct<T, new_element_init>({new_buffer_move_end, num_new_elements * sizeof(T)}, num_new_elements);

          num_elements = new_size;
          buffer       = new_buffer;

          return true;
        }
      }

      return false;
    }

    void destroy()
    {
      resize(0u);
    }

    ~ScopedBuffer() { destroy(); }
  };
}  // namespace bf

#endif  // BF_SCOPED_BUFFER_HPP

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
