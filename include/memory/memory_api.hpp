/******************************************************************************/
/*!
 * @file   memory_api.hpp
 * @author Shareef Raheem (https://blufedora.github.io/)
 * @date   2022-09-12
 * @brief
 *
 * @copyright Copyright (c) 2022-2023 Shareef Abdoul-Raheem
 */
/******************************************************************************/
#ifndef LibFoundation_Memory_API_HPP
#define LibFoundation_Memory_API_HPP

#include "memory/allocation.hpp"  // Allocation API

#include "memory/alignment.hpp"  // AlignPointer

#include <iterator>     // make_reverse_iterator
#include <memory>       // uninitialized_default_construct, uninitialized_value_construct, uninitialized_move
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

//-------------------------------------------------------------------------------------//
// Utilities Interface
//-------------------------------------------------------------------------------------//

// TODO(SR): Add ZeroStruct and ByteCopyStruct.

/*!
 * @brief
 *   Same as std::memcpy.
 *   Byte by byte copy from \p src to \p dst.
 *
 * @param dst
 *   The destination of the byte copy, must be at least \p num_bytes in size.
 *
 * @param src
 *   The source of the byte copy, must be at least \p num_bytes in size.
 *
 * @param num_bytes
 *   The number of bytes to copy from \p src to \p dst.
 */
void bfMemCopy(void* const dst, const void* const src, std::size_t num_bytes);

void bfMemSet(void* const dst, const unsigned char value, std::size_t num_bytes);

template<typename T>
constexpr void bfMemDestruct(T* const ptr)
{
  std::destroy_at(ptr);
}

template<typename Iterator>
constexpr void bfMemDestructRange(const Iterator bgn, const Iterator end)
{
  using Traits     = std::iterator_traits<Iterator>;
  using value_type = typename Traits::value_type;

  if constexpr (!std::is_trivially_destructible_v<value_type>)
  {
    std::destroy(bgn, end);
  }
}

template<typename SrcIterator, typename DstIterator>
DstIterator bfMemUninitializedMove(SrcIterator src_bgn, SrcIterator src_end, DstIterator dst_bgn)
{
  return std::uninitialized_move(src_bgn, src_end, dst_bgn);
}

template<typename SrcIterator, typename DstIterator>
DstIterator bfMemUninitializedMoveRev(SrcIterator src_bgn, SrcIterator src_end, DstIterator dst_end)
{
  return bfMemUninitializedMove(
          std::make_reverse_iterator(src_end),
          std::make_reverse_iterator(src_bgn),
          std::make_reverse_iterator(dst_end))
   .base();
}

#endif /* LibFoundation_Memory_API_HPP */

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2022-2023 Shareef Abdoul-Raheem

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
