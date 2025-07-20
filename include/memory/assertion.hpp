/******************************************************************************/
/*!
 * @file   assertion.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Optional extra checking of function preconditions.
 *
 * @copyright Copyright (c) 2023-2025 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_ASSERTION_HPP
#define LIB_FOUNDATION_MEMORY_ASSERTION_HPP

#ifndef BF_MEMORY_ASSERTIONS
#define BF_MEMORY_ASSERTIONS 1  //!< Extra checks for function preconditions, disable for shipping build.
#endif

#if defined(_MSC_VER)  // msvc
#define MemInvariant(condition) __assume(condition)
#elif defined(__clang__)  // clang
#define MemInvariant(condition) __builtin_assume(condition)
#elif defined(__GNUC__)  // GCC
#define MemInvariant(condition) \
  do {                          \
    if (!(condition))           \
    {                           \
      __builtin_unreachable();  \
    }                           \
  } while (0)
#else
#define MemInvariant(condition) ((void)0)
#endif

#if BF_MEMORY_ASSERTIONS
void MemAssertImpl(const char* const expr_str, const char* const filename, const int line_number, const char* const assert_msg, ...);

#define MemAssert(expr, msg, ...) ((!(expr)) ? ::MemAssertImpl(#expr, __FILE__, __LINE__, (msg), ##__VA_ARGS__) : (void)(0))
#else
#define MemAssert(expr, ...) MemInvariant(expr)
#endif

#endif  // LIB_FOUNDATION_MEMORY_ASSERTION_HPP

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
