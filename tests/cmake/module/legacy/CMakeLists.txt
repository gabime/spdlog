cmake_minimum_required(VERSION 3.10)
project(spdlog_module_legacy_consumer LANGUAGES CXX)

# The point of this project: an old CMake and C++11 consumer must be able to use an
# install that was built with SPDLOG_BUILD_MODULE=ON. If the module target ever leaks
# into the main export set, the generated targets file gains a hard
# "CMake >= 3.28 required" error and configuring this project fails.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(spdlog CONFIG REQUIRED)

# spdlog::spdlog_module is defined here only when the consuming CMake is >= 3.28.
# Either way this project must configure and build; the assertion that the classic
# targets file carries no version requirement lives in module_test.cmake.
if(TARGET spdlog::spdlog_module)
    message(STATUS "spdlog::spdlog_module is available (CMake ${CMAKE_VERSION}), but unused here")
endif()

add_executable(legacy-compiled main.cpp)
target_link_libraries(legacy-compiled PRIVATE spdlog::spdlog)

add_executable(legacy-header-only main.cpp)
target_link_libraries(legacy-header-only PRIVATE spdlog::spdlog_header_only)
