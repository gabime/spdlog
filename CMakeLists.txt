# Copyright(c) 2019 spdlog authors Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.11)

# ---------------------------------------------------------------------------------------
# Start spdlog project
# ---------------------------------------------------------------------------------------
include(cmake/utils.cmake)
include(cmake/ide.cmake)

spdlog_extract_version()

project(spdlog VERSION ${SPDLOG_VERSION} LANGUAGES CXX)
message(STATUS "Build spdlog: ${SPDLOG_VERSION}")

include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ---------------------------------------------------------------------------------------
# Set default build to release
# ---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

# ---------------------------------------------------------------------------------------
# Compiler config
# ---------------------------------------------------------------------------------------
# c++ standard >=17 is required
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
elseif(CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "Minimum supported CMAKE_CXX_STANDARD is 17, but it is set to ${CMAKE_CXX_STANDARD}")
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# make sure __cplusplus is defined when using msvc and enable parallel build
if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus /MP")
endif()

set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SYSTEM_NAME MATCHES "CYGWIN" OR CMAKE_SYSTEM_NAME MATCHES "MSYS")
    set(CMAKE_CXX_EXTENSIONS ON)
endif()

# ---------------------------------------------------------------------------------------
# Set SPDLOG_MASTER_PROJECT to ON if we are building spdlog
# ---------------------------------------------------------------------------------------
# Check if spdlog is being used directly or via add_subdirectory, but allow overriding
if(NOT DEFINED SPDLOG_MASTER_PROJECT)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(SPDLOG_MASTER_PROJECT ON)
    else()
        set(SPDLOG_MASTER_PROJECT OFF)
    endif()
endif()

option(SPDLOG_BUILD_ALL "Build all artifacts" OFF)

# build shared option
option(SPDLOG_BUILD_SHARED "Build shared library" OFF)

# build position independent code
option(SPDLOG_BUILD_PIC "Build position independent code (-fPIC)" OFF)

# example options
option(SPDLOG_BUILD_EXAMPLE "Build example" ${SPDLOG_MASTER_PROJECT})

# testing options
option(SPDLOG_BUILD_TESTS "Build tests" OFF)

# bench options
option(SPDLOG_BUILD_BENCH "Build benchmarks (Requires https://github.com/google/benchmark.git to be installed)" OFF)

# sanitizer options
option(SPDLOG_SANITIZE_ADDRESS "Enable address sanitizer in tests" OFF)

# warning options
option(SPDLOG_BUILD_WARNINGS "Enable compiler warnings" OFF)

# install options
option(SPDLOG_SYSTEM_INCLUDES "Include as system headers (skip for clang-tidy)." OFF)
option(SPDLOG_INSTALL "Generate the install target" ${SPDLOG_MASTER_PROJECT})
option(SPDLOG_USE_STD_FORMAT "Use std::format instead of fmt library." OFF)
option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of bundled" OFF)
option(SPDLOG_FMT_EXTERNAL_HO "Use external fmt header-only library instead of bundled" OFF)
option(SPDLOG_NO_EXCEPTIONS "Compile with -fno-exceptions. Call abort() on any spdlog exceptions" OFF)

if(SPDLOG_FMT_EXTERNAL AND SPDLOG_FMT_EXTERNAL_HO)
    message(FATAL_ERROR "SPDLOG_FMT_EXTERNAL and SPDLOG_FMT_EXTERNAL_HO are mutually exclusive")
endif()

if(SPDLOG_USE_STD_FORMAT AND SPDLOG_FMT_EXTERNAL_HO)
    message(FATAL_ERROR "SPDLOG_USE_STD_FORMAT and SPDLOG_FMT_EXTERNAL_HO are mutually exclusive")
endif()

if(SPDLOG_USE_STD_FORMAT AND SPDLOG_FMT_EXTERNAL)
    message(FATAL_ERROR "SPDLOG_USE_STD_FORMAT and SPDLOG_FMT_EXTERNAL are mutually exclusive")
endif()

if(SPDLOG_USE_STD_FORMAT AND CMAKE_CXX_STANDARD LESS 20)
    message(FATAL_ERROR "SPDLOG_USE_STD_FORMAT must be used with CMAKE_CXX_STANDARD >= 20")
endif()

# misc tweakme options
if(WIN32)
    option(SPDLOG_WCHAR_FILENAMES "Support wchar filenames" OFF)
else()
    set(SPDLOG_WCHAR_FILENAMES OFF CACHE BOOL "non supported option" FORCE)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    option(SPDLOG_CLOCK_COARSE "Use CLOCK_REALTIME_COARSE instead of the regular clock," OFF)
else()
    set(SPDLOG_CLOCK_COARSE OFF CACHE BOOL "non supported option" FORCE)
endif()

option(SPDLOG_PREVENT_CHILD_FD "Prevent from child processes to inherit log file descriptors" OFF)
option(SPDLOG_NO_THREAD_ID "prevent spdlog from querying the thread id on each log call if thread id is not needed" OFF)
option(
    SPDLOG_NO_ATOMIC_LEVELS
    "prevent spdlog from using of std::atomic log levels (use only if your code never modifies log levels concurrently"
    OFF)
option(SPDLOG_DISABLE_DEFAULT_LOGGER "Disable default logger creation" OFF)

# clang-tidy
option(SPDLOG_TIDY "run clang-tidy" OFF)

if(SPDLOG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    message(STATUS "Enabled clang-tidy")
endif()

if(SPDLOG_BUILD_PIC)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

find_package(Threads REQUIRED)
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
# ---------------------------------------------------------------------------------------
# Library sources
# ---------------------------------------------------------------------------------------
set(SPDLOG_HEADERS
    "include/spdlog/async.h"
    "include/spdlog/async_logger.h"
    "include/spdlog/common.h"
    "include/spdlog/formatter.h"
    "include/spdlog/fwd.h"
    "include/spdlog/logger.h"
    "include/spdlog/pattern_formatter.h"
    "include/spdlog/source_loc.h"
    "include/spdlog/spdlog.h"
    "include/spdlog/stopwatch.h"
    "include/spdlog/tweakme.h"
    "include/spdlog/version.h"
    "include/spdlog/cfg/argv.h"
    "include/spdlog/cfg/env.h"
    "include/spdlog/cfg/helpers.h"
    "include/spdlog/details/circular_q.h"
    "include/spdlog/details/console_globals.h"
    "include/spdlog/details/file_helper.h"
    "include/spdlog/details/fmt_helper.h"
    "include/spdlog/details/log_msg.h"
    "include/spdlog/details/log_msg_buffer.h"
    "include/spdlog/details/mpmc_blocking_q.h"
    "include/spdlog/details/null_mutex.h"
    "include/spdlog/details/os.h"
    "include/spdlog/details/periodic_worker.h"
    "include/spdlog/details/registry.h"
    "include/spdlog/details/synchronous_factory.h"
    "include/spdlog/details/tcp_client-windows.h"
    "include/spdlog/details/tcp_client.h"
    "include/spdlog/details/thread_pool.h"
    "include/spdlog/details/udp_client-windows.h"
    "include/spdlog/details/udp_client.h"
    "include/spdlog/details/windows_include.h"
    "include/spdlog/fmt/bin_to_hex.h"
    "include/spdlog/fmt/chrono.h"
    "include/spdlog/fmt/compile.h"
    "include/spdlog/fmt/fmt.h"
    "include/spdlog/fmt/ostr.h"
    "include/spdlog/fmt/ranges.h"
    "include/spdlog/fmt/std.h"
    "include/spdlog/fmt/xchar.h"
    "include/spdlog/sinks/android_sink.h"
    "include/spdlog/sinks/ansicolor_sink.h"
    "include/spdlog/sinks/base_sink.h"
    "include/spdlog/sinks/basic_file_sink.h"
    "include/spdlog/sinks/callback_sink.h"
    "include/spdlog/sinks/daily_file_sink.h"
    "include/spdlog/sinks/dist_sink.h"
    "include/spdlog/sinks/dup_filter_sink.h"
    "include/spdlog/sinks/hourly_file_sink.h"
    "include/spdlog/sinks/kafka_sink.h"
    "include/spdlog/sinks/mongo_sink.h"
    "include/spdlog/sinks/msvc_sink.h"
    "include/spdlog/sinks/null_sink.h"
    "include/spdlog/sinks/ostream_sink.h"
    "include/spdlog/sinks/qt_sinks.h"
    "include/spdlog/sinks/ringbuffer_sink.h"
    "include/spdlog/sinks/rotating_file_sink.h"
    "include/spdlog/sinks/sink.h"
    "include/spdlog/sinks/stdout_color_sinks.h"
    "include/spdlog/sinks/stdout_sinks.h"
    "include/spdlog/sinks/syslog_sink.h"
    "include/spdlog/sinks/systemd_sink.h"
    "include/spdlog/sinks/tcp_sink.h"
    "include/spdlog/sinks/udp_sink.h"
    "include/spdlog/sinks/win_eventlog_sink.h"
    "include/spdlog/sinks/wincolor_sink.h")

set(SPDLOG_SRCS 
    "src/async_logger.cpp"
    "src/common.cpp"
    "src/logger.cpp"
    "src/pattern_formatter.cpp"
    "src/spdlog.cpp"
    "src/cfg/helpers.cpp"
    "src/details/file_helper.cpp"
    "src/details/log_msg.cpp"
    "src/details/log_msg_buffer.cpp"
    "src/details/os.cpp"
    "src/details/periodic_worker.cpp"
    "src/details/registry.cpp"
    "src/details/thread_pool.cpp"    
    "src/sinks/ansicolor_sink.cpp"
    "src/sinks/base_sink.cpp"
    "src/sinks/basic_file_sink.cpp"
    "src/sinks/rotating_file_sink.cpp"
    "src/sinks/sink.cpp"
    "src/sinks/stdout_color_sinks.cpp"
    "src/sinks/stdout_sinks.cpp"
    "src/sinks/wincolor_sink.cpp")

# bundle {fmt} lib sources if required (true by default)
if(NOT SPDLOG_USE_STD_FORMAT AND NOT SPDLOG_FMT_EXTERNAL AND NOT SPDLOG_FMT_EXTERNAL_HO)    
    set(BUNDLED_FMT_HEADERS     
        "include/spdlog/fmt/bundled/args.h"
        "include/spdlog/fmt/bundled/chrono.h"
        "include/spdlog/fmt/bundled/color.h"
        "include/spdlog/fmt/bundled/compile.h"
        "include/spdlog/fmt/bundled/core.h"
        "include/spdlog/fmt/bundled/format-inl.h"
        "include/spdlog/fmt/bundled/format.h"
        "include/spdlog/fmt/bundled/locale.h"
        "include/spdlog/fmt/bundled/os.h"
        "include/spdlog/fmt/bundled/ostream.h"
        "include/spdlog/fmt/bundled/printf.h"
        "include/spdlog/fmt/bundled/ranges.h"
        "include/spdlog/fmt/bundled/std.h"
        "include/spdlog/fmt/bundled/xchar.h")
    list(APPEND SPDLOG_HEADERS ${BUNDLED_FMT_HEADERS})
    list(APPEND SPDLOG_SRCS src/fmt/bundled_fmtlib_format.cpp)
endif()

if(SPDLOG_BUILD_SHARED OR BUILD_SHARED_LIBS)
    if(WIN32)
        set(VERSION_RC ${CMAKE_CURRENT_BINARY_DIR}/version.rc )
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc @ONLY)        
    endif()
    add_library(spdlog SHARED ${SPDLOG_SRCS} ${SPDLOG_HEADERS} ${VERSION_RC})
    target_compile_definitions(spdlog PUBLIC SPDLOG_SHARED_LIB)
    if(MSVC)
        target_compile_options(spdlog PUBLIC $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<NOT:$<COMPILE_LANGUAGE:CUDA>>>:/wd4251
                                             /wd4275>)
    endif()
    if(NOT SPDLOG_USE_STD_FORMAT AND NOT SPDLOG_FMT_EXTERNAL AND NOT SPDLOG_FMT_EXTERNAL_HO)
        target_compile_definitions(spdlog PRIVATE FMT_LIB_EXPORT PUBLIC FMT_SHARED)
    endif()
else()
    add_library(spdlog STATIC ${SPDLOG_SRCS} ${SPDLOG_HEADERS})
endif()

add_library(spdlog::spdlog ALIAS spdlog)

set(SPDLOG_INCLUDES_LEVEL "")
if(SPDLOG_SYSTEM_INCLUDES)
    set(SPDLOG_INCLUDES_LEVEL "SYSTEM")
endif()

target_include_directories(spdlog ${SPDLOG_INCLUDES_LEVEL} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
                                                                  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(spdlog PUBLIC Threads::Threads)
spdlog_enable_warnings(spdlog)

set_target_properties(spdlog PROPERTIES VERSION ${SPDLOG_VERSION} SOVERSION
                                                                  ${SPDLOG_VERSION_MAJOR}.${SPDLOG_VERSION_MINOR})
set_target_properties(spdlog PROPERTIES DEBUG_POSTFIX d)

# set source groups for visual studio
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    source_group(TREE ${CMAKE_SOURCE_DIR}/include PREFIX include FILES ${SPDLOG_HEADERS})
    source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX sources FILES ${SPDLOG_SRCS})    
    source_group(sources FILES ${VERSION_RC})
endif()
# ---------------------------------------------------------------------------------------
# Use {fmt} lib package if using external fmt
# ---------------------------------------------------------------------------------------
if(SPDLOG_FMT_EXTERNAL OR SPDLOG_FMT_EXTERNAL_HO)
    if(NOT TARGET fmt::fmt)
        find_package(fmt CONFIG REQUIRED)
    endif()
    target_compile_definitions(spdlog PUBLIC SPDLOG_FMT_EXTERNAL)

    # use external fmt-header-nly
    if(SPDLOG_FMT_EXTERNAL_HO)
        target_link_libraries(spdlog PUBLIC fmt::fmt-header-only)
    else() # use external compile fmt
        target_link_libraries(spdlog PUBLIC fmt::fmt)
    endif()

    set(PKG_CONFIG_REQUIRES fmt) # add dependency to pkg-config
endif()

# ---------------------------------------------------------------------------------------
# Add required libraries for Android CMake build
# ---------------------------------------------------------------------------------------
if(ANDROID)
    target_link_libraries(spdlog PUBLIC log)
endif()

# ---------------------------------------------------------------------------------------
# Misc definitions according to tweak options
# ---------------------------------------------------------------------------------------
foreach(
    SPDLOG_OPTION
    SPDLOG_WCHAR_FILENAMES
    SPDLOG_NO_EXCEPTIONS
    SPDLOG_CLOCK_COARSE
    SPDLOG_PREVENT_CHILD_FD
    SPDLOG_NO_THREAD_ID
    SPDLOG_NO_ATOMIC_LEVELS
    SPDLOG_DISABLE_DEFAULT_LOGGER
    SPDLOG_USE_STD_FORMAT)
    if(${SPDLOG_OPTION})
        target_compile_definitions(spdlog PUBLIC ${SPDLOG_OPTION})
    endif()
endforeach()

# ---------------------------------------------------------------------------------------
# If exceptions are disabled, disable them in the bundled fmt as well
# ---------------------------------------------------------------------------------------
if(SPDLOG_NO_EXCEPTIONS)
    if(NOT SPDLOG_FMT_EXTERNAL AND NOT SPDLOG_FMT_EXTERNAL_HO)
        target_compile_definitions(spdlog PUBLIC FMT_EXCEPTIONS=0)
    endif()
    if(NOT MSVC)
        target_compile_options(spdlog PRIVATE -fno-exceptions)
    else()
        target_compile_options(spdlog PRIVATE /EHsc)
    endif()
endif()
# ---------------------------------------------------------------------------------------
# Build binaries
# ---------------------------------------------------------------------------------------
if(SPDLOG_BUILD_EXAMPLE OR SPDLOG_BUILD_ALL)
    message(STATUS "Generating example(s)")
    add_subdirectory(example)
    spdlog_enable_warnings(example)
endif()

if(SPDLOG_BUILD_TESTS OR SPDLOG_BUILD_ALL)
    message(STATUS "Generating tests")
    enable_testing()
    add_subdirectory(tests)
endif()

if(SPDLOG_BUILD_BENCH OR SPDLOG_BUILD_ALL)
    message(STATUS "Generating benchmarks")
    add_subdirectory(bench)
endif()

# ---------------------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------------------
if(SPDLOG_INSTALL)
    message(STATUS "Generating install")
    set(project_config_in "${CMAKE_CURRENT_LIST_DIR}/cmake/spdlogConfig.cmake.in")
    set(project_config_out "${CMAKE_CURRENT_BINARY_DIR}/spdlogConfig.cmake")
    set(config_targets_file "spdlogConfigTargets.cmake")
    set(version_config_file "${CMAKE_CURRENT_BINARY_DIR}/spdlogConfigVersion.cmake")
    set(export_dest_dir "${CMAKE_INSTALL_LIBDIR}/cmake/spdlog")
    set(pkgconfig_install_dir "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
    set(pkg_config "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc")

    # ---------------------------------------------------------------------------------------
    # Include files
    # ---------------------------------------------------------------------------------------
    install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" PATTERN "fmt/bundled" EXCLUDE)
    install(
        TARGETS spdlog
        EXPORT spdlog
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    if(NOT SPDLOG_USE_STD_FORMAT AND NOT SPDLOG_FMT_EXTERNAL AND NOT SPDLOG_FMT_EXTERNAL_HO)
        install(DIRECTORY include/${PROJECT_NAME}/fmt/bundled/
                DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/fmt/bundled/")
    endif()

    # ---------------------------------------------------------------------------------------
    # Install pkg-config file
    # ---------------------------------------------------------------------------------------
    if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
        set(PKG_CONFIG_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
    else()
        set(PKG_CONFIG_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
    endif()
    if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
        set(PKG_CONFIG_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
    else()
        set(PKG_CONFIG_LIBDIR "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
    endif()
    get_target_property(PKG_CONFIG_DEFINES spdlog INTERFACE_COMPILE_DEFINITIONS)
    string(REPLACE ";" " -D" PKG_CONFIG_DEFINES "${PKG_CONFIG_DEFINES}")
    string(CONCAT PKG_CONFIG_DEFINES "-D" "${PKG_CONFIG_DEFINES}")
    configure_file("cmake/${PROJECT_NAME}.pc.in" "${pkg_config}" @ONLY)
    install(FILES "${pkg_config}" DESTINATION "${pkgconfig_install_dir}")

    # ---------------------------------------------------------------------------------------
    # Install CMake config files
    # ---------------------------------------------------------------------------------------
    install(EXPORT spdlog DESTINATION ${export_dest_dir} NAMESPACE spdlog:: FILE ${config_targets_file})

    include(CMakePackageConfigHelpers)
    configure_package_config_file("${project_config_in}" "${project_config_out}" INSTALL_DESTINATION ${export_dest_dir})

    write_basic_package_version_file("${version_config_file}" COMPATIBILITY SameMajorVersion)
    install(FILES "${project_config_out}" "${version_config_file}" DESTINATION "${export_dest_dir}")

    # ---------------------------------------------------------------------------------------
    # Support creation of installable packages
    # ---------------------------------------------------------------------------------------
    include(cmake/spdlogCPack.cmake)
endif()
