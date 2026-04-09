add_library(performance_test_utils STATIC
    performance_utils.h
    performance_utils.cpp
)

target_link_libraries(performance_test_utils PUBLIC spdlog::spdlog)

target_include_directories(performance_test_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

spdlog_enable_warnings(performance_test_utils)

add_executable(spdlog-performance-tests
    main.cpp
    test_sync_performance.cpp
    test_async_performance.cpp
    test_multithread_performance.cpp
    test_sink_comparison.cpp
)

spdlog_enable_warnings(spdlog-performance-tests)

target_link_libraries(spdlog-performance-tests PRIVATE
    spdlog::spdlog
    performance_test_utils
    Catch2::Catch2WithMain
)

target_include_directories(spdlog-performance-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_test(NAME spdlog-performance-tests COMMAND spdlog-performance-tests --order decl)
set_tests_properties(spdlog-performance-tests PROPERTIES RUN_SERIAL ON)
