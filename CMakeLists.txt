#
# Copyright(c) 2015 Ruslan Baratov.
# Distributed under the MIT License (http://opensource.org/licenses/MIT)
#

cmake_minimum_required(VERSION 3.1)
project(spdlog VERSION 1.0.0)
include(CTest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")
endif()

add_library(spdlog INTERFACE)

option(SPDLOG_BUILD_EXAMPLES "Build examples" OFF)
option(SPDLOG_BUILD_TESTING "Build spdlog tests" ON)

target_include_directories(
    spdlog
    INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
)

set(HEADER_BASE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(SPDLOG_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(SPDLOG_BUILD_TESTING)
    add_subdirectory(tests)
endif()

### Install ###
# * https://github.com/forexample/package-example
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(include_install_dir "include")
set(pkgconfig_install_dir "lib/pkgconfig")

set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(pkg_config "${generated_dir}/${PROJECT_NAME}.pc")
set(targets_export_name "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${version_config}" COMPATIBILITY SameMajorVersion
)

# Note: use 'targets_export_name'
configure_file("cmake/Config.cmake.in" "${project_config}" @ONLY)
configure_file("cmake/spdlog.pc.in" "${pkg_config}" @ONLY)

install(
    TARGETS spdlog
    EXPORT "${targets_export_name}"
    INCLUDES DESTINATION "${include_install_dir}"
)

install(DIRECTORY "include/spdlog" DESTINATION "${include_install_dir}")

install(
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
)

install(
    FILES "${pkg_config}"
    DESTINATION "${pkgconfig_install_dir}"
)

install(
    EXPORT "${targets_export_name}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
)

file(GLOB_RECURSE spdlog_include_SRCS "${HEADER_BASE}/*.h")
add_custom_target(spdlog_headers_for_ide SOURCES ${spdlog_include_SRCS})
