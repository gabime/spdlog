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
#include "formatter.h"
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
    using sink_ptr = std::shared_ptr<sinks::sink>;
    using formatter_ptr = std::shared_ptr<formatter>;

    logger(const std::string& name, std::initializer_list<sink_ptr>, formatter_ptr formatter = nullptr);
    template<class It>
    logger(const std::string& name, const It& begin, const It& end);

    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;

    void level(level::level_enum);
    level::level_enum level() const;

    const std::string& name() const;
    bool should_log(level::level_enum) const;

    void formatter(formatter_ptr);
    formatter_ptr formatter() const;


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
    formatter_ptr  _default_formatter();
    const char* _default_pattern = "[%Y:%m:%d %H:%M:%S.%e] [%n:%l] %t";
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


inline c11log::logger::logger(const std::string& logger_name, std::initializer_list<sink_ptr> sinks_list, formatter_ptr formatter) :
    _name(logger_name),
    _formatter(formatter),
    _sinks(sinks_list)
{
    if (!formatter) //default formatter
        _formatter = std::make_shared<details::pattern_formatter>(_default_pattern);

    //Seems that vs2013 doesn't support atomic member initialization yet
    _level = level::INFO;

}

template<class It>
inline c11log::logger::logger(const std::string& logger_name, const It& begin, const It& end):
    _name(logger_name),
    _sinks(begin, end)
{}

template <typename... Args>
inline c11log::details::line_logger c11log::logger::log(level::level_enum lvl, const Args&... args) {
    details::line_logger l(this, lvl, true);
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


inline void c11log::logger::formatter(formatter_ptr formatter)
{
    _formatter = formatter;
}

inline c11log::logger::formatter_ptr c11log::logger::formatter() const
{
    return _formatter;
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
    _formatter->format(msg);
    for (auto &sink : _sinks)
        sink->log(msg);
}


