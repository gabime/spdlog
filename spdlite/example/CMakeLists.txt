project(spdlite-example CXX)

set(LITE_SOURCES example.cpp create_logger.cpp)

add_executable(${PROJECT_NAME} ${LITE_SOURCES})

find_package(Threads)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlite)


