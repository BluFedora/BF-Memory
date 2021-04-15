/******************************************************************************/
/*!
* @file   bf_stack_allocator.hpp
* @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
* @brief
*   This allocator is a designed for allocations where you can guarantee
*   deallocation is in a LIFO (Last in First Out) order in return you get
*   some speed.
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2021
*/
/******************************************************************************/
#ifndef BF_STACK_ALLOCATOR_HPP
#define BF_STACK_ALLOCATOR_HPP

#include "bf_imemory_manager.hpp" /* MemoryManager */

namespace bf
{
  class StackAllocator final : public MemoryManager
  {
   private:
    char*       m_StackPtr;
    std::size_t m_MemoryLeft;

   public:
    StackAllocator(char* memory_block, std::size_t memory_size);

    std::size_t usedMemory() const { return size() - m_MemoryLeft; }
    void*       allocate(std::size_t size) override;
    void        deallocate(void* ptr, std::size_t num_bytes) override;

   private:
    class StackHeader
    {
     public:
      std::size_t block_size;
      std::size_t align_size;
    };

   public:
    static constexpr std::size_t header_size = sizeof(StackHeader);
  };
}  // namespace bifrost

#endif /* BF_STACK_ALLOCATOR_HPP */

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
