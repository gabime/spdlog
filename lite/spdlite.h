//
// Copyright(c) 2015-present Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

// lite logger - a pimpl around spdlog::logger shared_ptr:
//  much faster compile times.
//  can be used as lib or separate compilation unit.
//  very cheap copy and move.
//  supports printf format for even faster compile (by avoiding variadic templates).
//
// see lite-example/ for usage.

#include <memory>
#include <string>
#include "spdlog/fmt/fmt.h"

namespace spdlog {
class logger;
}
namespace spdlite {

// string_view type - either std::string_view or fmt::string_view (pre c++17)
#if defined(FMT_USE_STD_STRING_VIEW)
using string_view_t = std::string_view;
#else
using string_view_t = fmt::string_view;
#endif

enum class level
{
    trace,
    debug,
    info,
    warn,
    err,
    critical,
    off
};

class logger
{
public:
    explicit logger(std::shared_ptr<spdlog::logger> impl);
    // logger() = default; //logger with nullptr impl
    logger(const logger &) = default;
    logger(logger &&) = default;
    logger &operator=(const logger &) = default;

    ~logger() = default;

    void set_impl(std::shared_ptr<spdlog::logger> impl);
    bool should_log(spdlite::level lvl) const noexcept;

    template<typename... Args>
    void log(spdlite::level lvl, const char *fmt, const Args &... args)
    {
        if (!should_log(lvl))
        {
            return;
        }
        fmt::memory_buffer formatted_buf;
        fmt::format_to(formatted_buf, fmt, args...);
        log_formatted_(lvl, formatted_buf);
    }

    // log string view
    void log(spdlite::level lvl, const string_view_t &sv);
    
    //
    // trace
    //

    void trace(const char *msg)
    {
        log(spdlite::level::trace, string_view_t(msg));
    }

    template<typename T>
    void trace(const T &msg)
    {
        log(spdlite::level::trace, string_view_t(msg));
    }

    template<typename... Args>
    void trace(const char *fmt, const Args &... args)
    {
        log(spdlite::level::trace, fmt, args...);
    }
   
    //
    // debug
    //
    void debug(const char *msg)
    {
        log(spdlite::level::debug, string_view_t(msg));
    }

    template<typename T>
    void debug(const T &msg)
    {
        log(spdlite::level::debug, string_view_t(msg));
    }

    template<typename... Args>
    void debug(const char *fmt, const Args &... args)
    {
        log(spdlite::level::debug, fmt, args...);
    }
   
    // info
    void info(const char *msg)
    {
        log(spdlite::level::info, string_view_t(msg));
    }

    template<typename T>
    void info(const T &msg)
    {
        log(spdlite::level::info, string_view_t(msg));
    }

    template<typename... Args>
    void info(const char *fmt, const Args &... args)
    {
        log(spdlite::level::info, fmt, args...);
    }

    // warn
    void warn(const char *msg)
    {
        log(spdlite::level::warn, string_view_t(msg));
    }

    template<typename T>
    void warn(const T &msg)
    {
        log(spdlite::level::warn, string_view_t(msg));
    }

    template<typename... Args>
    void warn(const char *fmt, const Args &... args)
    {
        log(spdlite::level::warn, fmt, args...);
    }
  
    // error
    void error(const char *msg)
    {
        log(spdlite::level::err, string_view_t(msg));
    }

    template<typename T>
    void error(const T &msg)
    {
        log(spdlite::level::err, string_view_t(msg));
    }

    template<typename... Args>
    void error(const char *fmt, const Args &... args)
    {
        log(spdlite::level::err, fmt, args...);
    }

    // critical
    void critical(const char *msg)
    {
        log(spdlite::level::critical, string_view_t(msg));
    }

    template<typename T>
    void critical(const T &msg)
    {
        log(spdlite::level::critical, string_view_t(msg));
    }

    template<typename... Args>
    void critical(const char *fmt, const Args &... args)
    {
        log(spdlite::level::critical, fmt, args...);
    }

	// printf formatting    
    void log_printf(spdlite::level lvl, const char *format, va_list args);
    void trace_printf(const char *format, ...);
    void debug_printf(const char *format, ...);
    void info_printf(const char *format, ...);
    void warn_printf(const char *format, ...);
    void error_printf(const char *format, ...);
    void critical_printf(const char *format, ...);
    
    // setters/getters
    void set_level(spdlite::level level) noexcept;
    void set_pattern(std::string pattern) noexcept;
    spdlite::level level() const noexcept;
    std::string name() const noexcept;
    spdlite::level flush_level() const noexcept;

    // flush
    void flush();
    void flush_on(spdlite::level log_level);

    // clone with new name
    spdlite::logger clone(std::string logger_name);

    static spdlite::logger &default_logger();

protected:
    std::shared_ptr<spdlog::logger> impl_;
    void log_formatted_(spdlite::level lvl, const fmt::memory_buffer &formatted);
};

//
// spdlite namespace functions - forward the calls to the default_logger.
//
spdlite::logger &default_logger();

template<typename... Args>
inline void trace(const char *fmt, const Args &... args)
{
    default_logger().trace(fmt, args...);
}

template<typename... Args>
inline void debug(const char *fmt, const Args &... args)
{
    default_logger().debug(fmt, args...);
}


template<typename... Args>
inline void info(const char *fmt, const Args &... args)
{
    default_logger().info(fmt, args...);
}

template<typename... Args>
inline void warn(const char *fmt, const Args &... args)
{
    default_logger().warn(fmt, args...);
}

template<typename... Args>
inline void error(const char *fmt, const Args &... args)
{
    default_logger().error(fmt, args...);
}

template<typename... Args>
inline void critical(const char *fmt, const Args &... args)
{
    default_logger().critical(fmt, args...);
}

void log_printf(spdlite::level lvl, const char *format, va_list args);
void trace_printf(const char *format, ...);
void debug_printf(const char *format, ...);
void info_printf(const char *format, ...);
void warn_printf(const char *format, ...);
void error_printf(const char *format, ...);
void critical_printf(const char *format, ...);

} // namespace spdlite

// user implemented factory to create lite logger
// implement it in a seperated and dedicated compilation unit for fast compiles.
spdlite::logger create_logger(void *ctx = nullptr);
