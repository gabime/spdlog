cmake_minimum_required(VERSION 3.10)
project(spdlog_tweakme_consumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(SPDLOG_TEST_INSTALLED)
    find_package(spdlog CONFIG REQUIRED)

    add_executable(package-compiled main.cpp)
    target_link_libraries(package-compiled PRIVATE spdlog::spdlog)

    add_executable(package-header-only main.cpp)
    target_link_libraries(package-header-only PRIVATE spdlog::spdlog_header_only)

    find_package(Threads REQUIRED)
    add_executable(raw-header-only main.cpp)
    target_include_directories(raw-header-only PRIVATE "${SPDLOG_TEST_PREFIX}/include")
    target_link_libraries(raw-header-only PRIVATE Threads::Threads)
else()
    set(SPDLOG_GENERATE_TWEAKME_H ON CACHE BOOL "" FORCE)
    set(SPDLOG_NO_THREAD_ID ON CACHE BOOL "" FORCE)
    set(SPDLOG_NO_TLS ON CACHE BOOL "" FORCE)
    set(SPDLOG_DISABLE_DEFAULT_LOGGER ON CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_SHARED ${SPDLOG_TEST_SHARED} CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SPDLOG_INSTALL ON CACHE BOOL "" FORCE)
    add_subdirectory("${SPDLOG_TEST_SOURCE_DIR}" spdlog)

    add_executable(source-compiled main.cpp)
    target_link_libraries(source-compiled PRIVATE spdlog::spdlog)

    add_executable(source-header-only main.cpp)
    target_link_libraries(source-header-only PRIVATE spdlog::spdlog_header_only)
endif()
