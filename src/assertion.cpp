/******************************************************************************/
/*!
 * @file   assertion.cpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Optional extra checking of function preconditions.
 *
 * @copyright Copyright (c) 2023-2025 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#include "memory/assertion.hpp"

#if BF_MEMORY_ASSERTIONS

#include <cstdarg>  // va_list, va_start, va_end
#include <cstdio>   // vsnprintf, stderr
#include <cstdlib>  // abort

void MemAssertImpl(const char* const expr_str, const char* const filename, const int line_number, const char* const assert_msg, ...)
{
  char assert_message_buffer[256];

  std::va_list args;
  va_start(args, assert_msg);
  std::vsnprintf(assert_message_buffer, sizeof(assert_message_buffer), assert_msg, args);
  va_end(args);

  std::fprintf(stderr, "Memory[%s:%i] Assertion '%s' failed, %s.\n", filename, line_number, expr_str, assert_message_buffer);
  std::abort();
}

#endif

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2023-2025 Shareef Abdoul-Raheem

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
