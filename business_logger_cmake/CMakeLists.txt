cmake_minimum_required(VERSION 3.10)
project(business_logger_example)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加spdlog库（使用header-only版本）
add_subdirectory(.. spdlog)

# 创建示例可执行文件
add_executable(business_logger_example ../business_logger_example.cpp)

# 链接spdlog
target_link_libraries(business_logger_example PRIVATE spdlog::spdlog)

# 设置输出目录
set_target_properties(business_logger_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
