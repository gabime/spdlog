
#
# Copyright(c) 2015 Ruslan Baratov.
# Distributed under the MIT License (http://opensource.org/licenses/MIT)
#

cmake_minimum_required(VERSION 3.1)
project(spdlog VERSION 1.3.1 LANGUAGES CXX)
include(CMakeDependentOption)

#---------------------------------------------------------------------------------------
# set default build to release
#---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

#---------------------------------------------------------------------------------------
# compiler config
#---------------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    add_compile_options("-Wall")
    add_compile_options("-Wextra")
    add_compile_options("-Wconversion")
    add_compile_options("-pedantic")
    add_compile_options("-Wfatal-errors")
endif()

#---------------------------------------------------------------------------------------
# address sanitizers check
#---------------------------------------------------------------------------------------
include(cmake/sanitizers.cmake)

#---------------------------------------------------------------------------------------
# spdlog target
#---------------------------------------------------------------------------------------

# Check if spdlog is being used directly or via add_subdirectory
set(SPDLOG_MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(SPDLOG_MASTER_PROJECT ON)
endif()

option(SPDLOG_HEADER_ONLY "Header only version. Turn OFF to build as static lib" OFF)
option(SPDLOG_BUILD_EXAMPLES "Build examples" ${SPDLOG_MASTER_PROJECT})
option(SPDLOG_BUILD_BENCH "Build benchmarks (Requires https://github.com/google/benchmark.git to be installed)" ON)
option(SPDLOG_BUILD_TESTS "Build tests" ${SPDLOG_MASTER_PROJECT})
option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of bundled" OFF)
option(SPDLOG_INSTALL "Generate the install target." ${SPDLOG_MASTER_PROJECT})

set(HEADER_BASE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(SPDLOG_HEADER_ONLY)
	add_definitions(-DSPDLOG_HEADER_ONLY)
    add_library(spdlog INTERFACE)
    add_library(spdlog::spdlog ALIAS spdlog)
    target_include_directories(
            spdlog
            INTERFACE
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
    )

else()
	remove_definitions(-DSPDLOG_HEADER_ONLY)
    file(GLOB SRC_FILES ${HEADER_BASE}/spdlog/impl/*.cpp)
    add_library(spdlog STATIC ${SRC_FILES})
    add_library(spdlog::spdlog ALIAS spdlog)
    target_include_directories(
            spdlog
            PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
    )
endif()

if(SPDLOG_FMT_EXTERNAL AND NOT TARGET fmt::fmt)
    find_package(fmt REQUIRED CONFIG)
endif()


if(SPDLOG_FMT_EXTERNAL)
    target_compile_definitions(spdlog INTERFACE SPDLOG_FMT_EXTERNAL)
    target_link_libraries(spdlog INTERFACE fmt::fmt)
endif()


if(SPDLOG_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(SPDLOG_BUILD_TESTS)
    include(CTest)
    add_subdirectory(tests)
endif()

if(SPDLOG_BUILD_BENCH)
    add_subdirectory(bench)
endif()

#---------------------------------------------------------------------------------------
# install
#---------------------------------------------------------------------------------------
install(DIRECTORY ${HEADER_BASE}/spdlog DESTINATION include)

if(NOT SPDLOG_HEADER_ONLY)
    install(TARGETS spdlog ARCHIVE DESTINATION lib)
endif()

#---------------------------------------------------------------------------------------
# register project in CMake user registry
#---------------------------------------------------------------------------------------
export(PACKAGE ${PROJECT_NAME})

