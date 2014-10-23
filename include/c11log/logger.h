#pragma once

// Thread safe logger
// Has name, log level, vector of std::shared sink pointers and formatter
// Upon each log write the logger:
// 1. Checks if its log level is enough to log the message
// 2. Format the message using the formatter function
// 3. Pass the formatted message to its sinks to performa the actual logging

#include<vector>
#include<memory>
#include<atomic>
#include <sstream>

#include "sinks/base_sink.h"
#include "common.h"
#include "details/pattern_formatter.h"

namespace c11log
{

namespace details
{
class line_logger;
}

class logger
{
public:

    logger(const std::string& name, sinks_init_list);
    template<class It>
    logger(const std::string& name, const It& begin, const It& end);


    //get/set default formatter
    static formatter_ptr& default_formatter(formatter_ptr formatter = nullptr);

    void set_formatter(formatter_ptr);
    formatter_ptr get_formatter() const;




    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;

    void level(level::level_enum);
    level::level_enum level() const;

    const std::string& name() const;
    bool should_log(level::level_enum) const;

    template <typename... Args> details::line_logger log(level::level_enum lvl, const Args&... args);
    template <typename... Args> details::line_logger trace(const Args&... args);
    template <typename... Args> details::line_logger debug(const Args&... args);
    template <typename... Args> details::line_logger info(const Args&... args);
    template <typename... Args> details::line_logger warn(const Args&... args);
    template <typename... Args> details::line_logger error(const Args&... args);
    template <typename... Args> details::line_logger critical(const Args&... args);

private:
    friend details::line_logger;
    std::string _name;
    formatter_ptr _formatter;
    std::vector<sink_ptr> _sinks;
    std::atomic_int _level;
    void _variadic_log(details::line_logger& l);
    template <typename First, typename... Rest>
    void _variadic_log(details::line_logger&l, const First& first, const Rest&... rest);
    void _log_msg(details::log_msg& msg);
};
}

//
// trace & debug macros
//
#ifdef FFLOG_ENABLE_TRACE
#define FFLOG_TRACE(logger, ...) logger->log(c11log::level::TRACE, __FILE__, " #", __LINE__,": " __VA_ARGS__)
#else
#define FFLOG_TRACE(logger, ...) {}
#endif

#ifdef FFLOG_ENABLE_DEBUG
#define FFLOG_DEBUG(logger, ...) logger->log(c11log::level::DEBUG, __VA_ARGS__)
#else
#define FFLOG_DEBUG(logger, ...) {}
#endif

//
// Logger implementation
//

#include "details/line_logger.h"


inline c11log::logger::logger(const std::string& logger_name, sinks_init_list sinks_list):
    _name(logger_name),
    _sinks(sinks_list)
{
    // no support under vs2013 for member initialization for std::atomic
    _level = level::INFO;
}

template<class It>
inline c11log::logger::logger(const std::string& logger_name, const It& begin, const It& end) :
    _name(logger_name),
    _sinks(begin, end)
{}


inline void c11log::logger::set_formatter(c11log::formatter_ptr msg_formatter)
{
    _formatter = msg_formatter;
}

inline c11log::formatter_ptr c11log::logger::get_formatter() const
{
    return _formatter;
}

inline c11log::formatter_ptr& c11log::logger::default_formatter(formatter_ptr formatter)
{
    static formatter_ptr g_default_formatter = std::make_shared<details::pattern_formatter>("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %t");
    if (formatter)
        g_default_formatter = formatter;
    return g_default_formatter;
}

template <typename... Args>
inline c11log::details::line_logger c11log::logger::log(level::level_enum lvl, const Args&... args) {
    bool msg_enabled = should_log(lvl);
    details::line_logger l(this, lvl, msg_enabled);
    if (msg_enabled)
        _variadic_log(l, args...);
    return l;
}

template <typename... Args>
inline c11log::details::line_logger c11log::logger::trace(const Args&... args)
{
    return log(level::TRACE, args...);
}

template <typename... Args>
inline c11log::details::line_logger c11log::logger::debug(const Args&... args)
{
    return log(level::DEBUG, args...);
}

template <typename... Args>
inline c11log::details::line_logger c11log::logger::info(const Args&... args)
{
    return log(level::INFO, args...);
}

template <typename... Args>
inline c11log::details::line_logger c11log::logger::warn(const Args&... args)
{
    return log(level::WARNING, args...);
}

template <typename... Args>
inline c11log::details::line_logger c11log::logger::error(const Args&... args)
{
    return log(level::ERROR, args...);
}

template <typename... Args>
inline c11log::details::line_logger c11log::logger::critical(const Args&... args)
{
    return log(level::CRITICAL, args...);
}

inline const std::string& c11log::logger::name() const
{
    return _name;
}

inline void c11log::logger::level(c11log::level::level_enum log_level)
{
    _level.store(log_level);
}

inline c11log::level::level_enum c11log::logger::level() const
{
    return static_cast<c11log::level::level_enum>(_level.load());
}

inline bool c11log::logger::should_log(c11log::level::level_enum msg_level) const
{
    return msg_level >= _level.load();
}




inline void c11log::logger::_variadic_log(c11log::details::line_logger&) {}

template <typename First, typename... Rest>
void c11log::logger::_variadic_log(c11log::details::line_logger& l, const First& first, const Rest&... rest)
{
    l.write(first);
    _variadic_log(l, rest...);
}

inline void c11log::logger::_log_msg(details::log_msg& msg)
{
    auto& formatter = _formatter ? _formatter : logger::default_formatter();
    formatter->format(msg);
    for (auto &sink : _sinks)
        sink->log(msg);
}


