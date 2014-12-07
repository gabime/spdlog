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


/* public functions */
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


inline spdlog::logger::~logger() {}

inline void spdlog::logger::set_formatter(spdlog::formatter_ptr msg_formatter)
{
    _set_formatter(msg_formatter);
}

inline void spdlog::logger::set_pattern(const std::string& pattern)
{
    _set_pattern(pattern);
}

//
// cppformat API of the form logger.info("hello {} {}", "world", 1);
//
template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::_log(level::level_enum lvl, const std::string& fmt, const Args&... args)
{
    bool msg_enabled = should_log(lvl);
    details::line_logger l(this, lvl, msg_enabled);
    if (msg_enabled)
    {
        try
        {
            l.write(fmt, args...);
        }
        catch(const fmt::FormatError& e)
        {
            throw spdlog_ex(fmt::format("formatting error while processing format string '{}': {}", fmt, e.what()));
        }

    }
    return l;
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::log(const std::string& fmt, const Args&... args)
{
    return _log(level::ALWAYS, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::trace(const std::string& fmt, const Args&... args)
{
    return _log(level::TRACE, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::debug(const std::string& fmt, const Args&... args)
{
    return _log(level::DEBUG, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::info(const std::string& fmt, const Args&... args)
{
    return _log(level::INFO, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::notice(const std::string& fmt, const Args&... args)
{
    return _log(level::NOTICE, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::warn(const std::string& fmt, const Args&... args)
{
    return _log(level::WARN, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::error(const std::string& fmt, const Args&... args)
{
    return _log(level::ERR, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::critical(const std::string& fmt, const Args&... args)
{
    return _log(level::CRITICAL, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::alert(const std::string& fmt, const Args&... args)
{
    return _log(level::ALERT, fmt, args...);
}

template <typename... Args>
inline spdlog::details::line_logger spdlog::logger::emerg(const std::string& fmt, const Args&... args)
{
    return _log(level::EMERG, fmt, args...);
}


//
// //API to support logger.info() << ".." calls
//


inline spdlog::details::line_logger spdlog::logger::_log(level::level_enum lvl)
{
    bool msg_enabled = should_log(lvl);
    details::line_logger l(this, lvl, msg_enabled);
    return l;
}

inline spdlog::details::line_logger spdlog::logger::log()
{
    return _log(level::ALWAYS);
}

inline spdlog::details::line_logger spdlog::logger::trace()
{
    return _log(level::TRACE);
}


inline spdlog::details::line_logger spdlog::logger::debug()
{
    return _log(level::DEBUG);
}

inline spdlog::details::line_logger spdlog::logger::info()
{
    return _log(level::INFO);
}

inline spdlog::details::line_logger spdlog::logger::notice()
{
    return _log(level::NOTICE);
}

inline spdlog::details::line_logger spdlog::logger::warn()
{
    return _log(level::WARN);
}

inline spdlog::details::line_logger spdlog::logger::error()
{
    return _log(level::ERR);
}

inline spdlog::details::line_logger spdlog::logger::critical()
{
    return _log(level::CRITICAL);
}

inline spdlog::details::line_logger spdlog::logger::alert()
{
    return _log(level::ALERT);
}

inline spdlog::details::line_logger spdlog::logger::emerg()
{
    return _log(level::EMERG);
}



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

/* protected virtual */
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





