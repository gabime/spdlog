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
#include "../formatter.h"
#include "./flag_formatter.h"
#include "./log_msg.h"
#include "./line_logger.h"


// create logger with given name, sinks and the default pattern formatter
// all other ctors will call this one
template<class It>
spdlog::logger::logger(const std::string& logger_name, const It& begin, const It& end) :
    _name(logger_name),
    _sinks(begin, end),
    _formatter(std::make_shared<pattern_formatter>("%+"))
{

    // no support under vs2013 for member initialization for std::atomic
    _level = level::info;
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

template<typename T>
inline spdlog::details::line_logger spdlog::logger::_log_if_enabled(level::level_enum lvl, const T& msg)
{
    bool msg_enabled = should_log(lvl);
    details::line_logger l(this, lvl, msg_enabled);
    l << msg;
    return l;
}

//
// logger.info(cppformat_string, arg1, arg2, arg3, ...) call style
//
template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::trace(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::trace, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::debug(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::debug, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::info(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::info, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::notice(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::notice, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::warn(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::warn, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::error(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::err, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::critical(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::critical, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::alert(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::alert, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::emerg(const char* fmt, const Args&... args)
{
    return _log_if_enabled(level::emerg, fmt, args...);
}

//
// logger.info(msg) << ".." call style
//
template<typename T>
inline spdlog::details::line_logger spdlog::logger::trace(const T& msg)
{
    return _log_if_enabled(level::trace, msg);
}

template<typename T>
inline spdlog::details::line_logger spdlog::logger::debug(const T& msg)
{
    return _log_if_enabled(level::debug, msg);
}


template<typename T>
inline spdlog::details::line_logger spdlog::logger::info(const T& msg)
{
    return _log_if_enabled(level::info, msg);
}

template<typename T>
inline spdlog::details::line_logger spdlog::logger::notice(const T& msg)
{
    return _log_if_enabled(level::notice, msg);
}

template<typename T>
inline spdlog::details::line_logger spdlog::logger::warn(const T& msg)
{
    return _log_if_enabled(level::warn, msg);
}

template<typename T>
inline spdlog::details::line_logger spdlog::logger::error(const T& msg)
{
    return _log_if_enabled(level::err, msg);
}

template<typename T>
inline spdlog::details::line_logger spdlog::logger::critical(const T& msg)
{
    return _log_if_enabled(level::critical, msg);
}

template<typename T>
inline spdlog::details::line_logger spdlog::logger::alert(const T& msg)
{
    return _log_if_enabled(level::alert, msg);
}

template<typename T>
inline spdlog::details::line_logger spdlog::logger::emerg(const T& msg)
{
    return _log_if_enabled(level::emerg, msg);
}

// always log, no matter what is the actual logger's log level
template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::force_log(level::level_enum lvl, const char* fmt, const Args&... args)
{
    details::line_logger l(this, lvl, true);
    l.write(fmt, args...);
    return l;
}

#ifndef SPDLOG_LIBRARY
#include "./logger_impl.cc"
#endif
