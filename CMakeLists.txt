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
# spdlog target
#---------------------------------------------------------------------------------------

# Check if spdlog is being used directly or via add_subdirectory, but allow overriding
if (NOT DEFINED SPDLOG_MASTER_PROJECT)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(SPDLOG_MASTER_PROJECT ON)
else()
    set(SPDLOG_MASTER_PROJECT OFF)
endif()
endif ()

option(SPDLOG_BUILD_EXAMPLES "Build examples" ${SPDLOG_MASTER_PROJECT})
option(SPDLOG_BUILD_BENCH "Build benchmarks (Requires https://github.com/google/benchmark.git to be installed)" OFF)
option(SPDLOG_BUILD_TESTS "Build tests" OFF)
option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of bundled" OFF)
option(SPDLOG_INSTALL "Generate the install target." ${SPDLOG_MASTER_PROJECT})

message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

find_package(Threads REQUIRED)

# Static library version
add_library(spdlog src/spdlog.cpp)
target_compile_definitions(spdlog PUBLIC SPDLOG_COMPILED_LIB)
target_include_directories(spdlog PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
)

target_link_libraries(spdlog PUBLIC Threads::Threads)

# Header only version
add_library(spdlog_header_only INTERFACE)
target_include_directories(spdlog_header_only INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(spdlog_header_only INTERFACE Threads::Threads)

#---------------------------------------------------------------------------------------
# address sanitizers check
#---------------------------------------------------------------------------------------
if(SPDLOG_MASTER_PROJECT)
    include(cmake/sanitizers.cmake)
endif()

#---------------------------------------------------------------------------------------
# use fmt package if using exertnal fmt
#---------------------------------------------------------------------------------------
if(SPDLOG_FMT_EXTERNAL)
    if (NOT TARGET fmt::fmt)
        find_package(fmt REQUIRED)
    endif ()

    target_compile_definitions(spdlog PUBLIC SPDLOG_FMT_EXTERNAL)
    target_link_libraries(spdlog PUBLIC fmt::fmt)

    target_compile_definitions(spdlog_header_only INTERFACE SPDLOG_FMT_EXTERNAL)
    target_link_libraries(spdlog_header_only INTERFACE fmt::fmt)
endif()

#---------------------------------------------------------------------------------------
# build binries
#---------------------------------------------------------------------------------------
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
if (SPDLOG_INSTALL)
    set(project_config_in "${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake.in")
    set(project_config_out "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake")
    set(config_targets_file "${PROJECT_NAME}ConfigTargets.cmake")
    set(version_config_file "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
    set(dest_dir lib/${PROJECT_NAME}/cmake)

    #---------------------------------------------------------------------------------------
    # lib in include files
    #---------------------------------------------------------------------------------------
    install(DIRECTORY include/ DESTINATION include)
    install(TARGETS spdlog EXPORT ${PROJECT_NAME} DESTINATION lib)

    #---------------------------------------------------------------------------------------
    # package and version files
    #---------------------------------------------------------------------------------------
    install(EXPORT ${PROJECT_NAME}
            DESTINATION ${dest_dir}
            NAMESPACE ${PROJECT_NAME}::
            FILE ${config_targets_file})

    include(CMakePackageConfigHelpers)
    configure_file("${project_config_in}" "${project_config_out}" @ONLY)
    write_basic_package_version_file("${version_config_file}" COMPATIBILITY SameMajorVersion)
    install(FILES
            "${project_config_out}"
            "${version_config_file}" DESTINATION "${dest_dir}")

    #---------------------------------------------------------------------------------------
    # Support creation of installable packages
    #---------------------------------------------------------------------------------------
    include(cmake/SpdlogCPack.cmake)

    #---------------------------------------------------------------------------------------
    # register project in CMake user registry - disabled by default since the
    # installed/packaged version of the project is preferred.
    #---------------------------------------------------------------------------------------
    option(CMAKE_EXPORT_NO_PACKAGE_REGISTRY "Disable registration of CMake's build directory." ON)
    export(PACKAGE ${PROJECT_NAME})
endif ()

