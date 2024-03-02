# BluFedora Memory Library

Basic polymorphic memory allocator library for C++17.

## Preprocessor Options


| Define                      | default value |
| --------------------------- | ------------- |
| `BF_MEMORY_ASSERTIONS`      | `1`           |
| `BF_MEMORY_ALLOCATION_INFO` | `1`           |
| `BF_MEMORY_NO_DEFAULT_HEAP` | `0`           |
| `BF_MEMORY_DEBUG_HEAP`      | `1`           |

# Build Requirements

- C++17 or above

## Standard Library Features Used

| Header | feature(s) |
| ---- | ---- |
| `#include <atomic>` | `std::atomic<byte*>` |
| `#include <cstdarg>` | `va_list, va_start, va_end` |
| `#include <cstddef>` | `max_align_t` |
| `#include <cstdint>` | `uintptr_t, ptrdiff_t` |
| `#include <cstdio>` | `vsnprintf, stderr` |
| `#include <cstdlib>` | `abort` |
| `#include <cstring>` | `memset, memcpy` |
| `#include <iterator>` | `make_reverse_iterator` |
| `#include <memory>` | `uninitialized_move, shared_ptr, allocate_shared, unique_ptr` |
| `#include <new>` | `'placement-new' align_val_t, nothrow` |
| `#include <type_traits>` | `is_trivially_destructible_v, true_type, is_array_v, is_bounded_array_v, is_unbounded_array_v, enable_if_t` |
| `#include <utility>` | `forward, move, exchange` |

## Good Reads On Memory Allocators

- [malloc() and free() are a bad API by **Jonathan Müller**](https://www.foonathan.net/2022/08/malloc-interface/)
- [Untangling Lifetimes: The Arena Allocator by **Ryan Fluery**](https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator)
- [Memory Allocation Strategies by **gingerBill**](https://www.gingerbill.org/series/memory-allocation-strategies/)
