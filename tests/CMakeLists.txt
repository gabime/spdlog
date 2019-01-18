project(spdlog-utests CXX)

find_package(Threads REQUIRED)

set(SPDLOG_UTESTS_SOURCES
    test_errors.cpp
    test_file_helper.cpp
    test_file_logging.cpp
    test_misc.cpp
    test_pattern_formatter.cpp
    test_async.cpp
    includes.h
    test_registry.cpp
    test_macros.cpp
    utils.cpp
    utils.h
    main.cpp
    test_mpmc_q.cpp
    test_sink.h
    test_fmt_helper.cpp)

add_executable(${PROJECT_NAME} ${SPDLOG_UTESTS_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/logs")

enable_testing()
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
