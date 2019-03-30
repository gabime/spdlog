cmake_minimum_required(VERSION 3.1)
project(spdlog_lite)

add_library(spdlog_lite spdlite.cpp spdlite.h spdlite_global.cpp spdlite_global.h spdlite_macros.h)

target_link_libraries(spdlog_lite spdlog::spdlog)
