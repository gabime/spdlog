add_library(module_network STATIC
    module_network.cpp
    module_network.h
)

target_link_libraries(module_network PUBLIC spdlog::spdlog)

target_include_directories(module_network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

spdlog_enable_warnings(module_network)
