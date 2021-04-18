/******************************************************************************/
/*!
* @file   bf_pool_allocator.hpp
* @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
* @brief
*   This allocator is a designed for static (known at compile time)
*   pools of objects. Features O(1) allocation and O(1) deletion.
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2021
*/
/******************************************************************************/
#ifndef BF_POOL_ALLOCATOR_HPP
#define BF_POOL_ALLOCATOR_HPP

#include "bf_imemory_manager.hpp"

namespace bf
{
  namespace detail
  {
    template<std::size_t size_of_t, std::size_t alignment>
    static constexpr std::size_t aligned_size()
    {
      return ((size_of_t + alignment - 1) / alignment) * alignment;
    }
  }  // namespace detail

  class PoolAllocatorImpl : public MemoryManager
  {
   protected:
    class PoolHeader
    {
     public:
      PoolHeader* next;
    };

   public:
    static constexpr std::size_t header_size = sizeof(PoolHeader);

   private:
    PoolHeader* m_PoolStart;
    std::size_t m_BlockSize;

   public:
    PoolAllocatorImpl(char* memory_block, std::size_t memory_block_size, std::size_t sizeof_block, std::size_t alignof_block);

    void*       allocate(std::size_t size) override final;
    void        deallocate(void* ptr, std::size_t num_bytes) override final;
    std::size_t indexOf(const void* ptr) const;
    void*       fromIndex(std::size_t index);  // The index must have been from 'indexOf'
    void        reset();

    std::size_t capacity() const
    {
      return size() / m_BlockSize;
    }
  };

  template<typename T, std::size_t num_elements>
  class PoolAllocator final : public PoolAllocatorImpl
  {
   private:
    template<size_t arg1, size_t... others>
    struct static_max;

    template<size_t arg>
    struct static_max<arg>
    {
      static constexpr size_t value = arg;
    };

    template<size_t arg1, size_t arg2, size_t... others>
    struct static_max<arg1, arg2, others...>
    {
      static constexpr size_t value = arg1 >= arg2 ? static_max<arg1, others...>::value : static_max<arg2, others...>::value;
    };

   public:
    static constexpr std::size_t header_size       = PoolAllocatorImpl::header_size;
    static constexpr std::size_t alignment_req     = static_max<alignof(T), alignof(PoolHeader)>::value;
    static constexpr std::size_t allocation_size   = static_max<sizeof(T), header_size>::value;
    static constexpr std::size_t pool_stride       = detail::aligned_size<allocation_size, alignment_req>();
    static constexpr std::size_t memory_block_size = pool_stride * num_elements;

   private:
    char m_AllocBlock[memory_block_size];

   public:
    // NOTE(SR):
    //   `m_AllocBlock` is not initialized by design since the `PoolAllocatorImpl`
    //   ctor does the setup.
    //
    // ReSharper disable once CppPossiblyUninitializedMember
    PoolAllocator() :
      PoolAllocatorImpl{m_AllocBlock, memory_block_size, sizeof(T), alignof(T)}
    {
    }
  };
}  // namespace bf

#endif /* BF_POOL_ALLOCATOR_HPP */

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
