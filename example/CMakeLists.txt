# Copyright(c) 2019 spdlog authors
# Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.1)
project(SpdlogExamples CXX)

if(TARGET spdlog)
    # If we're running this example as part of the primary spdlog applciation
    # then add an alias. This allows us to use the same "spdlog::spdlog"
    # below that a user would use (with the namespace)
    add_library(spdlog::spdlog ALIAS spdlog)
    add_library(spdlog::spdlog_header_only ALIAS spdlog_header_only)
else()
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

#---------------------------------------------------------------------------------------
# Example of using pre-compiled library
#---------------------------------------------------------------------------------------
add_executable(example example.cpp)
target_link_libraries(example spdlog::spdlog)

#---------------------------------------------------------------------------------------
# Example of using header-only library
#---------------------------------------------------------------------------------------
add_executable(example_header_only example.cpp)
target_link_libraries(example_header_only spdlog::spdlog_header_only)

# Create logs directory
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/logs")
