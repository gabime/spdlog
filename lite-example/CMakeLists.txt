project(spdlog-lite-example CXX)

find_package(Threads REQUIRED)

set(LITE_SOURCES example.cpp create_lite.cpp)

add_executable(${PROJECT_NAME} ${LITE_SOURCES})

include_directories(../lite)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog_lite)


