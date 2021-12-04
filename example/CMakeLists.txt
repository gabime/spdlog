# Copyright(c) 2019 spdlog authors Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.10)
project(spdlog_examples CXX)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

# ---------------------------------------------------------------------------------------
# Example of using shared libraries
# ---------------------------------------------------------------------------------------
if(SPDLOG_BUILD_SHARED OR BUILD_SHARED_LIBS)
    add_library(mylibrary SHARED mylibrary.cpp mylibrary.h)
    target_link_libraries(mylibrary PRIVATE spdlog::spdlog)
    target_compile_definitions(mylibrary PUBLIC SPDLOG_SHARED_LIB)
    target_compile_definitions(mylibrary PUBLIC SPDLOG_COMPILED_LIB)
    
    add_executable(library_example library_example.cpp)
    target_link_libraries(library_example PRIVATE spdlog::spdlog mylibrary)
    target_compile_definitions(library_example PUBLIC SPDLOG_SHARED_LIB)
    target_compile_definitions(library_example PUBLIC SPDLOG_COMPILED_LIB)
else()    
    add_library(mylibrary SHARED mylibrary.cpp mylibrary.h)
    target_link_libraries(mylibrary PRIVATE spdlog::spdlog_header_only)

    add_executable(library_example library_example.cpp)
    target_link_libraries(library_example PRIVATE spdlog::spdlog_header_only mylibrary)
endif()

# ---------------------------------------------------------------------------------------
# Example of using pre-compiled library
# ---------------------------------------------------------------------------------------
add_executable(example example.cpp)
target_link_libraries(example PRIVATE spdlog::spdlog)

# ---------------------------------------------------------------------------------------
# Example of using header-only library
# ---------------------------------------------------------------------------------------
if(SPDLOG_BUILD_EXAMPLE_HO)
    add_executable(example_header_only example.cpp)
    target_link_libraries(example_header_only PRIVATE spdlog::spdlog_header_only)
endif()
