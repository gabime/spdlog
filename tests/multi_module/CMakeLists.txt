cmake_minimum_required(VERSION 3.11)
project(spdlog_multi_module_tests CXX)

if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()

include(../../cmake/utils.cmake)

find_package(Catch2 3 QUIET)
if(Catch2_FOUND)
    message(STATUS "Packaged version of Catch will be used.")
else()
    message(STATUS "Bundled version of Catch will be downloaded and used.")
    include(FetchContent)
    FetchContent_Declare(
        Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG 53d0d913a422d356b23dd927547febdf69ee908
    )
    FetchContent_MakeAvailable(Catch2)
endif()

enable_testing()

add_subdirectory(module_core)
add_subdirectory(module_network)
add_subdirectory(module_data)

add_executable(spdlog-multi-module-tests
    main.cpp
    test_utils.cpp
    test_isolation.cpp
    test_integration.cpp
)

spdlog_enable_warnings(spdlog-multi-module-tests)

target_link_libraries(spdlog-multi-module-tests PRIVATE
    spdlog::spdlog
    module_core
    module_network
    module_data
    Catch2::Catch2WithMain
)

target_include_directories(spdlog-multi-module-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_test(NAME spdlog-multi-module-tests COMMAND spdlog-multi-module-tests --order decl)
set_tests_properties(spdlog-multi-module-tests PROPERTIES RUN_SERIAL ON)

add_subdirectory(performance)
