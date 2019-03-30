cmake_minimum_required(VERSION 3.1)
project(spdlite)

include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h)

add_library(spdlite ${SRC_FILES} ${HEADER_FILES})

target_link_libraries(spdlite spdlog::spdlog)

add_subdirectory(example)    


