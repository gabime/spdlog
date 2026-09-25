# Get spdlog version from include/spdlog/version.h and put it in SPDLOG_VERSION
function(spdlog_extract_version)
    file(READ "${CMAKE_CURRENT_LIST_DIR}/include/spdlog/version.h" file_contents)
    string(REGEX MATCH "SPDLOG_VER_MAJOR ([0-9]+)" _ "${file_contents}")
    if(NOT CMAKE_MATCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Could not extract major version number from spdlog/version.h")
    endif()
    set(ver_major ${CMAKE_MATCH_1})

    string(REGEX MATCH "SPDLOG_VER_MINOR ([0-9]+)" _ "${file_contents}")
    if(NOT CMAKE_MATCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Could not extract minor version number from spdlog/version.h")
    endif()

    set(ver_minor ${CMAKE_MATCH_1})
    string(REGEX MATCH "SPDLOG_VER_PATCH ([0-9]+)" _ "${file_contents}")
    if(NOT CMAKE_MATCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Could not extract patch version number from spdlog/version.h")
    endif()
    set(ver_patch ${CMAKE_MATCH_1})

    set(SPDLOG_VERSION_MAJOR ${ver_major} PARENT_SCOPE)
    set(SPDLOG_VERSION_MINOR ${ver_minor} PARENT_SCOPE)
    set(SPDLOG_VERSION_PATCH ${ver_patch} PARENT_SCOPE)
    set(SPDLOG_VERSION "${ver_major}.${ver_minor}.${ver_patch}" PARENT_SCOPE)
endfunction()

# Turn on warnings on the given target
function(spdlog_enable_warnings target_name)
    if(SPDLOG_BUILD_WARNINGS)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            list(APPEND MSVC_OPTIONS "/W3")
            if(MSVC_VERSION GREATER 1900) # Allow non fatal security warnings for msvc 2015
                list(APPEND MSVC_OPTIONS "/WX")
            endif()
        endif()

        target_compile_options(
            ${target_name}
            PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                    -Wall
                    -Wextra
                    -Wconversion
                    -pedantic
                    -Werror
                    -Wfatal-errors>
                    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_OPTIONS}>)
    endif()
endfunction()

# Enable address sanitizer (gcc/clang only)
function(spdlog_enable_addr_sanitizer target_name)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "Sanitizer supported only for gcc/clang")
    endif()
    message(STATUS "Address sanitizer enabled")
    target_compile_options(${target_name} PRIVATE -fsanitize=address,undefined)
    target_compile_options(${target_name} PRIVATE -fno-sanitize=signed-integer-overflow)
    target_compile_options(${target_name} PRIVATE -fno-sanitize-recover=all)
    target_compile_options(${target_name} PRIVATE -fno-omit-frame-pointer)
    target_link_libraries(${target_name} PRIVATE -fsanitize=address,undefined)
endfunction()

# Enable thread sanitizer (gcc/clang only)
function(spdlog_enable_thread_sanitizer target_name)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "Sanitizer supported only for gcc/clang")
    endif()
    message(STATUS "Thread sanitizer enabled")
    target_compile_options(${target_name} PRIVATE -fsanitize=thread)
    target_compile_options(${target_name} PRIVATE -fno-omit-frame-pointer)
    target_link_libraries(${target_name} PRIVATE -fsanitize=thread)
endfunction()

# Show a status message during the first CMake run only, as repeating it during
# the automatic reconfigurations of a parent project including us is just noise.
# The key identifies the message and has to be unique, the text is shown again
# if it changes, e.g. when the build type is different from the previous run.
function(spdlog_message_once key text)
    if(NOT "${text}" STREQUAL "$CACHE{SPDLOG_MESSAGE_${key}}")
        set(SPDLOG_MESSAGE_${key} "${text}" CACHE INTERNAL "")
        message(STATUS "${text}")
    endif()
endfunction()

# Determine whether this CMake / generator / compiler combination can build a
# C++20 module target. Sets <out_ok> to ON/OFF, and <out_reason> to an
# explanation of why not when it is OFF.
function(spdlog_check_module_support out_ok out_reason)
    set(${out_ok} OFF PARENT_SCOPE)

    if(CMAKE_VERSION VERSION_LESS "3.28")
        set(${out_reason} "CMake >= 3.28 is required (found ${CMAKE_VERSION})" PARENT_SCOPE)
        return()
    endif()

    if(CMAKE_GENERATOR MATCHES "^Ninja")
        execute_process(
            COMMAND "${CMAKE_MAKE_PROGRAM}" --version
            OUTPUT_VARIABLE ninja_version
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE ninja_result)
        if(ninja_result EQUAL 0 AND ninja_version VERSION_LESS "1.11")
            set(${out_reason} "the Ninja generator needs ninja >= 1.11 for C++ modules (found ${ninja_version})"
                PARENT_SCOPE)
            return()
        endif()
    elseif(NOT CMAKE_GENERATOR MATCHES "^Visual Studio (1[7-9]|[2-9][0-9]) ")
        set(${out_reason}
            "generator \"${CMAKE_GENERATOR}\" does not support C++ modules; use Ninja, Ninja Multi-Config or Visual Studio 17 2022 and newer"
            PARENT_SCOPE)
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # MSVC 19.44 (Visual Studio 2022 17.14) still miscompiles the module: formatter
        # specializations are not found from importers, and object files exceed 4 GB (C1605)
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.50")
            set(${out_reason} "MSVC >= 19.50 (Visual Studio 2026) is required (found ${CMAKE_CXX_COMPILER_VERSION})"
                PARENT_SCOPE)
            return()
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC 14 hits an internal compiler error on translation units importing the module
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15")
            set(${out_reason} "GCC >= 15 is required (found ${CMAKE_CXX_COMPILER_VERSION})" PARENT_SCOPE)
            return()
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "17")
            set(${out_reason} "Clang >= 17 is required (found ${CMAKE_CXX_COMPILER_VERSION})" PARENT_SCOPE)
            return()
        endif()
    else()
        set(${out_reason} "compiler \"${CMAKE_CXX_COMPILER_ID}\" is not known to support C++ modules" PARENT_SCOPE)
        return()
    endif()

    set(${out_ok} ON PARENT_SCOPE)
    set(${out_reason} "" PARENT_SCOPE)
endfunction()
