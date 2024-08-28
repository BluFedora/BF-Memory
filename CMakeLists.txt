################################################################################
### BF Memory: Polymorphic Memory Allocator Library For C++17 and above.     ###
################################################################################

cmake_minimum_required(VERSION 3.12)

project(LibFoundation_Memory VERSION 1.0.0 DESCRIPTION "Custom allocator interface for various allocation schemes.")

add_library(
  LibFoundation_Memory 
    STATIC
      # Headers
      "include/memory/alignment.hpp"
      "include/memory/allocation.hpp"
      "include/memory/assertion.hpp"
      "include/memory/basic_types.hpp"
      "include/memory/default_heap.hpp"
      "include/memory/fixed_st_allocators.hpp"
      "include/memory/fixed_mt_allocators.hpp"
      "include/memory/growing_st_allocators.hpp"
      "include/memory/memory_api.hpp"
      "include/memory/scoped_buffer.hpp"
      "include/memory/smart_pointer.hpp"
      "include/memory/stl_allocator.hpp"

      # Sources
      "src/assertion.cpp"
      "src/alignment.cpp"
      "src/default_heap.cpp"
      "src/fixed_st_allocators.cpp"
      "src/fixed_mt_allocators.cpp"
      "src/growing_st_allocators.cpp"
      "src/memory_api.cpp"
)

target_include_directories(
  LibFoundation_Memory
  PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)

target_compile_features(
  LibFoundation_Memory
  PUBLIC
    cxx_std_17
)

set_target_properties(
  LibFoundation_Memory
  PROPERTIES
    FOLDER                   "BluFedora/Foundation"
    CXX_STANDARD             17
    CXX_STANDARD_REQUIRED    on
    CXX_EXTENSIONS           off
    COMPILE_WARNING_AS_ERROR on
)

target_compile_options(
  LibFoundation_Memory
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>                             # /WX
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic> # -Werror
)
