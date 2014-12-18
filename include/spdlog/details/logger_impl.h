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


// create logger with given name, sinks and the default pattern formatter
// all other ctors will call this one
template<class It>
inline spdlog::logger::logger(const std::string& logger_name, const It& begin, const It& end) :
    _name(logger_name),
    _sinks(begin, end),
    _formatter(std::make_shared<pattern_formatter>("%+"))
{

    // no support under vs2013 for member initialization for std::atomic
    _level = level::INFO;
}

// ctor with sinks as init list
inline spdlog::logger::logger(const std::string& logger_name, sinks_init_list sinks_list) :
    logger(logger_name, sinks_list.begin(), sinks_list.end()) {}


// ctor with single sink
inline spdlog::logger::logger(const std::string& logger_name, spdlog::sink_ptr single_sink) :
    logger(logger_name, { single_sink }) {}


inline spdlog::logger::~logger() = default;


inline void spdlog::logger::set_formatter(spdlog::formatter_ptr msg_formatter)
{
    _set_formatter(msg_formatter);
}

inline void spdlog::logger::set_pattern(const std::string& pattern)
{
    _set_pattern(pattern);
}

//
// log only if given level>=logger's log level
//


template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::_log_if_enabled(level::level_enum lvl, const char* fmt, const Args&... args)
{
    bool msg_enabled = should_log(lvl);
    details::line_logger l(this, lvl, msg_enabled);
    l.write(fmt, args...);
    return l;
}

inline spdlog::details::line_logger spdlog::logger::_log_if_enabled(level::level_enum lvl)
{
    return details::line_logger(this, lvl, should_log(lvl));
}


//
// following functions will log only if at the right level
//
template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::trace(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::TRACE, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::debug(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::DEBUG, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::info(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::INFO, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::notice(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::NOTICE, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::warn(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::WARN, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::error(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::ERR, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::critical(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::CRITICAL, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::alert(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::ALERT, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::emerg(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::EMERG, fmt, args...);
}


//
// support logger.info() << ".." calls
//



inline spdlog::details::line_logger spdlog::logger::trace()
{
    return _log_if_enabled(level::TRACE);
}


inline spdlog::details::line_logger spdlog::logger::debug()
{
    return _log_if_enabled(level::DEBUG);
}

inline spdlog::details::line_logger spdlog::logger::info()
{
    return _log_if_enabled(level::INFO);
}

inline spdlog::details::line_logger spdlog::logger::notice()
{
    return _log_if_enabled(level::NOTICE);
}

inline spdlog::details::line_logger spdlog::logger::warn()
{
    return _log_if_enabled(level::WARN);
}

inline spdlog::details::line_logger spdlog::logger::error()
{
    return _log_if_enabled(level::ERR);
}

inline spdlog::details::line_logger spdlog::logger::critical()
{
    return _log_if_enabled(level::CRITICAL);
}

inline spdlog::details::line_logger spdlog::logger::alert()
{
    return _log_if_enabled(level::ALERT);
}

inline spdlog::details::line_logger spdlog::logger::emerg()
{
    return _log_if_enabled(level::EMERG);
}


// always log, no matter what is the actual logger's log level
template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::force_log(level::level_enum lvl, const char* fmt, const Args&... args)
{
    details::line_logger l(this, lvl, true);
    l.write(fmt, args...);
    return l;
}

//
// name and level
//
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
    _stop();
}

//
// protected virtual called at end of each user log call (if enabled) by the line_logger
//
inline void spdlog::logger::_log_msg(details::log_msg& msg)
{
    _formatter->format(msg);
    for (auto &sink : _sinks)
        sink->log(msg);
}

inline void spdlog::logger::_set_pattern(const std::string& pattern)
{
    _formatter = std::make_shared<pattern_formatter>(pattern);
}
inline void spdlog::logger::_set_formatter(formatter_ptr msg_formatter)
{
    _formatter = msg_formatter;
}

inline void spdlog::logger::_stop()
{
    set_level(level::OFF);
}





