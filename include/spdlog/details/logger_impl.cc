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

#ifdef SPDLOG_LIBRARY
#include "../logger.h"
#else
#pragma once
#endif
#include "../sinks/sink.h"
#include "./line_logger.h"
#include "./config.h"

// ctor with sinks as init list
SPDLOG_INLINE spdlog::logger::logger(const std::string& logger_name, sinks_init_list sinks_list) :
    logger(logger_name, sinks_list.begin(), sinks_list.end()) {}


// ctor with single sink
SPDLOG_INLINE spdlog::logger::logger(const std::string& logger_name, spdlog::sink_ptr single_sink) :
    logger(logger_name, {
    single_sink
}) {}


SPDLOG_INLINE spdlog::logger::~logger() = default;


SPDLOG_INLINE void spdlog::logger::set_formatter(spdlog::formatter_ptr msg_formatter)
{
    _set_formatter(msg_formatter);
}

SPDLOG_INLINE void spdlog::logger::set_pattern(const std::string& pattern)
{
    _set_pattern(pattern);
}

//
// log only if given level>=logger's log level
//

SPDLOG_INLINE spdlog::details::line_logger spdlog::logger::_log_if_enabled(level::level_enum lvl)
{
    return details::line_logger(this, lvl, should_log(lvl));
}


//
// logger.info() << ".." call  style
//
SPDLOG_INLINE spdlog::details::line_logger spdlog::logger::trace()
{
    return _log_if_enabled(level::trace);
}

SPDLOG_INLINE spdlog::details::line_logger spdlog::logger::debug()
{
    return _log_if_enabled(level::debug);
}

SPDLOG_INLINE spdlog::details::line_logger spdlog::logger::info()
{
    return _log_if_enabled(level::info);
}

SPDLOG_INLINE spdlog::details::line_logger spdlog::logger::notice()
{
    return _log_if_enabled(level::notice);
}

SPDLOG_INLINE spdlog::details::line_logger spdlog::logger::warn()
{
    return _log_if_enabled(level::warn);
}

SPDLOG_INLINE spdlog::details::line_logger spdlog::logger::error()
{
    return _log_if_enabled(level::err);
}

SPDLOG_INLINE spdlog::details::line_logger spdlog::logger::critical()
{
    return _log_if_enabled(level::critical);
}

SPDLOG_INLINE spdlog::details::line_logger spdlog::logger::alert()
{
    return _log_if_enabled(level::alert);
}

SPDLOG_INLINE spdlog::details::line_logger spdlog::logger::emerg()
{
    return _log_if_enabled(level::emerg);
}

//
// name and level
//
SPDLOG_INLINE const std::string& spdlog::logger::name() const
{
    return _name;
}

SPDLOG_INLINE void spdlog::logger::set_level(spdlog::level::level_enum log_level)
{
    _level.store(log_level);
}

SPDLOG_INLINE spdlog::level::level_enum spdlog::logger::level() const
{
    return static_cast<spdlog::level::level_enum>(_level.load(std::memory_order_relaxed));
}

SPDLOG_INLINE bool spdlog::logger::should_log(spdlog::level::level_enum msg_level) const
{
    return msg_level >= _level.load(std::memory_order_relaxed);
}

//
// protected virtual called at end of each user log call (if enabled) by the line_logger
//
SPDLOG_INLINE void spdlog::logger::_log_msg(details::log_msg& msg)
{
    _formatter->format(msg);
    for (auto &sink : _sinks)
        sink->log(msg);
}

SPDLOG_INLINE void spdlog::logger::_set_pattern(const std::string& pattern)
{
    _formatter = std::make_shared<pattern_formatter>(pattern);
}

SPDLOG_INLINE void spdlog::logger::_set_formatter(formatter_ptr msg_formatter)
{
    _formatter = msg_formatter;
}

SPDLOG_INLINE void spdlog::logger::flush() {
    for (auto& sink : _sinks)
        sink->flush();
}
