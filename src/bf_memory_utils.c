/******************************************************************************/
/*!
 * @file   bf_memory_utils.c
 * @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
 * @brief
 *   Contains functions useful for low level memory manipulations.
 *
 * @version 0.0.1
 * @date    2020-03-22
 *
 * @copyright Copyright (c) 2020-2021
 */
/******************************************************************************/
#include "bf/memory/bf_memory_utils.h"

#include <assert.h> /* assert */
#include <string.h> /* memcpy */

#define bfCast(e, T) ((T)(e))

size_t bfAlignUpSize(const size_t size, const size_t required_alignment)
{
  assert(required_alignment > 0 && (required_alignment & (required_alignment - 1)) == 0 && "bfAlignUpSize:: The alignment must be a non-zero power of two.");

  const size_t required_alignment_mask = required_alignment - 1;

  return size + required_alignment_mask & ~required_alignment_mask;
}

void* bfAlignUpPointer(const void* const ptr, const size_t required_alignment)
{
  assert(required_alignment > 0 && (required_alignment & (required_alignment - 1)) == 0 && "bfAlignUpPointer:: The alignment must be a non-zero power of two.");

  const size_t required_alignment_mask = required_alignment - 1;

  return bfCast(bfCast(ptr, uintptr_t) + required_alignment_mask & ~required_alignment_mask, void*);
}

/*
  Good read on the various implementations and performance characteristics:
    [https://github.com/KabukiStarship/KabukiToolkit/wiki/Fastest-Method-to-Align-Pointers#21-proof-by-example]
*/
void* bfStdAlign(size_t alignment, size_t size, void** ptr, size_t* space)
{
  assert(alignment > 0 && (alignment & (alignment - 1)) == 0 && "The alignment must be a non-zero power of two.");
  assert(ptr && "Passed in pointer must not be null.");
  assert(space && "Passed in space must not be null.");

  void* const     aligned_ptr = bfAlignUpPointer(*ptr, alignment);
  const uintptr_t offset      = bfCast(aligned_ptr, char*) - bfCast(*ptr, char*);

  if (*space >= (size + offset))
  {
    *ptr = aligned_ptr;
    *space -= offset;

    return aligned_ptr;
  }

  return NULL;
}

#undef bfCast

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2020-2021 Shareef Abdoul-Raheem

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
