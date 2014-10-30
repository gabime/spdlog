#pragma once
//
// Logger implementation
//


#include "./line_logger.h"


inline spdlog::logger::logger(const std::string& logger_name, sinks_init_list sinks_list) :
    _name(logger_name),
    _sinks(sinks_list)
{
    // no support under vs2013 for member initialization for std::atomic
    _level = level::INFO;
}

template<class It>
inline spdlog::logger::logger(const std::string& logger_name, const It& begin, const It& end) :
    _name(logger_name),
    _sinks(begin, end)
{}


inline void spdlog::logger::set_formatter(spdlog::formatter_ptr msg_formatter)
{
    _formatter = msg_formatter;
}

inline void spdlog::logger::set_pattern(const std::string& pattern)
{
    _formatter = std::make_shared<pattern_formatter>(pattern);
}

inline spdlog::formatter_ptr spdlog::logger::get_formatter() const
{
    return _formatter;
}


template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::log(level::level_enum lvl, const Args&... args) {
    bool msg_enabled = should_log(lvl);
    details::line_logger l(this, lvl, msg_enabled);
    if (msg_enabled)
        _variadic_log(l, args...);
    return l;
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::log(const Args&... args) {
    return log(level::ALWAYS, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::trace(const Args&... args)
{
    return log(level::TRACE, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::debug(const Args&... args)
{
    return log(level::DEBUG, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::info(const Args&... args)
{
    return log(level::INFO, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::warn(const Args&... args)
{
    return log(level::WARN, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::error(const Args&... args)
{
    return log(level::ERR, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::critical(const Args&... args)
{
    return log(level::CRITICAL, args...);
}

inline const std::string& spdlog::logger::name() const
{
    return _name;
}

inline void spdlog::logger::set_level(spdlog::level::level_enum log_level)
{
    _level.store(log_level);
}

inline spdlog::level::level_enum spdlog::logger::level() const
{
    return static_cast<spdlog::level::level_enum>(_level.load());
}

inline bool spdlog::logger::should_log(spdlog::level::level_enum msg_level) const
{
    return msg_level >= _level.load();
}

inline void spdlog::logger::stop_logging()
{
    set_level(level::OFF);
}


inline void spdlog::logger::_variadic_log(spdlog::details::line_logger&) {}

template <typename First, typename... Rest>
void spdlog::logger::_variadic_log(spdlog::details::line_logger& l, const First& first, const Rest&... rest)
{
    l.write(first);
    l.write(' ');
    _variadic_log(l, rest...);
}

inline void spdlog::logger::_log_msg(details::log_msg& msg)
{
    //Use default formatter if not set
    if (!_formatter)
        _formatter = std::make_shared<pattern_formatter>("%+");
    _formatter->format(msg);
    for (auto &sink : _sinks)
        sink->log(msg);
}

