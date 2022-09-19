/******************************************************************************/
/*!
 * @file   crt_allocator.hpp
 * @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
 * @date   2019-12-26
 * @brief
 *  This allocator is a wrapper around the built in memory allocator.
 *  Implemented using "malloc" and "free".
 *
 * @copyright Copyright (c) 2019-2022
 */
/******************************************************************************/
#ifndef BF_CRT_ALLOCATOR_HPP
#define BF_CRT_ALLOCATOR_HPP

#include "memory_api.hpp"

namespace bf
{
  struct CRTAllocator : public IAllocator
  {
    CRTAllocator();
  };
}  // namespace bf

#endif /* BF_CRT_ALLOCATOR_HPP */

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
