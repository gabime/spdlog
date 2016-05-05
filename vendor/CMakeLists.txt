#
# External libraries
#
#
# Most of these libraries are used for running comparison benchmarks against
# other logging libraries.

if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/zf_log")
    add_subdirectory(zf_log)
endif()

if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/glog")
    add_subdirectory(glog)
endif()

if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/easyloggingpp")
    add_library(easylogging INTERFACE)
    set(SPDLOG_VENDORED_EASYLOGGING_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/easyloggingpp" CACHE INTERNAL "" FORCE)
    target_include_directories(easylogging INTERFACE "${SPDLOG_VENDORED_EASYLOGGING_ROOT}/src")
endif()

if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/g3log")
    add_subdirectory(g3log)
endif()

