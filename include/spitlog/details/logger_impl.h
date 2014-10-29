#pragma once
//
// Logger implementation
//


#include "./line_logger.h"


inline spitlog::logger::logger(const std::string& logger_name, sinks_init_list sinks_list) :
    _name(logger_name),
    _sinks(sinks_list)
{
    // no support under vs2013 for member initialization for std::atomic
    _level = level::INFO;
}

template<class It>
inline spitlog::logger::logger(const std::string& logger_name, const It& begin, const It& end) :
    _name(logger_name),
    _sinks(begin, end)
{}


inline void spitlog::logger::set_formatter(spitlog::formatter_ptr msg_formatter)
{
    _formatter = msg_formatter;
}

inline void spitlog::logger::set_format(const std::string& format)
{
    _formatter = std::make_shared<pattern_formatter>(format);
}

inline spitlog::formatter_ptr spitlog::logger::get_formatter() const
{
    return _formatter;
}


template <typename... Args>
inline spitlog::details::line_logger spitlog::logger::log(level::level_enum lvl, const Args&... args) {
    bool msg_enabled = should_log(lvl);
    details::line_logger l(this, lvl, msg_enabled);
    if (msg_enabled)
        _variadic_log(l, args...);
    return l;
}

template <typename... Args>
inline spitlog::details::line_logger spitlog::logger::trace(const Args&... args)
{
    return log(level::TRACE, args...);
}

template <typename... Args>
inline spitlog::details::line_logger spitlog::logger::debug(const Args&... args)
{
    return log(level::DEBUG, args...);
}

template <typename... Args>
inline spitlog::details::line_logger spitlog::logger::info(const Args&... args)
{
    return log(level::INFO, args...);
}

template <typename... Args>
inline spitlog::details::line_logger spitlog::logger::warn(const Args&... args)
{
    return log(level::WARN, args...);
}

template <typename... Args>
inline spitlog::details::line_logger spitlog::logger::error(const Args&... args)
{
    return log(level::ERR, args...);
}

template <typename... Args>
inline spitlog::details::line_logger spitlog::logger::critical(const Args&... args)
{
    return log(level::CRITICAL, args...);
}

inline const std::string& spitlog::logger::name() const
{
    return _name;
}

inline void spitlog::logger::set_level(spitlog::level::level_enum log_level)
{
    _level.store(log_level);
}

inline spitlog::level::level_enum spitlog::logger::level() const
{
    return static_cast<spitlog::level::level_enum>(_level.load());
}

inline bool spitlog::logger::should_log(spitlog::level::level_enum msg_level) const
{
    return msg_level >= _level.load();
}

inline void spitlog::logger::_variadic_log(spitlog::details::line_logger&) {}

template <typename First, typename... Rest>
void spitlog::logger::_variadic_log(spitlog::details::line_logger& l, const First& first, const Rest&... rest)
{
    l.write(first);
    l.write(' ');
    _variadic_log(l, rest...);
}

inline void spitlog::logger::_log_msg(details::log_msg& msg)
{
    //Use default formatter if not set
    if (!_formatter)
        _formatter = std::make_shared<pattern_formatter>("%+");
    _formatter->format(msg);
    for (auto &sink : _sinks)
        sink->log(msg);
}

//
// Global registry functions
//
#include "./registry.h"
inline std::shared_ptr<spitlog::logger> spitlog::get(const std::string& name)
{
    return details::registry::instance().get(name);
}

inline std::shared_ptr<spitlog::logger> spitlog::create(const std::string& logger_name, spitlog::sinks_init_list sinks)
{
    return details::registry::instance().create(logger_name, sinks);
}


template <typename Sink, typename... Args>
inline std::shared_ptr<spitlog::logger> spitlog::create(const std::string& logger_name, const Args&... args)
{
    sink_ptr sink = std::make_shared<Sink>(args...);
    return details::registry::instance().create(logger_name, { sink });
}


template<class It>
inline std::shared_ptr<spitlog::logger> spitlog::create(const std::string& logger_name, const It& sinks_begin, const It& sinks_end)
{
    return details::registry::instance().create(logger_name, std::forward(sinks_begin), std::forward(sinks_end));
}

inline void spitlog::set_formatter(spitlog::formatter_ptr f)
{
    details::registry::instance().formatter(f);
}

inline void spitlog::set_format(const std::string& format_string)
{
    return details::registry::instance().set_format(format_string);
}
