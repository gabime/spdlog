cmake_minimum_required(VERSION 3.28)
project(spdlog_module_consumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(SPDLOG_TEST_INSTALLED)
    # Consume the installed package. This is what compiles the installed
    # spdlog.cppm into a BMI inside the consumer's own build.
    find_package(spdlog CONFIG REQUIRED COMPONENTS module)

    add_executable(package-module main.cpp)
    target_link_libraries(package-module PRIVATE spdlog::spdlog_module)
    set_target_properties(package-module PROPERTIES CXX_SCAN_FOR_MODULES ON)

    # the classic targets must keep working from the very same install
    add_executable(package-compiled legacy/main.cpp)
    target_link_libraries(package-compiled PRIVATE spdlog::spdlog)
else()
    set(SPDLOG_BUILD_MODULE ON CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SPDLOG_INSTALL ON CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_SHARED ${SPDLOG_TEST_SHARED} CACHE BOOL "" FORCE)
    add_subdirectory("${SPDLOG_TEST_SOURCE_DIR}" spdlog)

    add_executable(source-module main.cpp)
    target_link_libraries(source-module PRIVATE spdlog::spdlog_module)
    set_target_properties(source-module PROPERTIES CXX_SCAN_FOR_MODULES ON)
endif()
