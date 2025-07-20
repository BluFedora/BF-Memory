/******************************************************************************/
/*!
 * @file   alignment.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *   Utilities for handling alignment of memory allocations.
 *
 * @copyright Copyright (c) 2023-2025 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LIB_FOUNDATION_MEMORY_ALIGHNMENT_HPP
#define LIB_FOUNDATION_MEMORY_ALIGHNMENT_HPP

#include "assertion.hpp"    // MemAssert
#include "basic_types.hpp"  // MemoryIndex

#include <cstddef>  // max_align_t

namespace Memory
{
  inline constexpr MemoryIndex DefaultMallocAlignment = alignof(std::max_align_t); //!< The (minimum) alignment a pointer from malloc will have.

#if defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__)
  inline constexpr MemoryIndex DefaultNewAlignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__; //!< The alignment a pointer from new will have.
#else
  inline constexpr MemoryIndex DefaultNewAlignment = DefaultMallocAlignment; //!< The alignment a pointer from new will have.
#endif


  inline constexpr MemoryIndex DefaultAlignment = DefaultMallocAlignment < DefaultNewAlignment ? DefaultNewAlignment : DefaultMallocAlignment; //!< An address aligned to this value can support any non-overaligned datatype.

  /*!
   * @brief
   *   Returns if \p alignment is valid to be used for alignment of memory addresses.
   * 
   * @param alignment
   *   The value to be checking.
   * 
   * @return
   *   true of \p alignment is a non-zero power of two.
  */
  constexpr bool IsValidAlignment(const MemoryIndex alignment) noexcept
  {
    return alignment > 0 && (alignment & (alignment - 1)) == 0;
  }

  /*!
   * @brief
   *   returns if \p size is a multiple of \p alignment.
   * 
   * @param size
   *   The size to be checking.
   * 
   * @param alignment
   *   Must be a non-zero power of two.
   * 
   * @return
   *   true if \p size is aligned to \p alignment, false otherwise.
  */
  constexpr MemoryIndex IsSizeAligned(const MemoryIndex size, const MemoryIndex alignment) noexcept
  {
    MemAssert(IsValidAlignment(alignment), "The alignment (%zu) must be a non-zero power of two.", alignment);

    return (size & (alignment - 1u)) == 0;
  }

  /*!
   * @brief
   *   Aligns size to required_alignment.
   *
   * @param size
   *   The potentially unaligned size of an object.
   *
   * @param alignment
   *   Must be a non zero power of two.
   *
   * @return MemoryIndex
   *   The size of the object for the required alignment,
   */
  constexpr MemoryIndex AlignSize(const MemoryIndex size, const MemoryIndex alignment) noexcept
  {
    MemAssert(IsValidAlignment(alignment), "The alignment (%zu) must be a non-zero power of two.", alignment);

    const MemoryIndex required_alignment_mask = alignment - 1;

    return size + required_alignment_mask & ~required_alignment_mask;
  }

  bool IsPointerAligned(const void* const ptr, const MemoryIndex alignment) noexcept;

  /*!
   * @brief
   *   Moves `ptr` up a certain offset to be aligned.
   *
   * @param ptr
   *   The pointer you want aligned.
   *
   * @param alignment
   *   The desired alignment.
   *   Must be a non zero power of two.
   *
   * @return void*
   *   The newly aligned pointer.
   */
  void* AlignPointer(const void* const ptr, const MemoryIndex alignment) noexcept;

  /*!
   * @brief
   *  Returns the number of bytes needed to align \p ptr to the next \p alignment aligned address.
   *
   * @param ptr
   *   The pointer to align.
   *
   * @param alignment
   *   The desired alignment.
   *
   * @return
   *   The number of bytes needed to align \p ptr to the next \p alignment aligned address.
   */
  MemoryIndex PointerAlignOffset(const void* const ptr, const MemoryIndex alignment) noexcept;

  /*!
   * @brief
   *   Implements "std::align" but in a C compatible way.
   *   `size` must be <= *space or this functions will always fail.
   *
   * @param[in] alignment
   *   The alignment that you are wanting the pointer to be at.
   *   Must be a non zero power of two.
   *
   * @param[in] size
   *   The size of the block of that needs to be aligned.
   *
   * @param[in,out] ptr
   *   The pointer ot be aligned, will be aligned
   *   if this function succeeds otherwise unchanged.
   *
   * @param[in,out] space
   *   The size of the block of memory available to align memory to.
   *   if this function succeeds the space left in the allocation
   *   otherwise left unchanged.
   *
   * @return
   *   `ptr` is we can fit an aligned `size` into `space`,
   *   otherwise NULL is returned.
   */
  void* StandardAlign(const MemoryIndex alignment, const MemoryIndex size, void** ptr, MemoryIndex* space) noexcept;
}  // namespace Memory

#endif  // LIB_FOUNDATION_MEMORY_ALIGHNMENT_HPP

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
