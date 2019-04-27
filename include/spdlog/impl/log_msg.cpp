#include "spdlog/details/os.h"
#include "spdlog/sinks/sink.h"

#ifdef SPDLOG_STATIC_LIB
#include "spdlog/details/log_msg.h"
#endif

SPDLOG_INLINE spdlog::details::log_msg::log_msg(
    spdlog::source_loc loc, const std::string *loggers_name, spdlog::level::level_enum lvl, spdlog::string_view_t view)
    : logger_name(loggers_name)
    , level(lvl)
#ifndef SPDLOG_NO_DATETIME
    , time(os::now())
#endif

#ifndef SPDLOG_NO_THREAD_ID
    , thread_id(os::thread_id())
#endif
    , source(loc)
    , payload(view)
{
}

SPDLOG_INLINE spdlog::details::log_msg::log_msg(const std::string *loggers_name, spdlog::level::level_enum lvl, spdlog::string_view_t view)
    : log_msg(source_loc{}, loggers_name, lvl, view)
{
}