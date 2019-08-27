project(spdlog_utests CXX)

include(../cmake/utils.cmake)

find_package(PkgConfig)
if(PkgConfig_FOUND)
	pkg_check_modules(systemd libsystemd)
endif()

set(SPDLOG_UTESTS_SOURCES
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
    test_fmt_helper.cpp
  	test_stdout_api.cpp
    test_dup_filter.cpp
	test_backtrace.cpp)

if(NOT SPDLOG_NO_EXCEPTIONS)
	list(APPEND SPDLOG_UTESTS_SOURCES test_errors.cpp)
endif()

if(systemd_FOUND)
	list(APPEND SPDLOG_UTESTS_SOURCES test_systemd.cpp)
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/logs")
enable_testing()

# The compiled library tests
if(SPDLOG_BUILD_TESTS)
    add_executable(spdlog-utests ${SPDLOG_UTESTS_SOURCES})
	spdlog_enable_warnings(spdlog-utests)
    target_link_libraries(spdlog-utests PRIVATE spdlog)

	if(systemd_FOUND)
		target_link_libraries(spdlog-utests PRIVATE ${systemd_LIBRARIES})
	endif()
	if(SPDLOG_SANITIZE_ADDRESS)
		spdlog_enable_sanitizer(spdlog-utests)
	endif()
    add_test(NAME spdlog-utests COMMAND spdlog-utests)
endif()

# The header-only library version tests
if(SPDLOG_BUILD_TESTS_HO)
    add_executable(spdlog-utests-ho ${SPDLOG_UTESTS_SOURCES})
	spdlog_enable_warnings(spdlog-utests-ho)
    target_link_libraries(spdlog-utests-ho PRIVATE spdlog::spdlog_header_only)
	if(systemd_FOUND)
		target_link_libraries(spdlog-utests-ho PRIVATE ${systemd_LIBRARIES})
	endif()
	if(SPDLOG_SANITIZE_ADDRESS)
		spdlog_set_address_sanitizer(spdlog-utests-ho)
	endif()
    add_test(NAME spdlog-utests-ho COMMAND spdlog-utests-ho)
endif()
