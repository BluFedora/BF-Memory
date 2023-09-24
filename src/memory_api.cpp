#include "bf/memory/memory_api.hpp"

#include <cstring>  // memset, memcpy

void bfMemCopy(void* const dst, const void* const src, std::size_t num_bytes)
{
  std::memcpy(dst, src, num_bytes);
}

void bfMemSet(void* const dst, const unsigned char value, std::size_t num_bytes)
{
  std::memset(dst, value, num_bytes);
}
