//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
// spdlog main header file.
// see example.cpp for usage example

#pragma once

#include "common.h"
#include "logger.h"

#include <chrono>
#include <functional>
#include <memory>
#include <string>

namespace spdlog {

//
// Return an existing logger or nullptr if a logger with such name doesn't exist.
// example: spdlog::get("my_logger")->info("hello {}", "world");
//
shared_ptr<logger> get(const string &name);

//
// Set global formatting
// example: spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e %l : %v");
//
void set_pattern(const string &format_string);
void set_formatter(formatter_ptr f);

//
// Set global logging level
//
void set_level(level::level_enum log_level);

//
// Set global flush level
//
void flush_on(level::level_enum log_level);

//
// Set global error handler
//
void set_error_handler(log_err_handler handler);

//
// Turn on async mode (off by default) and set the queue size for each async_logger.
// effective only for loggers created after this call.
// queue_size: size of queue (must be power of 2):
//    Each logger will pre-allocate a dedicated queue with queue_size entries upon construction.
//
// async_overflow_policy (optional, block_retry by default):
//    async_overflow_policy::block_retry - if queue is full, block until queue has room for the new log entry.
//    async_overflow_policy::discard_log_msg - never block and discard any new messages when queue overflows.
//
// worker_warmup_cb (optional):
//     callback function that will be called in worker thread upon start (can be used to init stuff like thread affinity)
//
// worker_teardown_cb (optional):
//     callback function that will be called in worker thread upon exit
//
void set_async_mode(size_t queue_size, const async_overflow_policy overflow_policy = async_overflow_policy::block_retry,
    const std::function<void()> &worker_warmup_cb = nullptr,
    const std::chrono::milliseconds &flush_interval_ms = std::chrono::milliseconds::zero(),
    const std::function<void()> &worker_teardown_cb = nullptr);

// Turn off async mode
void set_sync_mode();

//
// Create and register multi/single threaded basic file logger.
// Basic logger simply writes to given file without any limitations or rotations.
//
shared_ptr<logger> basic_logger_mt(const string &logger_name, const filename_t &filename, bool truncate = false);
shared_ptr<logger> basic_logger_st(const string &logger_name, const filename_t &filename, bool truncate = false);

//
// Create and register multi/single threaded rotating file logger
//
shared_ptr<logger> rotating_logger_mt(
    const string &logger_name, const filename_t &filename, size_t max_file_size, size_t max_files);

shared_ptr<logger> rotating_logger_st(
    const string &logger_name, const filename_t &filename, size_t max_file_size, size_t max_files);

//
// Create file logger which creates new file on the given time (default in midnight):
//
shared_ptr<logger> daily_logger_mt(const string &logger_name, const filename_t &filename, int hour = 0, int minute = 0);
shared_ptr<logger> daily_logger_st(const string &logger_name, const filename_t &filename, int hour = 0, int minute = 0);

//
// Create and register stdout/stderr loggers
//
shared_ptr<logger> stdout_logger_mt(const string &logger_name);
shared_ptr<logger> stdout_logger_st(const string &logger_name);
shared_ptr<logger> stderr_logger_mt(const string &logger_name);
shared_ptr<logger> stderr_logger_st(const string &logger_name);
//
// Create and register colored stdout/stderr loggers
//
shared_ptr<logger> stdout_color_mt(const string &logger_name);
shared_ptr<logger> stdout_color_st(const string &logger_name);
shared_ptr<logger> stderr_color_mt(const string &logger_name);
shared_ptr<logger> stderr_color_st(const string &logger_name);

//
// Create and register a syslog logger
//
#ifdef SPDLOG_ENABLE_SYSLOG
shared_ptr<logger> syslog_logger(
    const string &logger_name, const string &ident = "", int syslog_option = 0, int syslog_facilty = (1 << 3));
#endif

#if defined(__ANDROID__)
shared_ptr<logger> android_logger(const string &logger_name, const string &tag = "spdlog");
#endif

// Create and register a logger with a single sink
shared_ptr<logger> create(const string &logger_name, const sink_ptr &sink);

// Create and register a logger with multiple sinks
shared_ptr<logger> create(const string &logger_name, sinks_init_list sinks);

template<class It>
shared_ptr<logger> create(const string &logger_name, const It &sinks_begin, const It &sinks_end);

// Create and register a logger with templated sink type
// Example:
// spdlog::create<daily_file_sink_st>("mylog", "dailylog_filename");
template<typename Sink, typename... Args>
shared_ptr<spdlog::logger> create(const string &logger_name, Args... args);

// Create and register an async logger with a single sink
shared_ptr<logger> create_async(const string &logger_name, const sink_ptr &sink, size_t queue_size,
    const async_overflow_policy overflow_policy = async_overflow_policy::block_retry,
    const std::function<void()> &worker_warmup_cb = nullptr,
    const std::chrono::milliseconds &flush_interval_ms = std::chrono::milliseconds::zero(),
    const std::function<void()> &worker_teardown_cb = nullptr);

// Create and register an async logger with multiple sinks
shared_ptr<logger> create_async(const string &logger_name, sinks_init_list sinks, size_t queue_size,
    const async_overflow_policy overflow_policy = async_overflow_policy::block_retry,
    const std::function<void()> &worker_warmup_cb = nullptr,
    const std::chrono::milliseconds &flush_interval_ms = std::chrono::milliseconds::zero(),
    const std::function<void()> &worker_teardown_cb = nullptr);

template<class It>
shared_ptr<logger> create_async(const string &logger_name, const It &sinks_begin, const It &sinks_end, size_t queue_size,
    const async_overflow_policy overflow_policy = async_overflow_policy::block_retry,
    const std::function<void()> &worker_warmup_cb = nullptr,
    const std::chrono::milliseconds &flush_interval_ms = std::chrono::milliseconds::zero(),
    const std::function<void()> &worker_teardown_cb = nullptr);

// Register the given logger with the given name
void register_logger(shared_ptr<logger> logger);

// Apply a user defined function on all registered loggers
// Example:
// spdlog::apply_all([&](shared_ptr<spdlog::logger> l) {l->flush();});
void apply_all(std::function<void(shared_ptr<logger>)> fun);

// Drop the reference to the given logger
void drop(const string &name);

// Drop all references from the registry
void drop_all();

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

#include "details/spdlog_impl.h"
