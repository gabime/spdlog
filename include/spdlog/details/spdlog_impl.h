#pragma once

//
// Global registry functions
//
#include "registry.h"

inline std::shared_ptr<spdlog::logger> spdlog::get(const std::string& name)
{
    return details::registry::instance().get(name);
}

inline std::shared_ptr<spdlog::logger> spdlog::create(const std::string& logger_name, spdlog::sinks_init_list sinks)
{
    return details::registry::instance().create(logger_name, sinks);
}


template <typename Sink, typename... Args>
inline std::shared_ptr<spdlog::logger> spdlog::create(const std::string& logger_name, const Args&... args)
{
    sink_ptr sink = std::make_shared<Sink>(args...);
    return details::registry::instance().create(logger_name, { sink });
}


template<class It>
inline std::shared_ptr<spdlog::logger> spdlog::create(const std::string& logger_name, const It& sinks_begin, const It& sinks_end)
{
    return details::registry::instance().create(logger_name, sinks_begin, sinks_end);
}

inline void spdlog::set_formatter(spdlog::formatter_ptr f)
{
    details::registry::instance().formatter(f);
}

inline void spdlog::set_pattern(const std::string& format_string)
{
    return details::registry::instance().set_pattern(format_string);
}

inline void spdlog::set_level(level::level_enum log_level)
{
    return details::registry::instance().set_level(log_level);
}

inline void spdlog::stop()
{
    return details::registry::instance().stop_all();
}
