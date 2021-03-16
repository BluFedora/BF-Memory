/******************************************************************************/
/*!
* @file   bf_allocators.cpp
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
#include "bf/memory/bf_allocators.hpp"

#include "bf/memory/bf_crt_allocator.hpp"  // CRTAllocator
#include "bf/memory/bf_memory_utils.h"     // bfMegabytes

namespace bf
{
  namespace
  {
    static CRTAllocator                          s_DefaultHeap = {};
    static FixedLinearAllocator<bfMegabytes(10)> s_DefaultTemp = {};
  }  // namespace

  GlobalAllocators g_DefaultAllocator = {&s_DefaultHeap, &s_DefaultTemp};
}  // namespace bf

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
