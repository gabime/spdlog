//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

//
// Global registry functions
//
#include <spdlog/spdlog.h>
#include <spdlog/details/registry.h>
#include <spdlog/sinks/file_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/sinks/ansicolor_sink.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>

inline void spdlog::register_logger(std::shared_ptr<logger> logger)
{
    return details::registry::instance().register_logger(logger);
}

inline std::shared_ptr<spdlog::logger> spdlog::get(const std::string& name)
{
    return details::registry::instance().get(name);
}

inline void spdlog::drop(const std::string &name)
{
    details::registry::instance().drop(name);
}

// Create multi/single threaded simple file logger
inline std::shared_ptr<spdlog::logger> spdlog::basic_logger_mt(const std::string& logger_name, const filename_t& filename, bool force_flush)
{
    return create<spdlog::sinks::simple_file_sink_mt>(logger_name, filename, force_flush);
}

inline std::shared_ptr<spdlog::logger> spdlog::basic_logger_st(const std::string& logger_name, const filename_t& filename, bool force_flush)
{
    return create<spdlog::sinks::simple_file_sink_st>(logger_name, filename, force_flush);
}

// Create multi/single threaded rotating file logger
inline std::shared_ptr<spdlog::logger> spdlog::rotating_logger_mt(const std::string& logger_name, const filename_t& filename, size_t max_file_size, size_t max_files, bool force_flush)
{
    return create<spdlog::sinks::rotating_file_sink_mt>(logger_name, filename, SPDLOG_FILENAME_T("txt"), max_file_size, max_files, force_flush);
}

inline std::shared_ptr<spdlog::logger> spdlog::rotating_logger_st(const std::string& logger_name, const filename_t& filename, size_t max_file_size, size_t max_files, bool force_flush)
{
    return create<spdlog::sinks::rotating_file_sink_st>(logger_name, filename, SPDLOG_FILENAME_T("txt"), max_file_size, max_files, force_flush);
}

// Create file logger which creates new file at midnight):
inline std::shared_ptr<spdlog::logger> spdlog::daily_logger_mt(const std::string& logger_name, const filename_t& filename, int hour, int minute, bool force_flush)
{
    return create<spdlog::sinks::daily_file_sink_mt>(logger_name, filename, SPDLOG_FILENAME_T("txt"), hour, minute, force_flush);
}

inline std::shared_ptr<spdlog::logger> spdlog::daily_logger_st(const std::string& logger_name, const filename_t& filename, int hour, int minute, bool force_flush)
{
    return create<spdlog::sinks::daily_file_sink_st>(logger_name, filename, SPDLOG_FILENAME_T("txt"), hour, minute, force_flush);
}

// Create stdout/stderr loggers (with optinal color support)
inline std::shared_ptr<spdlog::logger> create_console_logger(const std::string& logger_name, spdlog::sink_ptr sink, bool color)
{
    if (color) //use color wrapper sink
        sink = std::make_shared<spdlog::sinks::ansicolor_sink>(sink);
    return spdlog::details::registry::instance().create(logger_name, sink);
}

inline std::shared_ptr<spdlog::logger> spdlog::stdout_logger_mt(const std::string& logger_name, bool color)
{
    return create_console_logger(logger_name, sinks::stdout_sink_mt::instance(), color);
}

inline std::shared_ptr<spdlog::logger> spdlog::stdout_logger_st(const std::string& logger_name, bool color)
{
    return create_console_logger(logger_name, sinks::stdout_sink_st::instance(), color);
}

inline std::shared_ptr<spdlog::logger> spdlog::stderr_logger_mt(const std::string& logger_name, bool color)
{
    return create_console_logger(logger_name, sinks::stderr_sink_mt::instance(), color);
}

inline std::shared_ptr<spdlog::logger> spdlog::stderr_logger_st(const std::string& logger_name, bool color)
{
    return create_console_logger(logger_name, sinks::stderr_sink_st::instance(), color);
}

#if defined(__linux__) || defined(__APPLE__)
// Create syslog logger
inline std::shared_ptr<spdlog::logger> spdlog::syslog_logger(const std::string& logger_name, const std::string& syslog_ident, int syslog_option)
{
    return create<spdlog::sinks::syslog_sink>(logger_name, syslog_ident, syslog_option);
}
#endif

// Create and register a logger a single sink
inline std::shared_ptr<spdlog::logger> spdlog::create(const std::string& logger_name, const spdlog::sink_ptr& sink)
{
    return details::registry::instance().create(logger_name, sink);
}

//Create logger with multiple sinks

inline std::shared_ptr<spdlog::logger> spdlog::create(const std::string& logger_name, spdlog::sinks_init_list sinks)
{
    return details::registry::instance().create(logger_name, sinks);
}


template <typename Sink, typename... Args>
inline std::shared_ptr<spdlog::logger> spdlog::create(const std::string& logger_name, Args... args)
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


inline void spdlog::set_async_mode(size_t queue_size, const async_overflow_policy overflow_policy, const std::function<void()>& worker_warmup_cb, const std::chrono::milliseconds& flush_interval_ms, const std::function<void()>& worker_teardown_cb)
{
    details::registry::instance().set_async_mode(queue_size, overflow_policy, worker_warmup_cb, flush_interval_ms, worker_teardown_cb);
}

inline void spdlog::set_sync_mode()
{
    details::registry::instance().set_sync_mode();
}

inline void spdlog::drop_all()
{
    details::registry::instance().drop_all();
}
