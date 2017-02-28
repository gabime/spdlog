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
#ifdef SPDLOG_ENABLE_SYSLOG
#include <spdlog/sinks/syslog_sink.h>
#endif

#ifdef _WIN32
#include <spdlog/sinks/wincolor_sink.h>
#else
#include <spdlog/sinks/ansicolor_sink.h>
#endif


#ifdef __ANDROID__
#include <spdlog/sinks/android_sink.h>
#endif

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
inline std::shared_ptr<spdlog::logger> spdlog::basic_logger_mt(const std::string& logger_name, const filename_t& filename, bool truncate)
{
    return create<spdlog::sinks::simple_file_sink_mt>(logger_name, filename, truncate);
}

inline std::shared_ptr<spdlog::logger> spdlog::basic_logger_st(const std::string& logger_name, const filename_t& filename, bool truncate)
{
    return create<spdlog::sinks::simple_file_sink_st>(logger_name, filename, truncate);
}

// Create multi/single threaded rotating file logger
inline std::shared_ptr<spdlog::logger> spdlog::rotating_logger_mt(const std::string& logger_name, const filename_t& filename, size_t max_file_size, size_t max_files)
{
    return create<spdlog::sinks::rotating_file_sink_mt>(logger_name, filename, max_file_size, max_files);
}

inline std::shared_ptr<spdlog::logger> spdlog::rotating_logger_st(const std::string& logger_name, const filename_t& filename, size_t max_file_size, size_t max_files)
{
    return create<spdlog::sinks::rotating_file_sink_st>(logger_name, filename, max_file_size, max_files);
}

// Create file logger which creates new file at midnight):
inline std::shared_ptr<spdlog::logger> spdlog::daily_logger_mt(const std::string& logger_name, const filename_t& filename, int hour, int minute)
{
    return create<spdlog::sinks::daily_file_sink_mt>(logger_name, filename, hour, minute);
}

inline std::shared_ptr<spdlog::logger> spdlog::daily_logger_st(const std::string& logger_name, const filename_t& filename, int hour, int minute)
{
    return create<spdlog::sinks::daily_file_sink_st>(logger_name, filename, hour, minute);
}


//
// stdout/stderr loggers
//
inline std::shared_ptr<spdlog::logger> spdlog::stdout_logger_mt(const std::string& logger_name)
{
    return spdlog::details::registry::instance().create(logger_name, spdlog::sinks::stdout_sink_mt::instance());
}

inline std::shared_ptr<spdlog::logger> spdlog::stdout_logger_st(const std::string& logger_name)
{
    return spdlog::details::registry::instance().create(logger_name, spdlog::sinks::stdout_sink_st::instance());
}

inline std::shared_ptr<spdlog::logger> spdlog::stderr_logger_mt(const std::string& logger_name)
{
    return spdlog::details::registry::instance().create(logger_name, spdlog::sinks::stderr_sink_mt::instance());
}

inline std::shared_ptr<spdlog::logger> spdlog::stderr_logger_st(const std::string& logger_name)
{
    return spdlog::details::registry::instance().create(logger_name, spdlog::sinks::stderr_sink_st::instance());
}

//
// stdout/stderr color loggers
//
#ifdef _WIN32
inline std::shared_ptr<spdlog::logger> spdlog::stdout_color_mt(const std::string& logger_name)
{
    auto sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
    return spdlog::details::registry::instance().create(logger_name, sink);
}

inline std::shared_ptr<spdlog::logger> spdlog::stdout_color_st(const std::string& logger_name)
{
    auto sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>();
    return spdlog::details::registry::instance().create(logger_name, sink);
}

inline std::shared_ptr<spdlog::logger> spdlog::stderr_color_mt(const std::string& logger_name)
{
    auto sink = std::make_shared<spdlog::sinks::wincolor_stderr_sink_mt>();
    return spdlog::details::registry::instance().create(logger_name, sink);
}


inline std::shared_ptr<spdlog::logger> spdlog::stderr_color_st(const std::string& logger_name)
{
    auto sink = std::make_shared<spdlog::sinks::wincolor_stderr_sink_st>();
    return spdlog::details::registry::instance().create(logger_name, sink);
}

#else //ansi terminal colors

inline std::shared_ptr<spdlog::logger> spdlog::stdout_color_mt(const std::string& logger_name)
{
    auto sink = std::make_shared<spdlog::sinks::ansicolor_sink>(spdlog::sinks::stdout_sink_mt::instance());
    return spdlog::details::registry::instance().create(logger_name, sink);
}

inline std::shared_ptr<spdlog::logger> spdlog::stdout_color_st(const std::string& logger_name)
{
    auto sink = std::make_shared<spdlog::sinks::ansicolor_sink>(spdlog::sinks::stdout_sink_st::instance());
    return spdlog::details::registry::instance().create(logger_name, sink);
}

inline std::shared_ptr<spdlog::logger> spdlog::stderr_color_mt(const std::string& logger_name)
{
    auto sink = std::make_shared<spdlog::sinks::ansicolor_sink>(spdlog::sinks::stderr_sink_mt::instance());
    return spdlog::details::registry::instance().create(logger_name, sink);
}

inline std::shared_ptr<spdlog::logger> spdlog::stderr_color_st(const std::string& logger_name)
{
    auto sink = std::make_shared<spdlog::sinks::ansicolor_sink>(spdlog::sinks::stderr_sink_st::instance());
    return spdlog::details::registry::instance().create(logger_name, sink);
}
#endif

#ifdef SPDLOG_ENABLE_SYSLOG
// Create syslog logger
inline std::shared_ptr<spdlog::logger> spdlog::syslog_logger(const std::string& logger_name, const std::string& syslog_ident, int syslog_option)
{
    return create<spdlog::sinks::syslog_sink>(logger_name, syslog_ident, syslog_option);
}
#endif

#ifdef __ANDROID__
inline std::shared_ptr<spdlog::logger> spdlog::android_logger(const std::string& logger_name, const std::string& tag)
{
    return create<spdlog::sinks::android_sink>(logger_name, tag);
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

inline void spdlog::set_error_handler(log_err_handler handler)
{
    return details::registry::instance().set_error_handler(handler);
}


inline void spdlog::set_async_mode(size_t queue_size, const async_overflow_policy overflow_policy, const std::function<void()>& worker_warmup_cb, const std::chrono::milliseconds& flush_interval_ms, const std::function<void()>& worker_teardown_cb)
{
    details::registry::instance().set_async_mode(queue_size, overflow_policy, worker_warmup_cb, flush_interval_ms, worker_teardown_cb);
}

inline void spdlog::set_sync_mode()
{
    details::registry::instance().set_sync_mode();
}

inline void spdlog::apply_all(std::function<void(std::shared_ptr<logger>)> fun)
{
    details::registry::instance().apply_all(fun);
}

inline void spdlog::drop_all()
{
    details::registry::instance().drop_all();
}
