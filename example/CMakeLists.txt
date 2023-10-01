# Copyright(c) 2019 spdlog authors Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.11)
project(spdlog_example CXX)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

add_executable(example example.cpp)
target_link_libraries(example PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)

# copy dll to the executable folder for msvc
if(MSVC AND (SPDLOG_BUILD_SHARED OR BUILD_SHARED_LIBS))
    add_custom_command(TARGET example POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:spdlog>
                                                         $<TARGET_FILE_DIR:example>)
endif()
