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

option(BUILD_SHARED_LIBS "Global flag to cause add_library to create shared libraries if on." ON)
option(SPDLOG_BUILD_EXAMPLES "Build examples" ${SPDLOG_MASTER_PROJECT})
option(SPDLOG_BUILD_BENCH "Build benchmarks (Requires https://github.com/google/benchmark.git to be installed)" OFF)
option(SPDLOG_BUILD_TESTS "Build tests" ON)
option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of bundled" OFF)
option(SPDLOG_INSTALL "Generate the install target." ${SPDLOG_MASTER_PROJECT})

set(HEADER_BASE "${CMAKE_CURRENT_SOURCE_DIR}/include/spdlog")

message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

# Build library
add_library(spdlog src/spdlog.cpp)
add_library(spdlog::spdlog ALIAS spdlog)
target_compile_definitions(spdlog PUBLIC SPDLOG_COMPILED_LIB )
target_include_directories(spdlog PUBLIC  "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>")
set_target_properties(spdlog PROPERTIES OUTPUT_NAME "spdlog")
set_target_properties(spdlog PROPERTIES DEBUG_POSTFIX "-debug")

# Headr only
add_library(spdlog_header_only INTERFACE)
target_include_directories(spdlog_header_only INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>")
add_library(spdlog::header_only ALIAS spdlog_header_only)

if(SPDLOG_FMT_EXTERNAL AND NOT TARGET fmt::fmt)
    find_package(fmt REQUIRED CONFIG)
endif()

if(SPDLOG_FMT_EXTERNAL)
    target_compile_definitions(spdlog INTERFACE SPDLOG_FMT_EXTERNAL)
    target_link_libraries(spdlog INTERFACE fmt::fmt)
    target_compile_definitions(spdlog_header_only INTERFACE SPDLOG_FMT_EXTERNAL)
    target_link_libraries(spdlog_header_only INTERFACE fmt::fmt)
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
install(TARGETS spdlog DESTINATION lib)

#---------------------------------------------------------------------------------------
# register project in CMake user registry
#---------------------------------------------------------------------------------------
export(PACKAGE ${PROJECT_NAME})
