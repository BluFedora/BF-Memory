#include "memory/memory_api.hpp"

#include <cstring>  // memset, memcpy

void Memory::CopyBytes(void* const dst, const void* const src, const MemoryIndex num_bytes)
{
  std::memcpy(dst, src, num_bytes);
}

void Memory::SetBytes(void* const dst, const byte value, const MemoryIndex num_bytes)
{
  std::memset(dst, value, num_bytes);
}
