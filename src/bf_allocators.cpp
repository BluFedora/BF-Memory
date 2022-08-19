/******************************************************************************/
/*!
 * @file   bf_allocators.cpp
 * @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
 * @date   2021-03-15
 * @brief
 *   Defines global allocators for the program.
 *
 * @copyright Copyright (c) 2021-2022
 */
/******************************************************************************/
#include "bf/memory/bf_allocators.hpp"

#include "bf/memory/bf_crt_allocator.hpp"  // CRTAllocator
#include "bf/memory/bf_memory_utils.h"     // bfKilobytes

#include <cstdlib>  // abort
#include <utility>  // exchange

namespace bf
{
  static void memoryAssert(const bool condition)
  {
    if (!condition)
    {
      std::abort();
    }
  }

  static MemoryContext*& initMemCtx(MemoryContext*& mem_ctx)
  {
    if (!mem_ctx)
    {
      static CRTAllocator                                        s_DefaultHeap   = {};
      static thread_local FixedLinearAllocator<bfKilobytes(512)> s_DefaultTemp   = {};
      static thread_local MemoryContext                          s_DefaultMemCtx = {&s_DefaultHeap, &s_DefaultTemp};
    }

    return mem_ctx;
  }

  static MemoryContext*& getMemContextFast()
  {
    static thread_local MemoryContext* s_MemCtx = nullptr;

    return s_MemCtx;
  }

  static MemoryContext*& getMemContext()
  {
    return initMemCtx(getMemContextFast());
  }

  MemoryContext::MemoryContext(IMemoryManager* general_heap, LinearAllocator* temp_heap) :
    parent_ctx{std::exchange(getMemContext(), this)},
    general_heap{general_heap ? general_heap : parent_ctx->general_heap},
    temp_heap{temp_heap ? temp_heap : parent_ctx->temp_heap}
  {
  }

  MemoryContext::~MemoryContext()
  {
    auto& global_ctx = getMemContextFast();
    memoryAssert(global_ctx == this);
    global_ctx = parent_ctx;
  }

  MemoryContext& ParentMemoryContext()
  {
    const auto& global_ctx = getMemContext();

    memoryAssert(global_ctx->parent_ctx != nullptr);
    return *global_ctx->parent_ctx;
  }

  IMemoryManager&  GeneralHeap() { return *getMemContext()->general_heap; }
  LinearAllocator& TempHeap() { return *getMemContext()->temp_heap; }
}  // namespace bf

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2021-2022 Shareef Abdoul-Raheem

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
