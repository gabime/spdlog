cmake_minimum_required(VERSION 3.0)
set(TESTS_SRCS
    file_log.cpp
    format.cpp
    main.cpp
    registry.cpp
)

add_executable(spdlog-test ${TESTS_SRCS})
target_link_libraries(spdlog-test spdlog)
target_include_directories(spdlog-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET spdlog-test PROPERTY CXX_STANDARD 11)

