cmake_minimum_required(VERSION 3.1)
project(spdlog_lite)

add_library(spdlog_lite logger.cpp)

target_link_libraries(spdlog_lite spdlog::spdlog)
