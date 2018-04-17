//
// Copyright(c) 2015-2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
// spdlog main header file.
// see example.cpp for usage example

#pragma once

#include "details/registry.h"
#include "sinks/file_sinks.h"
#include "sinks/stdout_sinks.h"

#include "common.h"
#include "logger.h"

#if defined _WIN32
#include "sinks/wincolor_sink.h"
#else
#include "sinks/ansicolor_sink.h"
#endif

#ifdef __ANDROID__
#include "sinks/android_sink.h"
#endif

#include <chrono>
#include <functional>
#include <memory>
#include <string>

namespace spdlog {

// Default logger factory-  creates synchronous loggers
struct default_factory
{
    template<typename Sink, typename... SinkArgs>

    static std::shared_ptr<spdlog::logger> create(const std::string &logger_name, SinkArgs &&... args)
    {
        auto sink = std::make_shared<Sink>(std::forward<SinkArgs>(args)...);
        auto new_logger = std::make_shared<logger>(logger_name, std::move(sink));
        details::registry::instance().register_and_init(new_logger);
        return new_logger;
    }
};

// Create and register a logger with a templated sink type
// The logger's level, formatter and flush level will be set according the global settings.
// Example:
// spdlog::create<daily_file_sink_st>("logger_name", "dailylog_filename", 11, 59);
template<typename Sink, typename... SinkArgs>
inline std::shared_ptr<spdlog::logger> create(const std::string &logger_name, SinkArgs &&... sink_args)
{
    return default_factory::create<Sink>(logger_name, std::forward<SinkArgs>(sink_args)...);
}

//
// Return an existing logger or nullptr if a logger with such name doesn't exist.
// example: spdlog::get("my_logger")->info("hello {}", "world");
//
inline std::shared_ptr<logger> get(const std::string &name)
{
    return details::registry::instance().get(name);
}

//
// Set global formatting
// example: spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e %l : %v");
//
inline void set_pattern(const std::string &format_string)
{
    details::registry::instance().set_pattern(format_string);
}

inline void set_formatter(formatter_ptr f)
{
    details::registry::instance().set_formatter(std::move(f));
}

//
// Set global logging level
//
inline void set_level(level::level_enum log_level)
{
    details::registry::instance().set_level(log_level);
}

//
// Set global flush level
//
inline void flush_on(level::level_enum log_level)
{
    details::registry::instance().flush_on(log_level);
}

//
// Set global error handler
//
inline void set_error_handler(log_err_handler handler)
{
    details::registry::instance().set_error_handler(std::move(handler));
}

// Register the given logger with the given name
inline void register_logger(std::shared_ptr<logger> logger)
{
    details::registry::instance().register_logger(std::move(logger));
}

// Apply a user defined function on all registered loggers
// Example:
// spdlog::apply_all([&](std::shared_ptr<spdlog::logger> l) {l->flush();});
inline void apply_all(std::function<void(std::shared_ptr<logger>)> fun)
{
    details::registry::instance().apply_all(std::move(fun));
}

// Drop the reference to the given logger
inline void drop(const std::string &name)
{
    details::registry::instance().drop(name);
}

// Drop all references from the registry
inline void drop_all()
{
    details::registry::instance().drop_all();
}

//
// Create and register multi/single threaded basic file logger.
// Basic logger simply writes to given file without any limitations or rotations.
//
template<typename Factory = default_factory>
inline std::shared_ptr<logger> basic_logger_mt(const std::string &logger_name, const filename_t &filename, bool truncate = false)
{
    return Factory::template create<sinks::simple_file_sink_mt>(logger_name, filename, truncate);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> basic_logger_st(const std::string &logger_name, const filename_t &filename, bool truncate = false)
{
    return Factory::template create<sinks::simple_file_sink_st>(logger_name, filename, truncate);
}

//
// Create and register multi/single threaded rotating file logger
//
template<typename Factory = default_factory>
inline std::shared_ptr<logger> rotating_logger_mt(
    const std::string &logger_name, const filename_t &filename, size_t max_file_size, size_t max_files)
{
    return Factory::template create<sinks::rotating_file_sink_mt>(logger_name, filename, max_file_size, max_files);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> rotating_logger_st(
    const std::string &logger_name, const filename_t &filename, size_t max_file_size, size_t max_files)
{
    return Factory::template create<sinks::rotating_file_sink_st>(logger_name, filename, max_file_size, max_files);
}

//
// Create file logger which creates new file on the given time (default in midnight):
//
template<typename Factory = default_factory>
inline std::shared_ptr<logger> daily_logger_mt(const std::string &logger_name, const filename_t &filename, int hour = 0, int minute = 0)
{
    return Factory::template create<sinks::daily_file_sink_mt>(logger_name, filename, hour, minute);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> daily_logger_st(const std::string &logger_name, const filename_t &filename, int hour = 0, int minute = 0)
{
    return Factory::template create<sinks::daily_file_sink_st>(logger_name, filename, hour, minute);
}

//
// color console logger
//
template<typename Factory = default_factory>
inline std::shared_ptr<logger> stdout_color_mt(const std::string &logger_name)
{
	return Factory::template create<sinks::stdout_color_mt>(logger_name);
}
template<typename Factory = default_factory>
inline std::shared_ptr<logger> stdout_color_st(const std::string &logger_name)
{
	return Factory::template create<sinks::stdout_color_st>(logger_name);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> stderr_color_mt(const std::string &logger_name)
{
	return Factory::template create<sinks::stderr_color_mt>(logger_name);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> stderr_color_st(const std::string &logger_name)
{
	return Factory::template create<sinks::stderr_color_st>(logger_name);
}

// console loggers (no colors)
template<typename Factory = default_factory>
inline std::shared_ptr<logger> stdout_logger_mt(const std::string &logger_name)
{
	return Factory::template create<sinks::stdout_sink_mt>(logger_name);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> stdout_logger_st(const std::string &logger_name)
{
	return Factory::template create<sinks::stdout_sink_st>(logger_name);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> stderr_logger_mt(const std::string &logger_name)
{
	return Factory::template create<sinks::stderr_sink_mt>(logger_name);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> stderr_logger_st(const std::string &logger_name)
{
	return Factory::template create<sinks::stderr_sink_st>(logger_name);
}


#ifdef SPDLOG_ENABLE_SYSLOG
// Create and register a syslog logger
template<typename Factory = default_factory>
inline std::shared_ptr<logger> syslog_logger(
    const std::string &logger_name, const std::string &ident = "", int syslog_option = 0, int syslog_facilty = (1 << 3))
{
    return return Factory::template create<sinks::syslog_sink>(logger_name, syslog_ident, syslog_option, syslog_facility);
}
#endif

#if defined(__ANDROID__)
// Create and register android syslog logger
template<typename Factory = default_factory>
inline std::shared_ptr<logger> android_logger(const std::string &logger_name, const std::string &tag = "spdlog")
{
    return return Factory::template create<sinks::android_sink>(logger_name, tag);
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Trace & Debug can be switched on/off at compile time for zero cost debug statements.
// Uncomment SPDLOG_DEBUG_ON/SPDLOG_TRACE_ON in tweakme.h to enable.
// SPDLOG_TRACE(..) will also print current file and line.
//
// Example:
// spdlog::set_level(spdlog::level::trace);
// SPDLOG_TRACE(my_logger, "some trace message");
// SPDLOG_TRACE(my_logger, "another trace message {} {}", 1, 2);
// SPDLOG_DEBUG(my_logger, "some debug message {} {}", 3, 4);
///////////////////////////////////////////////////////////////////////////////

#ifdef SPDLOG_TRACE_ON
#define SPDLOG_STR_H(x) #x
#define SPDLOG_STR_HELPER(x) SPDLOG_STR_H(x)
#ifdef _MSC_VER
#define SPDLOG_TRACE(logger, ...) logger->trace("[ " __FILE__ "(" SPDLOG_STR_HELPER(__LINE__) ") ] " __VA_ARGS__)
#else
#define SPDLOG_TRACE(logger, ...) logger->trace("[ " __FILE__ ":" SPDLOG_STR_HELPER(__LINE__) " ] " __VA_ARGS__)
#endif
#else
#define SPDLOG_TRACE(logger, ...) (void)0
#endif

#ifdef SPDLOG_DEBUG_ON
#define SPDLOG_DEBUG(logger, ...) logger->debug(__VA_ARGS__)
#else
#define SPDLOG_DEBUG(logger, ...) (void)0
#endif

} // namespace spdlog
