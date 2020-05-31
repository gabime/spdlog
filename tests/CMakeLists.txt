cmake_minimum_required(VERSION 3.2)

project(spdlog_utests CXX)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

include(../cmake/utils.cmake)

find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(systemd libsystemd)
endif()

set(SPDLOG_UTESTS_SOURCES
    test_file_helper.cpp
    test_file_logging.cpp
    test_daily_logger.cpp
    test_misc.cpp
    test_eventlog.cpp
    test_pattern_formatter.cpp
    test_async.cpp
    test_registry.cpp
    test_macros.cpp
    utils.cpp
    main.cpp
    test_mpmc_q.cpp
    test_dup_filter.cpp
    test_fmt_helper.cpp
    test_stdout_api.cpp
    test_backtrace.cpp
    test_create_dir.cpp
    test_cfg.cpp
    test_time_point.cpp)

if(NOT SPDLOG_NO_EXCEPTIONS)
    list(APPEND SPDLOG_UTESTS_SOURCES test_errors.cpp)
endif()

if(systemd_FOUND)
    list(APPEND SPDLOG_UTESTS_SOURCES test_systemd.cpp)
endif()

enable_testing()

function(spdlog_prepare_test test_target spdlog_lib)
    add_executable(${test_target} ${SPDLOG_UTESTS_SOURCES})
    spdlog_enable_warnings(${test_target})
    target_link_libraries(${test_target} PRIVATE ${spdlog_lib})
    if(systemd_FOUND)
        target_link_libraries(${test_target} PRIVATE ${systemd_LIBRARIES})
    endif()
    if(SPDLOG_SANITIZE_ADDRESS)
        spdlog_enable_sanitizer(${test_target})
    endif()
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES RUN_SERIAL ON)
endfunction()

# The compiled library tests
if(SPDLOG_BUILD_TESTS OR SPDLOG_BUILD_ALL)
    spdlog_prepare_test(spdlog-utests spdlog::spdlog)
endif()

# The header-only library version tests
if(SPDLOG_BUILD_TESTS_HO OR SPDLOG_BUILD_ALL)
    spdlog_prepare_test(spdlog-utests-ho spdlog::spdlog_header_only)
endif()

# Set up compilation failure test case (only available if compiler supports relaxed constexpr and c++ standard is > 11)
set(HAVE_CXX_RELAXED_CONSTEXPR)
if(CMAKE_CXX_STANDARD GREATER 11) # If we're in c++11 mode we don't have relaxed constexpr, even if our compiler is new enough.
    list(FIND CMAKE_CXX_COMPILE_FEATURES "cxx_relaxed_constexpr" HAVE_CXX_RELAXED_CONSTEXPR)
endif()
if(HAVE_CXX_RELAXED_CONSTEXPR AND (SPDLOG_BUILD_FAILING_TESTS OR SPDLOG_BUILD_ALL))
    message(STATUS "Enabling negative compilation unit test target")
    set(SPDLOG_FAIL_COMPILATION_TARGET spdlog_fail_compilation_utests)
    add_executable(${SPDLOG_FAIL_COMPILATION_TARGET}
        test_compilation_failures.cpp
        main.cpp)
    spdlog_enable_warnings(${SPDLOG_FAIL_COMPILATION_TARGET})
    target_link_libraries(${SPDLOG_FAIL_COMPILATION_TARGET} PRIVATE spdlog::spdlog)
    set_target_properties(${SPDLOG_FAIL_COMPILATION_TARGET} PROPERTIES
        EXCLUDE_FROM_ALL TRUE
        EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    add_test(NAME ${SPDLOG_FAIL_COMPILATION_TARGET}
        COMMAND ${CMAKE_COMMAND} --build . --target ${SPDLOG_FAIL_COMPILATION_TARGET} --config $<CONFIG>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    set_tests_properties(${SPDLOG_FAIL_COMPILATION_TARGET} PROPERTIES PASS_REGULAR_EXPRESSION "invalid type specifier")
    
    add_custom_target(${SPDLOG_FAIL_COMPILATION_TARGET}_run_target
        COMMAND ${CMAKE_CTEST_COMMAND} -R ${SPDLOG_FAIL_COMPILATION_TARGET} --output-on-failure
        COMMENT "Running tests that fail to compile.")
endif()
