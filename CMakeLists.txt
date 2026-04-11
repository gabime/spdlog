cmake_minimum_required(VERSION 3.10)
project(BusinessLoggerExample)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加 spdlog 库
add_subdirectory(spdlog/spdlog-1.x)

# 创建可执行文件
add_executable(business_logger_example business_logger_example.cpp)

# 链接 spdlog
target_link_libraries(business_logger_example PRIVATE spdlog::spdlog)

# 如果是 MSVC，设置 UTF-8 编码
if(MSVC)
    target_compile_options(business_logger_example PRIVATE /utf-8)
endif()
