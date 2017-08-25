#
# Copyright(c) 2015 Ruslan Baratov.
# Distributed under the MIT License (http://opensource.org/licenses/MIT)
#

cmake_minimum_required(VERSION 3.1)
project(spdlog VERSION 0.14.0)
include(CTest)
include(CMakeDependentOption)
include(GNUInstallDirs)

#---------------------------------------------------------------------------------------
# compiler config
#---------------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")
endif()

#---------------------------------------------------------------------------------------
# spdlog target
#---------------------------------------------------------------------------------------
add_library(spdlog INTERFACE)

option(SPDLOG_BUILD_EXAMPLES "Build examples" OFF)
cmake_dependent_option(SPDLOG_BUILD_TESTING
    "Build spdlog tests" ON
    "BUILD_TESTING" OFF
)

target_include_directories(
    spdlog
    INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

set(HEADER_BASE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(SPDLOG_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(SPDLOG_BUILD_TESTING)
    add_subdirectory(tests)
endif()

#---------------------------------------------------------------------------------------
# Install/export targets and files
#---------------------------------------------------------------------------------------
# set files and directories
set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(include_install_dir "${CMAKE_INSTALL_INCLUDEDIR}")
set(pkgconfig_install_dir "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
set(version_config "${CMAKE_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${PROJECT_NAME}Config.cmake")
set(pkg_config "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc")
set(targets_export_name "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

# generate package version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${version_config}" COMPATIBILITY SameMajorVersion
)

# configure pkg config file
configure_file("cmake/spdlog.pc.in" "${pkg_config}" @ONLY)

# install targets
install(
    TARGETS spdlog
    EXPORT "${targets_export_name}"
    INCLUDES DESTINATION "${include_install_dir}"
)

# install headers
install(
    DIRECTORY "${HEADER_BASE}/${PROJECT_NAME}"
    DESTINATION "${include_install_dir}"
)

# install project version file
install(
    FILES "${version_config}"
    DESTINATION "${config_install_dir}"
)

# install pkg config file
install(
    FILES "${pkg_config}"
    DESTINATION "${pkgconfig_install_dir}"
)

# install project config file
install(
    EXPORT "${targets_export_name}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
    FILE ${project_config}
)

# export build directory config file
export(
    EXPORT ${targets_export_name}
    NAMESPACE "${namespace}"
    FILE ${project_config}
)

# register project in CMake user registry
export(PACKAGE ${PROJECT_NAME})

file(GLOB_RECURSE spdlog_include_SRCS "${HEADER_BASE}/*.h")
add_custom_target(spdlog_headers_for_ide SOURCES ${spdlog_include_SRCS})
