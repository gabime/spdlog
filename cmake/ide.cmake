#---------------------------------------------------------------------------------------
# IDE support for headers
#---------------------------------------------------------------------------------------
set(SPDLOG_HEADERS_DIR "${PROJECT_SOURCE_DIR}/include")
message(STATUS "${PROJECT_NAME} include path: ${SPDLOG_HEADERS_DIR}")


file(GLOB SPDLOG_TOP_HEADERS "${SPDLOG_HEADERS_DIR}/spdlog/*.h")
file(GLOB SPDLOG_DETAILS_HEADERS "${SPDLOG_HEADERS_DIR}/spdlog/details/*.h")
file(GLOB SPDLOG_SINKS_HEADERS "${SPDLOG_HEADERS_DIR}/spdlog/sinks/*.h")

if(NOT SPDLOG_FMT_EXTERNAL)
    file(GLOB SPDLOG_FMT_HEADERS "${SPDLOG_HEADERS_DIR}/spdlog/fmt/*.h")
    file(GLOB SPDLOG_FMT_BUNDELED_HEADERS "${SPDLOG_HEADERS_DIR}/spdlog/fmt/bundled/*.h")
endif()

set(SPDLOG_ALL_HEADERS ${SPDLOG_TOP_HEADERS} ${SPDLOG_DETAILS_HEADERS} ${SPDLOG_SINKS_HEADERS} ${SPDLOG_FMT_HEADERS} ${SPDLOG_FMT_BUNDELED_HEADERS})

source_group("Header Files\\spdlog" FILES ${SPDLOG_TOP_HEADERS})
source_group("Header Files\\spdlog\\details" FILES ${SPDLOG_DETAILS_HEADERS})
source_group("Header Files\\spdlog\\sinks" FILES ${SPDLOG_SINKS_HEADERS})

if(NOT SPDLOG_FMT_EXTERNAL)
    source_group("Header Files\\spdlog\\fmt" FILES ${SPDLOG_FMT_HEADERS})
    source_group("Header Files\\spdlog\\fmt\\bundled\\" FILES ${SPDLOG_FMT_BUNDELED_HEADERS})
endif()

