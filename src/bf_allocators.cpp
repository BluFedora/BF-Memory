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

#include <cassert>

namespace bf
{
  thread_local MemoryContext* g_MemCtx = nullptr;

  namespace
  {
    static CRTAllocator                                      s_DefaultHeap   = {};
    static thread_local FixedLinearAllocator<bfMegabytes(5)> s_DefaultTemp   = {};
    static thread_local MemoryContext                        s_DefaultMemCtx = {};
  }  // namespace

  MemoryContext::MemoryContext() :
    parent_ctx{g_MemCtx},
    general_heap{parent_ctx ? parent_ctx->general_heap : &s_DefaultHeap},
    temp_heap{parent_ctx ? parent_ctx->temp_heap : &s_DefaultTemp}
  {
    g_MemCtx = this;
  }

  MemoryContext::~MemoryContext()
  {
    assert(g_MemCtx == this);
    g_MemCtx = parent_ctx;
  }

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
