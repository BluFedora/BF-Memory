/******************************************************************************/
/*!
 * @file   alignment.cpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Utilities for handling alignment of memory allocations.
 *
 * @copyright Copyright (c) 2023 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#include "memory/alignment.hpp"

#include <cstdint>  // uintptr_t

bool Memory::IsPointerAligned(const void* const ptr, const MemoryIndex alignment) noexcept
{
  bfMemAssert(IsValidAlignment(alignment), "The alignment (%zu) must be a non-zero power of two less than %zu.", alignment, MaxAlignment);

  return (reinterpret_cast<std::uintptr_t>(ptr) & (alignment - 1u)) == 0u;
}

void* Memory::AlignPointer(const void* const ptr, const MemoryIndex alignment) noexcept
{
  bfMemAssert(IsValidAlignment(alignment), "The alignment (%zu) must be a non-zero power of two less than %zu.", alignment, MaxAlignment);

  const MemoryIndex required_alignment_mask = alignment - 1;

  return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(ptr) + required_alignment_mask & ~required_alignment_mask);
}

MemoryIndex Memory::PointerAlignOffset(const void* const ptr, const MemoryIndex alignment) noexcept
{
  const void* const aligned_ptr = Memory::AlignPointer(ptr, alignment);
  return static_cast<const unsigned char*>(aligned_ptr) - static_cast<const unsigned char*>(ptr);
}

/*
  Good read on the various implementations and performance characteristics:
    [https://github.com/KabukiStarship/KabukiToolkit/wiki/Fastest-Method-to-Align-Pointers#21-proof-by-example]
    [https://stackoverflow.com/a/51585463]
*/
void* Memory::StandardAlign(const size_t alignment, const size_t size, void** ptr, size_t* space) noexcept
{
  bfMemAssert(alignment > 0 && (alignment & (alignment - 1)) == 0, "The alignment must be a non-zero power of two.");
  bfMemAssert(ptr != NULL, "Passed in pointer must not be null.");
  bfMemAssert(space != NULL, "Passed in space must not be null.");

  void* const     aligned_ptr = Memory::AlignPointer(*ptr, alignment);
  const uintptr_t offset      = (char*)aligned_ptr - (char*)*ptr;

  if (*space >= (size + offset))
  {
    *ptr = aligned_ptr;
    *space -= offset;

    return aligned_ptr;
  }

  return NULL;
}

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2023 Shareef Abdoul-Raheem

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