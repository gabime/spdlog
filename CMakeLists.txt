# Copyright(c) 2019 spdlog authors
# Distributed under the MIT License (http://opensource.org/licenses/MIT)

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
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(SPDLOG_MASTER_PROJECT ON)
else()
    set(SPDLOG_MASTER_PROJECT OFF)
endif()

option(SPDLOG_STATIC_LIB "Build as static lib" ON)
option(SPDLOG_BUILD_EXAMPLES "Build examples" ${SPDLOG_MASTER_PROJECT})
option(SPDLOG_BUILD_BENCH "Build benchmarks (Requires https://github.com/google/benchmark.git to be installed)" OFF)
option(SPDLOG_BUILD_TESTS "Build tests" ON)
option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of bundled" OFF)
option(SPDLOG_INSTALL "Generate the install target." ${SPDLOG_MASTER_PROJECT})

set(HEADER_BASE "${CMAKE_CURRENT_SOURCE_DIR}/include/spdlog")

if(SPDLOG_STATIC_LIB)
    add_definitions(-DSPDLOG_STATIC_LIB)
    file(GLOB SRC_FILES ${HEADER_BASE}/impl/*.cpp)
    add_library(spdlog STATIC ${SRC_FILES})
    target_include_directories(spdlog PUBLIC  "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>")
    target_link_libraries(spdlog -Wl,--as-needed)
else()
    add_library(spdlog INTERFACE)
    target_include_directories(spdlog INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>")
endif()

add_library(spdlog::spdlog ALIAS spdlog)

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
install(DIRECTORY ${HEADER_BASE} DESTINATION include)

if(SPDLOG_STATIC_LIB)
    install(TARGETS spdlog ARCHIVE DESTINATION lib)
endif()

#---------------------------------------------------------------------------------------
# register project in CMake user registry
#---------------------------------------------------------------------------------------
export(PACKAGE ${PROJECT_NAME})
