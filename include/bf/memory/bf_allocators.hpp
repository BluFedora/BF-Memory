/******************************************************************************/
/*!
* @file   bf_allocators.hpp
* @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
* @brief
*   Defines some global default allocators for the program.
*
* @version 0.0.1
* @date    2021-03-15
*
* @copyright Copyright (c) 2021
*/
/******************************************************************************/
#ifndef BF_ALLOCATORS_HPP
#define BF_ALLOCATORS_HPP

#include "bf_imemory_manager.hpp"   // IMemoryManager
#include "bf_linear_allocator.hpp"  // LinearAllocator

namespace bf
{
  struct GlobalAllocators
  {
    IMemoryManager*  general_heap;
    LinearAllocator* temporary;
  };

  // If you reassign the default allocators you should do so before
  // running any code that uses them as that will cause a memory leak.
  extern thread_local GlobalAllocators g_DefaultAllocator;
}  // namespace bf

#endif /* BF_ALLOCATORS_HPP */

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2021 Shareef Abdoul-Raheem

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
