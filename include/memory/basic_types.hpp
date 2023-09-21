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
