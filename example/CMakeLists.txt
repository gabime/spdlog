# Copyright(c) 2019 spdlog authors Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.11)
project(spdlog_examples CXX)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

# ---------------------------------------------------------------------------------------
# Example of using pre-compiled library
# ---------------------------------------------------------------------------------------
add_executable(example example.cpp)
target_link_libraries(example PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)

# ---------------------------------------------------------------------------------------
# Example of using header-only library
# ---------------------------------------------------------------------------------------
if(SPDLOG_BUILD_EXAMPLE_HO)
    add_executable(example_header_only example.cpp)
    target_link_libraries(example_header_only PRIVATE spdlog::spdlog_header_only)
endif()

# ---------------------------------------------------------------------------------------
# Example of using the C++20 named module
# ---------------------------------------------------------------------------------------
if(SPDLOG_BUILD_MODULE AND TARGET spdlog::spdlog_module)
    add_executable(example_module module_example.cpp)
    target_link_libraries(example_module PRIVATE spdlog::spdlog_module $<$<BOOL:${MINGW}>:ws2_32>)
    # This directory declares cmake_minimum_required(VERSION 3.11), so CMP0155 is OLD here and
    # sources that only `import` are not scanned unless we ask for it explicitly.
    set_target_properties(example_module PROPERTIES CXX_SCAN_FOR_MODULES ON)
endif()
