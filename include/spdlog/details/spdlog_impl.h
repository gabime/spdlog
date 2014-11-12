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
// Global registry functions
//
#include "registry.h"
#include "../sinks/file_sinks.h"
#include "../sinks/stdout_sinks.h"
#include "../sinks/syslog_sink.h"

inline std::shared_ptr<spdlog::logger> spdlog::get(const std::string& name)
{
    return details::registry::instance().get(name);
}


// Create multi/single threaded rotating file logger
inline std::shared_ptr<spdlog::logger> spdlog::rotating_logger_mt(const std::string& logger_name, const std::string& filename, size_t max_file_size, size_t max_files, size_t flush_inverval)
{
    return create<spdlog::sinks::rotating_file_sink_mt>(logger_name, filename, "txt", max_file_size, max_files, flush_inverval);
}

inline std::shared_ptr<spdlog::logger> spdlog::rotating_logger_st(const std::string& logger_name, const std::string& filename, size_t max_file_size, size_t max_files, size_t flush_inverval)
{
    return create<spdlog::sinks::rotating_file_sink_st>(logger_name, filename, "txt", max_file_size, max_files, flush_inverval);
}

// Create file logger which creates new file at midnight):
inline std::shared_ptr<spdlog::logger> spdlog::daily_logger_mt(const std::string& logger_name, const std::string& filename, size_t flush_inverval)
{
    return create<spdlog::sinks::daily_file_sink_mt>(logger_name, filename, "txt", flush_inverval);
}
inline std::shared_ptr<spdlog::logger> spdlog::daily_logger_st(const std::string& logger_name, const std::string& filename, size_t flush_inverval)
{
    return create<spdlog::sinks::daily_file_sink_st>(logger_name, filename, "txt", flush_inverval);
}


// Create stdout/stderr loggers
inline std::shared_ptr<spdlog::logger> spdlog::stdout_logger_mt(const std::string& logger_name)
{
    return create<spdlog::sinks::stdout_sink_mt>(logger_name);
}

inline std::shared_ptr<spdlog::logger> spdlog::stdout_logger_st(const std::string& logger_name)
{
    return create<spdlog::sinks::stdout_sink_st>(logger_name);
}

inline std::shared_ptr<spdlog::logger> spdlog::stderr_logger_mt(const std::string& logger_name)
{
    return create<spdlog::sinks::stderr_sink_mt>(logger_name);
}

inline std::shared_ptr<spdlog::logger> spdlog::stderr_logger_st(const std::string& logger_name)
{
    return create<spdlog::sinks::stderr_sink_st>(logger_name);
}

#ifdef __linux__
// Create syslog logger
inline std::shared_ptr<spdlog::logger> spdlog::syslog_logger(const std::string& logger_name)
{
    return create<spdlog::sinks::syslog_sink>(logger_name);
}

#endif


//Create logger with multiple sinks

inline std::shared_ptr<spdlog::logger> spdlog::create(const std::string& logger_name, spdlog::sinks_init_list sinks)
{
    return details::registry::instance().create(logger_name, sinks);
}


template <typename Sink, typename... Args>
inline std::shared_ptr<spdlog::logger> spdlog::create(const std::string& logger_name, const Args&... args)
{
    sink_ptr sink = std::make_shared<Sink>(args...);
    return details::registry::instance().create(logger_name, { sink });
}


template<class It>
inline std::shared_ptr<spdlog::logger> spdlog::create(const std::string& logger_name, const It& sinks_begin, const It& sinks_end)
{
    return details::registry::instance().create(logger_name, sinks_begin, sinks_end);
}

inline void spdlog::set_formatter(spdlog::formatter_ptr f)
{
    details::registry::instance().formatter(f);
}

inline void spdlog::set_pattern(const std::string& format_string)
{
    return details::registry::instance().set_pattern(format_string);
}

inline void spdlog::set_level(level::level_enum log_level)
{
    return details::registry::instance().set_level(log_level);
}

inline void spdlog::stop()
{
    return details::registry::instance().stop_all();
}
