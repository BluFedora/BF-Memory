/******************************************************************************/
/*!
 * @file   bf_memory_utils.h
 * @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
 * @date   2020-03-22
 * @brief
 *   Contains functions useful for low level memory manipulations.
 *
 * @copyright Copyright (c) 2020-2022
 */
/******************************************************************************/
#ifndef BF_MEMORY_UTILS_H
#define BF_MEMORY_UTILS_H

#include <stddef.h> /* size_t */

#if __cplusplus
extern "C" {
#endif

// clang-format off
#define bfBytes(n)     (n)
#define bfKilobytes(n) (bfBytes(n)     * 1024)
#define bfMegabytes(n) (bfKilobytes(n) * 1024)
#define bfGigabytes(n) (bfMegabytes(n) * 1024)
// clang-format on

/*!
 * @brief
 *   Aligns size to required_alignment.
 *
 * @param size
 *   The potentially unaligned size of an object.
 *
 * @param required_alignment
 *   Must be a non zero power of two.
 *
 * @return size_t
 *   The size of the object for the required alignment,
 */
size_t bfAlignUpSize(const size_t size, const size_t required_alignment);

/*!
 * @brief
 *   Moves `ptr` up a certain offset to be aligned.
 *
 * @param ptr
 *   The pointer you want aligned.
 *
 * @param required_alignment
 *   The desired alignment.
 *   Must be a non zero power of two.
 *
 * @return
 *   The newly aligned pointer.
 */
void* bfAlignUpPointer(const void* const ptr, const size_t required_alignment);

/*!
 * @brief
 *   Implements "std::align" but in C.
 *   `size` must be <= *space or ths functions will always fail.
 *
 * @param alignment[in]
 *   The alignment that you are wanting the pointer to be at.
 *   Must be a non zero power of two.
 *
 * @param size[in]
 *   The size of the block of that needs to be aligned.
 *
 * @param ptr[in/out]
 *   The pointer ot be aligned, will be aligned
 *   if this function succeeds otherwise unchanged.
 *
 * @param space[in/out]
 *   The size of the block of memory available to align memory to.
 *   if this function succeeds the space left in the allocation
 *   otherwise left unchanged.
 *
 * @return
 *   `ptr` is we can fit an aligned `size` into `space`,
 *   otherwise NULL is returned.
 */
void* bfStdAlign(size_t alignment, size_t size, void** ptr, size_t* space);

#if __cplusplus
}
#endif

#endif /* BF_MEMORY_UTILS_H */

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2020-2022 Shareef Abdoul-Raheem

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
