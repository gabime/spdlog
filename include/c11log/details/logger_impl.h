#pragma once
//
// Logger implementation
//


#include "details/line_logger.h"


inline c11log::logger::logger(const std::string& logger_name, sinks_init_list sinks_list) :
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

inline void c11log::logger::set_format(const std::string& format)
{
    _formatter = std::make_shared<pattern_formatter>(format);
}

inline c11log::formatter_ptr c11log::logger::get_formatter() const
{
    return _formatter;
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
    return log(level::WARN, args...);
}

template <typename... Args>
inline c11log::details::line_logger c11log::logger::error(const Args&... args)
{
    return log(level::ERR, args...);
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

inline void c11log::logger::set_level(c11log::level::level_enum log_level)
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
    l.write(' ');
    _variadic_log(l, rest...);
}

inline void c11log::logger::_log_msg(details::log_msg& msg)
{
    if (!_formatter)
        _formatter = std::make_shared<pattern_formatter>("%+ %t");
    _formatter->format(msg);
    for (auto &sink : _sinks)
        sink->log(msg);
}

//
// Global registry functions
//
#include "details/registry.h"
inline std::shared_ptr<c11log::logger> c11log::get(const std::string& name)
{
    return details::registry::instance().get(name);
}

inline std::shared_ptr<c11log::logger> c11log::create(const std::string& logger_name, c11log::sinks_init_list sinks)
{
    return details::registry::instance().create(logger_name, sinks);
}


template <typename Sink, typename... Args>
inline std::shared_ptr<c11log::logger> c11log::create(const std::string& logger_name, const Args&... args)
{
    sink_ptr sink = std::make_shared<Sink>(args...);
    return details::registry::instance().create(logger_name, { sink });
}


template<class It>
inline std::shared_ptr<c11log::logger> c11log::create(const std::string& logger_name, const It& sinks_begin, const It& sinks_end)
{
    return details::registry::instance().create(logger_name, std::forward(sinks_begin), std::forward(sinks_end));
}


inline void c11log::formatter(c11log::formatter_ptr f)
{
    return details::registry::instance().formatter(f);
}

inline c11log::formatter_ptr c11log::formatter()
{
    return details::registry::instance().formatter();
}

inline void c11log::set_format(const std::string& format_string)
{
    return details::registry::instance().set_format(format_string);
}
