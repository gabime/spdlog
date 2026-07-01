// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// spdlog main header file.
// see example.cpp for usage example

#ifndef SPDLOG_H
#define SPDLOG_H

#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "./common.h"
#include "./logger.h"

SPDLOG_NAMESPACE_BEGIN

// Create a logger with a templated sink type
// Example:
//   spdlog::create<daily_file_sink_st>("logger_name", "dailylog_filename", 11, 59);
template <typename Sink, typename... SinkArgs>
std::shared_ptr<logger> create(std::string logger_name, SinkArgs &&...sink_args) {
    return std::make_shared<logger>(std::move(logger_name), std::make_shared<Sink>(std::forward<SinkArgs>(sink_args)...));
}

// Set formatter of the global logger. Each sink in each logger will get a clone of this object
SPDLOG_API void set_formatter(std::unique_ptr<formatter> formatter);

// Set format string of the global logger.
// example: spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e %l : %v");
SPDLOG_API void set_pattern(std::string pattern, pattern_time_type time_type = pattern_time_type::local);

// Get logging level of the global logger
SPDLOG_API level get_level();

// Set logging level of the global logger
SPDLOG_API void set_level(level level);

// Determine whether the global logger should log messages with a certain level
SPDLOG_API bool should_log(level log_level);

// Set flush level of the global logger.
SPDLOG_API void flush_on(level level);

// Set error handler for the global logger
SPDLOG_API void set_error_handler(void (*handler)(const std::string &msg));

// calls context::shutdown() to perform final cleanups
SPDLOG_API void shutdown();

// API for using global logger (stdout_color_mt),
// e.g: spdlog::info("Message {}", 1);
//
// The global logger object can be accessed using the spdlog::global_logger():
// For example, to add another sink to it:
// spdlog::global_logger()->sinks().push_back(some_sink);
//
// The global logger can be replaced using spdlog::set_global_logger(new_logger).
// For example, to replace it with a file logger.
//
// IMPORTANT:
// Do not call set_global_logger() from one thread while calling spdlog::info() from another.
SPDLOG_API std::shared_ptr<logger> global_logger();

// Set the global logger. (for example, to replace the global logger with a custom logger)
SPDLOG_API void set_global_logger(std::shared_ptr<logger> global_logger);

// Return the global logger raw pointer.
// To be used directly by the spdlog default API (e.g. spdlog::info)
// This make the default API faster, but cannot be used concurrently with set_global_logger().
// e.g do not call set_global_logger() from one thread while calling spdlog::info() from another.
SPDLOG_API logger *global_logger_raw() noexcept;

template <typename... Args>
void log(source_loc source, level lvl, format_string_t<Args...> fmt, Args &&...args) {
    global_logger_raw()->log(source, lvl, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log(level lvl, format_string_t<Args...> fmt, Args &&...args) {
    global_logger_raw()->log(lvl, fmt, std::forward<Args>(args)...);
}

inline void log(level lvl, std::string_view msg) { global_logger_raw()->log(lvl, msg); }

inline void log(source_loc loc, level lvl, std::string_view msg) { global_logger_raw()->log(loc, lvl, msg); }

template <typename... Args>
void trace(format_string_t<Args...> fmt, Args &&...args) {
    log(level::trace, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void debug(format_string_t<Args...> fmt, Args &&...args) {
    log(level::debug, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void info(format_string_t<Args...> fmt, Args &&...args) {
    log(level::info, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void warn(format_string_t<Args...> fmt, Args &&...args) {
    log(level::warn, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void error(format_string_t<Args...> fmt, Args &&...args) {
    log(level::err, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void critical(format_string_t<Args...> fmt, Args &&...args) {
    log(level::critical, fmt, std::forward<Args>(args)...);
}

// log functions with no format string, just string
inline void trace(std::string_view msg) { log(level::trace, msg); }

inline void debug(std::string_view msg) { log(level::debug, msg); }

inline void info(std::string_view msg) { log(level::info, msg); }

inline void warn(std::string_view msg) { log(level::warn, msg); }

inline void error(std::string_view msg) { log(level::err, msg); }

inline void critical(std::string_view msg) { log(level::critical, msg); }

SPDLOG_NAMESPACE_END

//
// enable/disable log calls at compile time according to global level.
//
// define SPDLOG_ACTIVE_LEVEL to one of those (before including spdlog.h):
// SPDLOG_LEVEL_TRACE,
// SPDLOG_LEVEL_DEBUG,
// SPDLOG_LEVEL_INFO,
// SPDLOG_LEVEL_WARN,
// SPDLOG_LEVEL_ERROR,
// SPDLOG_LEVEL_CRITICAL,
// SPDLOG_LEVEL_OFF
//

#ifndef SPDLOG_NO_SOURCE_LOC
    #define SPDLOG_LOGGER_CALL(logger, level, ...) \
        (logger)->log(SPDLOG_NAMESPACE::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, level, __VA_ARGS__)
#else
    #define SPDLOG_LOGGER_CALL(logger, level, ...) (logger)->log(SPDLOG_NAMESPACE::source_loc{}, level, __VA_ARGS__)
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE
    #define SPDLOG_LOGGER_TRACE(logger, ...) SPDLOG_LOGGER_CALL(logger, SPDLOG_NAMESPACE::level::trace, __VA_ARGS__)
    #define SPDLOG_TRACE(...) SPDLOG_LOGGER_TRACE(SPDLOG_NAMESPACE::global_logger_raw(), __VA_ARGS__)
#else
    #define SPDLOG_LOGGER_TRACE(logger, ...) (void)0
    #define SPDLOG_TRACE(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
    #define SPDLOG_LOGGER_DEBUG(logger, ...) SPDLOG_LOGGER_CALL(logger, SPDLOG_NAMESPACE::level::debug, __VA_ARGS__)
    #define SPDLOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(SPDLOG_NAMESPACE::global_logger(), __VA_ARGS__)
#else
    #define SPDLOG_LOGGER_DEBUG(logger, ...) (void)0
    #define SPDLOG_DEBUG(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_INFO
    #define SPDLOG_LOGGER_INFO(logger, ...) SPDLOG_LOGGER_CALL(logger, SPDLOG_NAMESPACE::level::info, __VA_ARGS__)
    #define SPDLOG_INFO(...) SPDLOG_LOGGER_INFO(SPDLOG_NAMESPACE::global_logger(), __VA_ARGS__)
#else
    #define SPDLOG_LOGGER_INFO(logger, ...) (void)0
    #define SPDLOG_INFO(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_WARN
    #define SPDLOG_LOGGER_WARN(logger, ...) SPDLOG_LOGGER_CALL(logger, SPDLOG_NAMESPACE::level::warn, __VA_ARGS__)
    #define SPDLOG_WARN(...) SPDLOG_LOGGER_WARN(SPDLOG_NAMESPACE::global_logger(), __VA_ARGS__)
#else
    #define SPDLOG_LOGGER_WARN(logger, ...) (void)0
    #define SPDLOG_WARN(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_ERROR
    #define SPDLOG_LOGGER_ERROR(logger, ...) SPDLOG_LOGGER_CALL(logger, SPDLOG_NAMESPACE::level::err, __VA_ARGS__)
    #define SPDLOG_ERROR(...) SPDLOG_LOGGER_ERROR(SPDLOG_NAMESPACE::global_logger(), __VA_ARGS__)
#else
    #define SPDLOG_LOGGER_ERROR(logger, ...) (void)0
    #define SPDLOG_ERROR(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_CRITICAL
    #define SPDLOG_LOGGER_CRITICAL(logger, ...) SPDLOG_LOGGER_CALL(logger, SPDLOG_NAMESPACE::level::critical, __VA_ARGS__)
    #define SPDLOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(SPDLOG_NAMESPACE::global_logger(), __VA_ARGS__)
#else
    #define SPDLOG_LOGGER_CRITICAL(logger, ...) (void)0
    #define SPDLOG_CRITICAL(...) (void)0
#endif

#endif  // SPDLOG_H
