/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2014 Gabi Melman.                                       */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once
//
// Logger implementation
//

#include "./line_logger.h"



template<class It>
inline spdlog::logger::logger(const std::string& logger_name, const It& begin, const It& end) :
    _name(logger_name),
    _sinks(begin, end),
    _formatter(std::make_shared<pattern_formatter>("%+"))
{

    // no support under vs2013 for member initialization for std::atomic
    _level = level::INFO;
}

inline spdlog::logger::logger(const std::string& logger_name, sinks_init_list sinks_list) :
    logger(logger_name, sinks_list.begin(), sinks_list.end()) {}



inline spdlog::logger::logger(const std::string& logger_name, spdlog::sink_ptr single_sink) :logger(logger_name, { single_sink })
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
inline spdlog::details::line_logger spdlog::logger::log(level::level_enum lvl, const Args&... args)
{
    bool msg_enabled = should_log(lvl);
    details::line_logger l(this, lvl, msg_enabled);
    if (msg_enabled)
        _variadic_log(l, args...);
    return l;
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::log(const Args&... args)
{
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
inline spdlog::details::line_logger spdlog::logger::notice(const Args&... args)
{
    return log(level::NOTICE, args...);
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

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::alert(const Args&... args)
{
    return log(level::ALERT, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::emerg(const Args&... args)
{
    return log(level::EMERG, args...);
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

inline void spdlog::logger::stop()
{
    set_level(level::OFF);
}


inline void spdlog::logger::_variadic_log(spdlog::details::line_logger&) {}

template <typename Last>
inline void spdlog::logger::_variadic_log(spdlog::details::line_logger& l, const Last& last)
{
    l.write(last);
}



template <typename First, typename... Rest>
inline void spdlog::logger::_variadic_log(spdlog::details::line_logger& l, const First& first, const Rest&... rest)
{
    l.write(first);
    l.write(' ');
    _variadic_log(l, rest...);
}

inline void spdlog::logger::_log_msg(details::log_msg& msg)
{
    _formatter->format(msg);
    for (auto &sink : _sinks)
        sink->log(msg);
}

