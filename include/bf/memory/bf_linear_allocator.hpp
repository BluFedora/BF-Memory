/******************************************************************************/
/*!
* @file   bf_linear_allocator.hpp
* @author Shareef Abdoul-Raheem (https://blufedora.github.io/)
* @brief
*   This allocator is very good for temporary memory allocations throughout
*   the frame. There is no individual deallocation but a whole clear operation
*   that should happen at the beginning (or end) of each 'frame'.
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2021
*/
/******************************************************************************/
#ifndef BF_LINEAR_ALLOCATOR_HPP
#define BF_LINEAR_ALLOCATOR_HPP

#include "bf_imemory_manager.hpp" /* MemoryManager */

namespace bf
{
  class LinearAllocatorSavePoint;

  class LinearAllocator : public MemoryManager
  {
    friend class LinearAllocatorSavePoint;

   public:
    static constexpr std::size_t header_size = 0u;

   private:
    std::size_t m_MemoryOffset;

   public:
    LinearAllocator(void* memory_block, std::size_t memory_block_size);

    std::size_t usedMemory() const { return m_MemoryOffset; }

    void  clear(void);
    void* allocate(std::size_t size) override final;
    void  deallocate(void* ptr, std::size_t num_bytes) override final;

    virtual ~LinearAllocator() = default;

   private:
    char* currentBlock() const;
  };

  template<std::size_t k_BufferSize>
  class FixedLinearAllocator final : public LinearAllocator
  {
   private:
    char m_FixedBuffer[k_BufferSize];

   public:
    FixedLinearAllocator() :
      LinearAllocator(m_FixedBuffer, k_BufferSize),
      m_FixedBuffer{}
    {
    }
  };

  class LinearAllocatorSavePoint final
  {
   private:
    LinearAllocator* m_Allocator;
    std::size_t      m_OldOffset;

   public:
    void save(LinearAllocator& allocator);
    void restore();
  };

  class LinearAllocatorScope final
  {
   private:
    LinearAllocatorSavePoint m_SavePoint;

   public:
    LinearAllocatorScope(LinearAllocator& allocator) :
      m_SavePoint{}
    {
      m_SavePoint.save(allocator);
    }

    ~LinearAllocatorScope()
    {
      m_SavePoint.restore();
    }

    LinearAllocatorScope(const LinearAllocatorScope& rhs)     = delete;
    LinearAllocatorScope(LinearAllocatorScope&& rhs) noexcept = delete;
    LinearAllocatorScope& operator=(const LinearAllocatorScope& rhs) = delete;
    LinearAllocatorScope& operator=(LinearAllocatorScope&& rhs) noexcept = delete;
  };
}  // namespace bf

#endif /* BF_LINEAR_ALLOCATOR_HPP */

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
