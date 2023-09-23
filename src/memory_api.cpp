#include "bf/memory/memory_api.hpp"

#include <cstdarg>  // va_list, va_start, va_end
#include <cstdio>   // vsnprintf, stderr,
#include <cstring>  // memset
#include <limits>   // numeric_limits

//-------------------------------------------------------------------------------------//
// Utilities Interface
//-------------------------------------------------------------------------------------//

void bfMemCopy(void* const dst, const void* const src, std::size_t num_bytes)
{
  std::memcpy(dst, src, num_bytes);
}

void bfMemSet(void* const dst, const unsigned char value, std::size_t num_bytes)
{
  std::memset(dst, value, num_bytes);
}
