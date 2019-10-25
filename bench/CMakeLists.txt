# Copyright(c) 2019 spdlog authors
# Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.1)
project(spdlog_bench CXX)

if(NOT TARGET spdlog)
  # Stand-alone build
  find_package(spdlog CONFIG REQUIRED)
endif()

find_package(Threads REQUIRED)
find_package(benchmark CONFIG REQUIRED)

add_executable(bench bench.cpp)
spdlog_enable_warnings(bench)
target_link_libraries(bench PRIVATE spdlog::spdlog)

add_executable(async_bench async_bench.cpp)
target_link_libraries(async_bench PRIVATE spdlog::spdlog)

add_executable(latency latency.cpp)
target_link_libraries(latency PRIVATE benchmark::benchmark spdlog::spdlog)

add_executable(formatter-bench formatter-bench.cpp)
target_link_libraries(formatter-bench PRIVATE benchmark::benchmark spdlog::spdlog)
