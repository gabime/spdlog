//
// Copyright(c) 2015-present Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <memory>
#include <string>
#include "spdlog/fmt/fmt.h"

namespace spdlog {
class logger;

namespace lite {

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
    logger() = default;

    explicit logger(std::shared_ptr<spdlog::logger> impl);
    logger(const logger &) = default;
    logger(logger &&) = default;
    logger &operator=(const logger &) = default;

    ~logger() = default;

    bool should_log(spdlog::lite::level lvl) const noexcept;

    template<typename... Args>
    void log(spdlog::lite::level lvl, const char *fmt, const Args &... args)
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
    void log(spdlog::lite::level lvl, const string_view_t &sv);
    void log_printf(spdlog::lite::level lvl, const char* format, va_list args);

    //
    // trace
    //
    void trace(const char *msg)
    {
        log(spdlog::lite::level::trace, string_view_t(msg));
    }

    template<typename... Args>
    void trace(const char *fmt, const Args &... args)
    {
        log(spdlog::lite::level::trace, fmt, args...);
    }

    void trace_f(const char *printf_format, ...); 

    //
    // debug
    //
    void debug(const char *msg)
    {
        log(spdlog::lite::level::debug, string_view_t(msg));
    }

    template<typename... Args>
    void debug(const char *fmt, const Args &... args)
    {
        log(spdlog::lite::level::debug, fmt, args...);
    }

    void debug_f(const char *printf_format, ...); 

    //
    // info
    //
    void info(const char *msg)
    {
        log(spdlog::lite::level::info, string_view_t(msg));
    }

    template<typename... Args>
    void info(const char *fmt, const Args &... args)
    {
        log(spdlog::lite::level::info, fmt, args...);
    }

    void info_f(const char *printf_format, ...);

    //
    // warn
    //
    void warn(const char *msg)
    {
        log(spdlog::lite::level::warn, string_view_t(msg));
    }

    template<typename... Args>
    void warn(const char *fmt, const Args &... args)
    {
        log(spdlog::lite::level::warn, fmt, args...);
    }

    void warn_f(const char *printf_format, ...); 

    //
    // error
    //
    void error(const char *msg)
    {
        log(spdlog::lite::level::err, string_view_t(msg));
    }


    template<typename... Args>
    void error(const char *fmt, const Args &... args)
    {
        log(spdlog::lite::level::err, fmt, args...);
    }

    void error_f(const char *printf_format, ...); 

    //
    // critical
    //
    void critical(const char *msg)
    {
        log(spdlog::lite::level::critical, string_view_t(msg));
    }

    template<typename... Args>
    void critical(const char *fmt, const Args &... args)
    {
        log(spdlog::lite::level::critical, fmt, args...);
    }

    void critical_f(const char *printf_format, ...); 

    //
    // setters/getters
    //
    std::string name() const;
    void set_level(lite::level level);
    lite::level get_level() const;

    //
    // flush
    //
    void flush();
    void flush_on(spdlog::lite::level log_level);
    spdlog::lite::level flush_level() const;
    void set_pattern(std::string pattern);

protected:
    std::shared_ptr<spdlog::logger> impl_;
    void log_formatted_(spdlog::lite::level lvl, const fmt::memory_buffer &formatted);

};

spdlog::lite::logger &default_logger();

template<typename... Args>
void trace(const char *fmt, const Args &... args)
{
    default_logger().trace(fmt, args...);
}

template<typename... Args>
void debug(const char *fmt, const Args &... args)
{
    default_logger().debug(fmt, args...);
}

template<typename... Args>
void info(const char *fmt, const Args &... args)
{
    default_logger().info(fmt, args...);
}

template<typename... Args>
void warn(const char *fmt, const Args &... args)
{
    default_logger().warn(fmt, args...);
}

template<typename... Args>
void error(const char *fmt, const Args &... args)
{
    default_logger().error(fmt, args...);
}

template<typename... Args>
void critical(const char *fmt, const Args &... args)
{
    default_logger().critical(fmt, args...);
}

} // namespace lite

// factory to create lite logger
// implement it in a dedicated compilation unit for fast compiles
spdlog::lite::logger create_lite(void *ctx = nullptr);

} // namespace spdlog