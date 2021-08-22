################################################################################
#                                                                              #
#                              BF MEMORY PROJECT                               #
#                                                                              #
################################################################################
###  CMakeList.txt : Custom Memory Allococator Library For C++14 or above.   ###
################################################################################

cmake_minimum_required(VERSION 3.12)

cmake_policy(PUSH)
cmake_policy(SET CMP0076 NEW)

project(BF_Memory VERSION 0.1.3 DESCRIPTION "Custom allocator interface for various allocation schemes.")

add_library(BF_Memory STATIC "")

target_include_directories(
  BF_Memory
  PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)

target_sources(
  BF_Memory
  
  PUBLIC
    "include/bf/memory/bf_allocators.hpp"
    "include/bf/memory/bf_crt_allocator.hpp"
    "include/bf/memory/bf_freelist_allocator.hpp"
    "include/bf/memory/bf_imemory_manager.hpp"
    "include/bf/memory/bf_linear_allocator.hpp"
    "include/bf/memory/bf_memory_utils.h"
    "include/bf/memory/bf_pool_allocator.hpp"
    "include/bf/memory/bf_proxy_allocator.hpp"
    "include/bf/memory/bf_stack_allocator.hpp"
    "include/bf/memory/bf_stl_allocator.hpp"

  PRIVATE
    "src/bf_allocators.cpp"
    "src/bf_crt_allocator.cpp"
    "src/bf_freelist_allocator.cpp"
    "src/bf_imemory_manager.cpp"
    "src/bf_linear_allocator.cpp"
    "src/bf_memory_utils.c"
    "src/bf_pool_allocator.cpp"
    "src/bf_proxy_allocator.cpp"
    "src/bf_stack_allocator.cpp"
)

target_compile_features(
  BF_Memory
  PUBLIC
    cxx_std_14
)

cmake_policy(POP)
