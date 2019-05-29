# Copyright(c) 2019 spdlog authors
# Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.1)


#---------------------------------------------------------------------------------------
# Start spdlog project
#---------------------------------------------------------------------------------------
include(cmake/version.cmake)
project(spdlog VERSION ${SPDLOG_VERSION} LANGUAGES CXX)
message(STATUS "Build spdlog: ${SPDLOG_VERSION}")

include(GNUInstallDirs)
include(cmake/ide.cmake)

#---------------------------------------------------------------------------------------
# Set default build to release
#---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

#---------------------------------------------------------------------------------------
# Compiler config
#---------------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#---------------------------------------------------------------------------------------
# Set SPDLOG_MASTER_PROJECT to ON if we are building spdlog
#---------------------------------------------------------------------------------------
# Check if spdlog is being used directly or via add_subdirectory, but allow overriding
if (NOT DEFINED SPDLOG_MASTER_PROJECT)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(SPDLOG_MASTER_PROJECT ON)
    else()
        set(SPDLOG_MASTER_PROJECT OFF)
    endif()
endif ()

option(SPDLOG_BUILD_EXAMPLES "Build examples" ON)
option(SPDLOG_BUILD_BENCH "Build benchmarks (Requires https://github.com/google/benchmark.git to be installed)" OFF)
option(SPDLOG_BUILD_TESTS "Build tests" OFF)
option(SPDLOG_BUILD_HO_TESTS "Build tests using the header only version" OFF)
option(SPDLOG_INSTALL "Generate the install target." ${SPDLOG_MASTER_PROJECT})
option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of bundled" OFF)

message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})


find_package(Threads REQUIRED)

#---------------------------------------------------------------------------------------
# Static library version
#---------------------------------------------------------------------------------------
add_library(spdlog STATIC src/spdlog.cpp ${SPDLOG_ALL_HEADERS})
add_library(spdlog::spdlog ALIAS spdlog)
    
target_compile_definitions(spdlog PUBLIC SPDLOG_COMPILED_LIB)
target_include_directories(spdlog PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(spdlog PUBLIC Threads::Threads)

#---------------------------------------------------------------------------------------
# Header only version
#---------------------------------------------------------------------------------------
add_library(spdlog_header_only INTERFACE)
add_library(spdlog::spdlog_header_only ALIAS spdlog_header_only)

target_include_directories(spdlog_header_only INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(spdlog_header_only INTERFACE Threads::Threads)

#---------------------------------------------------------------------------------------
# Turn on compiler warnings and sanitizers if we build our own project
#---------------------------------------------------------------------------------------
if(SPDLOG_MASTER_PROJECT)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options( spdlog PUBLIC -Wall -Wextra -Wconversion -pedantic -Wfatal-errors)
        target_compile_options( spdlog_header_only INTERFACE -Wall -Wextra -Wconversion -pedantic -Wfatal-errors)
    endif()
    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_compile_options( spdlog PUBLIC /W3 /WX )
        target_compile_options( spdlog_header_only INTERFACE /W3 /WX)
    endif()

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
    message(STATUS "Generating examples")
    add_subdirectory(example)
endif()

if(SPDLOG_BUILD_TESTS)
    message(STATUS "Generating tests")
    include(CTest)
    add_subdirectory(tests)
endif()

if(SPDLOG_BUILD_BENCH)
    message(STATUS "Generating benchmarks")
    add_subdirectory(bench)
endif()

#---------------------------------------------------------------------------------------
# install
#---------------------------------------------------------------------------------------
if (SPDLOG_INSTALL)
    message(STATUS "Generating install")
    set(project_config_in "${CMAKE_CURRENT_LIST_DIR}/cmake/spdlogConfig.cmake.in")
    set(project_config_out "${CMAKE_CURRENT_BINARY_DIR}/spdlogConfig.cmake")
    set(config_targets_file "spdlogConfigTargets.cmake")
    set(version_config_file "${CMAKE_CURRENT_BINARY_DIR}/spdlogConfigVersion.cmake")
    set(export_dest_dir "${CMAKE_INSTALL_LIBDIR}/spdlog/cmake")

    #---------------------------------------------------------------------------------------
    # include files
    #---------------------------------------------------------------------------------------
    install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
    install(TARGETS spdlog spdlog_header_only EXPORT spdlog DESTINATION "${CMAKE_INSTALL_LIBDIR}/spdlog")

    #---------------------------------------------------------------------------------------
    # package and version files
    #---------------------------------------------------------------------------------------
    install(EXPORT spdlog
            DESTINATION ${export_dest_dir}
            NAMESPACE spdlog::
            FILE ${config_targets_file})

    include(CMakePackageConfigHelpers)
    configure_file("${project_config_in}" "${project_config_out}" @ONLY)
    write_basic_package_version_file("${version_config_file}" COMPATIBILITY SameMajorVersion)
    install(FILES
            "${project_config_out}"
            "${version_config_file}" DESTINATION "${export_dest_dir}")

    #---------------------------------------------------------------------------------------
    # Support creation of installable packages
    #---------------------------------------------------------------------------------------
    include(cmake/spdlogCPack.cmake)

endif ()