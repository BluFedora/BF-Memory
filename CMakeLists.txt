################################################################################
#                                                                              #
#                              BF MEMORY PROJECT                               #
#                                                                              #
################################################################################
###         Polymorphic Memory Allocator Library For C++17 and above.        ###
################################################################################

cmake_minimum_required(VERSION 3.12)

project(BF_Memory VERSION 1.0.0 DESCRIPTION "Custom allocator interface for various allocation schemes.")

add_library(BF_Memory STATIC

    # Headers
    "include/bf/memory/basic_allocators.hpp"
    "include/bf/memory/crt_allocator.hpp"
    "include/bf/memory/memory_api.hpp"
    "include/bf/memory/scoped_buffer.hpp"
    "include/bf/memory/stl_allocator.hpp"

    # Sources
    "src/basic_allocators.cpp"
    "src/crt_allocator.cpp"
    "src/memory_api.cpp"
)

target_include_directories(
  BF_Memory
  PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)

target_compile_features(
  BF_Memory
  PUBLIC
    cxx_std_17
)

set_property(TARGET BF_Memory PROPERTY FOLDER "BluFedora/Foundation")
