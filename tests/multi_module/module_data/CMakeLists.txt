add_library(module_data STATIC
    module_data.cpp
    module_data.h
)

target_link_libraries(module_data PUBLIC spdlog::spdlog)

target_include_directories(module_data PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

spdlog_enable_warnings(module_data)
