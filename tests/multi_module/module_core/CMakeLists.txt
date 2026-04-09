add_library(module_core STATIC
    module_core.cpp
    module_core.h
)

target_link_libraries(module_core PUBLIC spdlog::spdlog)

target_include_directories(module_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

spdlog_enable_warnings(module_core)
