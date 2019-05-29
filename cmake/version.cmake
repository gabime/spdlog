#---------------------------------------------------------------------------------------
# Get spdlog version from include/spdlog/version.h
#---------------------------------------------------------------------------------------
file(READ "${CMAKE_CURRENT_LIST_DIR}/../include/spdlog/version.h" SPDLOG_VERSION_FILE)
string(REGEX MATCH "SPDLOG_VER_MAJOR ([0-9]+)" _  "${SPDLOG_VERSION_FILE}")
set(ver_major ${CMAKE_MATCH_1})

string(REGEX MATCH "SPDLOG_VER_MINOR ([0-9]+)" _  "${SPDLOG_VERSION_FILE}")
set(ver_minor ${CMAKE_MATCH_1})
string(REGEX MATCH "SPDLOG_VER_PATCH ([0-9]+)" _  "${SPDLOG_VERSION_FILE}")
set(ver_patch ${CMAKE_MATCH_1})

if (NOT ver_major OR NOT ver_minor OR NOT ver_patch)
    message(FATAL_ERROR "Could not extract valid version from spdlog/version.h")
endif()
set (SPDLOG_VERSION "${ver_major}.${ver_minor}.${ver_patch}")    