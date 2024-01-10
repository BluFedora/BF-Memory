/******************************************************************************/
/*!
 * @file   default_heap.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *    The default heap allocator is guaranteed to be thread-safe.
 * 
 * @copyright Copyright (c) 2023 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_DEFAULT_HEAP_HPP
#define LIB_FOUNDATION_MEMORY_DEFAULT_HEAP_HPP

#include "basic_types.hpp"

#ifndef BF_MEMORY_NO_DEFAULT_HEAP
#define BF_MEMORY_NO_DEFAULT_HEAP 0  //!< Disables the default heap, can be implmented by a user defined translation unit.
#endif

#ifndef BF_MEMORY_DEBUG_HEAP
#define BF_MEMORY_DEBUG_HEAP 1 //!< The default heap allocator will have debug checks.
#endif

namespace Memory
{
  IAllocator& DefaultHeap() noexcept;
}

#endif  // LIB_FOUNDATION_MEMORY_DEFAULT_HEAP_HPP

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
