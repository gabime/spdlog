project(spdlog-utests CXX)
enable_testing()
find_package(Threads REQUIRED)

set(SPDLOG_UTESTS_SOURCES
    errors.cpp
    file_helper.cpp
    file_log.cpp
    test_misc.cpp   
	test_pattern_formatter.cpp
    test_async.cpp
    includes.h
    registry.cpp
    test_macros.cpp
    utils.cpp
    utils.h
    main.cpp)

add_executable(${PROJECT_NAME} ${SPDLOG_UTESTS_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/logs")
