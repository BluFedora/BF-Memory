/******************************************************************************/
/*!
* @file   bf_crt_allocator.hpp
* @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
* @brief
*  This allocator is a wrapper around the built in memory allocator.
*  Implemented using "malloc / calloc" and "free".
*  TODO Look Into Alignment: [https://johanmabille.github.io/blog/2014/12/06/aligned-memory-allocator/]
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2021
*/
/******************************************************************************/
#ifndef BF_CRT_ALLOCATOR_HPP
#define BF_CRT_ALLOCATOR_HPP

#include "bf_imemory_manager.hpp"

namespace bf
{
  class CRTAllocator final : public IMemoryManager
  {
   public:
    CRTAllocator();

   public:
    void* allocate(std::size_t size) override;
    void  deallocate(void* ptr, std::size_t num_bytes) override;

    static constexpr std::size_t header_size = 0u;
  };
}  // namespace bifrost

#endif  /* BF_C_ALLOCATOR_HPP */

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2019-2021 Shareef Abdoul-Raheem

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
