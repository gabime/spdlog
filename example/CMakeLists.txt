# Copyright(c) 2019 spdlog authors
# Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.2)

project(spdlog_examples CXX)

if(NOT TARGET spdlog)
    # Stand-alone build
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    include(../cmake/utils.cmake)
    #TODO spdlog_extract_version()

    find_package(spdlog ${SPDLOG_VERSION} REQUIRED)
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

