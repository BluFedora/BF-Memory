/******************************************************************************/
/*!
 * @file   memory_api.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @brief
 *
 * @copyright Copyright (c) 2022-2025 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LibFoundation_Memory_API_HPP
#define LibFoundation_Memory_API_HPP

#include "memory/alignment.hpp"   // AlignPointer
#include "memory/allocation.hpp"  // Allocation API

#include <algorithm>    // move_backward
#include <iterator>     // make_reverse_iterator
#include <memory>       // uninitialized_move, uninitialized_move_n, destroy
#include <type_traits>  // is_trivially_destructible_v

namespace bf
{
  template<typename T>
  struct is_trivially_relocatable : public std::true_type
  {
  };

  template<typename T>
  inline constexpr bool is_trivially_relocatable_v = is_trivially_relocatable<T>::value;
}  // namespace bf

namespace Memory
{
  template<typename T>
  void ZeroObject(T* const object)
  {
    static_assert(!std::is_pointer_v<T>, "Pointer values should be assigned to nullptr explicitly.");
    SetBytes(object, 0x0, sizeof(T));
  }
}  // namespace Memory

template<typename SrcIterator, typename DstIterator>
DstIterator MemUninitializedMove(SrcIterator src_bgn, SrcIterator src_end, DstIterator dst_bgn)
{
  return std::uninitialized_move(src_bgn, src_end, dst_bgn);
}

template<typename SrcIterator, typename DstIterator>
DstIterator MemUninitializedMoveRev(SrcIterator src_bgn, SrcIterator src_end, DstIterator dst_end)
{
  return MemUninitializedMove(
          std::make_reverse_iterator(src_end),
          std::make_reverse_iterator(src_bgn),
          std::make_reverse_iterator(dst_end))
   .base();
}

namespace Memory
{
  // NOTE(SR): Assumes the buffer capacity >= (length + num_elements).
  template<typename T>
  T* InsertRange(T* const buffer, const MemoryIndex length, const MemoryIndex index, const MemoryIndex num_elements)
  {
    MemAssert(index <= length, "Invalid index.");

    // TODO(SR): Add optimized path for trivial types.

    const MemoryIndex total_elements_to_move  = length - index;
    const MemoryIndex num_uninitialized_moves = (total_elements_to_move < num_elements) ? total_elements_to_move : num_elements;

    T* const initialized_move_src_bgn = buffer + index;
    T* const initialized_move_src_end = buffer + (length - num_uninitialized_moves);
    T* const initialized_move_dst_end = initialized_move_src_end + num_elements;

    std::uninitialized_move_n(initialized_move_src_end, num_uninitialized_moves, initialized_move_dst_end);
    std::move_backward(initialized_move_src_bgn, initialized_move_src_end, initialized_move_dst_end);
    std::destroy_n(initialized_move_src_bgn, num_uninitialized_moves);

    return initialized_move_src_bgn;
  }

  template<typename T>
  void RemoveRange(T* const buffer, const MemoryIndex length, const MemoryIndex index, const MemoryIndex num_elements)
  {
    MemAssert((index + num_elements) <= length, "Invalid range.");

    // TODO(SR): Add optimized path for trivial types.

    T* const removed_bgn = buffer + index;
    T* const removed_end = removed_bgn + num_elements;
    T* const tail_end    = buffer + length;
    T* const destroy_bgn = tail_end - num_elements;

    std::move(removed_end, tail_end, removed_bgn);
    std::destroy(destroy_bgn, tail_end);
  }
}

#endif /* LibFoundation_Memory_API_HPP */

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2022-2025 Shareef Abdoul-Raheem

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
