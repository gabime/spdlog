cmake_minimum_required(VERSION 3.0)
project(spdlog CXX)
set(${PROJECT_NAME}_SRCS
  include/spdlog/details/spdlog_impl.cc
  include/spdlog/details/logger_impl.cc
  include/spdlog/details/async_logger_impl.cc
  include/spdlog/details/async_log_helper.cc
  include/spdlog/details/format.cc
  include/spdlog/details/pattern_formatter_impl.cc
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SRCS})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
target_compile_definitions(${PROJECT_NAME} PUBLIC SPDLOG_LIBRARY)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(tests)
