#
# External libraries
#
#
# Most of these libraries are used for running comparison benchmarks against
# other logging libraries.

add_subdirectory(zf_log)

#add_subdirectory(glog)

add_library(easylogging INTERFACE)
set(SPDLOG_VENDORED_EASYLOGGING_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/easyloggingpp" CACHE INTERNAL "" FORCE)
target_include_directories(easylogging INTERFACE "${SPDLOG_VENDORED_EASYLOGGING_ROOT}/src")

#add_library(zflog INTERFACE)
#set(SPDLOG_VENDORED_ZFLOG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/zf_log" CACHE INTERNAL "" FORCE)
#target_include_directories(zflog INTERFACE "${SPDLOG_VENDORED_ZFLOG_ROOT}/zf_log")
##target_compile_definitions(zflog INTERFACE ZFLOG_DEFINITIONS)


