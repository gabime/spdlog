include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang-format.cmake)

file(GLOB_RECURSE SPDLOG_FILES include/*.h)
file(GLOB_RECURSE SPDLOG_TEST_FILES tests/*.cpp tests/*.h)
file(GLOB_RECURSE SPDLOG_BENCH_FILES bench/*.cpp bench/*.h)

# TODO: list(FILTER) if cmake minimum is 3.6
foreach(item ${SPDLOG_FILES})
    string(REGEX MATCH ".+fmt/bundled/.*" item ${item})
    if(item)
        list(REMOVE_ITEM SPDLOG_FILES "${item}")
    endif()
endforeach()

if(CLANG_FORMAT_BIN AND CLANG_FORMAT_VERSION VERSION_EQUAL "5.0")
    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT_BIN} -i -style=file ${SPDLOG_FILES} ${SPDLOG_TEST_FILES} ${SPDLOG_BENCH_FILES}
    )
else()
    add_custom_target(
        format
        COMMAND echo "clang-format 5.0 not found"
    )
endif()
