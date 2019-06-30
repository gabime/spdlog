# Copyright(c) 2019 spdlog authors
# Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.1)
project(spdlog_examples CXX)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

#---------------------------------------------------------------------------------------
# Example of using pre-compiled library
#---------------------------------------------------------------------------------------
add_executable(example example.cpp)
spdlog_enable_warnings(example)
target_link_libraries(example PRIVATE spdlog::spdlog)

#---------------------------------------------------------------------------------------
# Example of using header-only library
#---------------------------------------------------------------------------------------
if(SPDLOG_BUILD_EXAMPLE_HO)
    add_executable(example_header_only example.cpp)
    spdlog_enable_warnings(example_header_only)
    target_link_libraries(example_header_only PRIVATE spdlog::spdlog_header_only)
endif()

# Create logs directory
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/logs")
