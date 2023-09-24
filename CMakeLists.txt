################################################################################
#                                                                              #
#                              BF MEMORY PROJECT                               #
#                                                                              #
################################################################################
###         Polymorphic Memory Allocator Library For C++17 and above.        ###
################################################################################

cmake_minimum_required(VERSION 3.12)

project(LibFoundation_Memory VERSION 1.0.0 DESCRIPTION "Custom allocator interface for various allocation schemes.")

add_library(
  LibFoundation_Memory 
    STATIC
      # Headers
      "include/bf/memory/basic_allocators.hpp"
      "include/bf/memory/memory_api.hpp"
      "include/bf/memory/scoped_buffer.hpp"
      "include/bf/memory/stl_allocator.hpp"

      "include/memory/alignment.hpp"
      "include/memory/assertion.hpp"
      "include/memory/basic_types.hpp"
      "include/memory/default_heap.hpp"
      "include/memory/fixed_st_allocators.hpp"
      "include/memory/fixed_mt_allocators.hpp"
      "include/memory/memory_manager.hpp"

      # Sources
      "src/assertion.cpp"
      "src/alignment.cpp"
      "src/default_heap.cpp"
      "src/fixed_st_allocators.cpp"
      "src/fixed_mt_allocators.cpp"
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

set_property(TARGET LibFoundation_Memory PROPERTY FOLDER "BluFedora/Foundation")
