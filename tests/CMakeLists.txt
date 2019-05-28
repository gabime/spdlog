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

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/logs")
enable_testing()

# The compiled library tests
if(SPDLOG_BUILD_TESTS)
    add_executable(spdlog-utests ${SPDLOG_UTESTS_SOURCES})
    target_link_libraries(spdlog-utests spdlog)
    add_test(NAME spdlog-utests COMMAND spdlog-utests)
endif()

# The header-only library version tests
if(SPDLOG_BUILD_HO_TESTS)
    add_executable(spdlog-utests-ho ${SPDLOG_UTESTS_SOURCES})
    target_link_libraries(spdlog-utests-ho spdlog::spdlog_header_only)
    add_test(NAME spdlog-utests-ho COMMAND spdlog-utests-ho)
endif()