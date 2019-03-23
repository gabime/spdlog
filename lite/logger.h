//
// Copyright(c) 2015-present Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <memory>
#include <string>
#include "spdlog/fmt/fmt.h"

//
// enable/disable log calls at compile time according to global level.
//
// define SPDLITE_ACTIVE_LEVEL to one of those (before including lite.h):

#define SPDLITE_LEVEL_TRACE 0
#define SPDLITE_LEVEL_DEBUG 1
#define SPDLITE_LEVEL_INFO 2
#define SPDLITE_LEVEL_WARN 3
#define SPDLITE_LEVEL_ERROR 4
#define SPDLITE_LEVEL_CRITICAL 5
#define SPDLITE_LEVEL_OFF 6

#define SPDLITE_LOGGER_CALL(logger, level, ...)                                                                                            \
    if (logger.should_log(level))                                                                                                          \
    logger.log(level, __VA_ARGS__)

#if SPDLITE_ACTIVE_LEVEL <= SPDLITE_LEVEL_TRACE
#define SPDLITE_LOGGER_TRACE(logger, ...) SPDLITE_LOGGER_CALL(logger, spdlog::lite::level::trace, __VA_ARGS__)
#define SPDLITE_TRACE(...) SPDLITE_LOGGER_TRACE(spdlog::lite::default_logger(), __VA_ARGS__)
#else
#define SPDLITE_LOGGER_TRACE(logger, ...) (void)0
#define SPDLITE_TRACE(...) (void)0
#endif

#if SPDLITE_ACTIVE_LEVEL <= SPDLITE_LEVEL_DEBUG
#define SPDLITE_LOGGER_DEBUG(logger, ...) SPDLITE_LOGGER_CALL(logger, spdlog::lite::level::debug, __VA_ARGS__)
#define SPDLITE_DEBUG(...) SPDLITE_LOGGER_DEBUG(spdlog::lite::default_logger(), __VA_ARGS__)
#else
#define SPDLITE_LOGGER_DEBUG(logger, ...) (void)0
#define SPDLITE_DEBUG(...) (void)0
#endif

#if SPDLITE_ACTIVE_LEVEL <= SPDLITE_LEVEL_INFO
#define SPDLITE_LOGGER_INFO(logger, ...) SPDLITE_LOGGER_CALL(logger, spdlog::lite::level::info, __VA_ARGS__)
#define SPDLITE_INFO(...) SPDLITE_LOGGER_INFO(spdlog::lite::default_logger(), __VA_ARGS__)
#else
#define SPDLITE_LOGGER_INFO(logger, ...) (void)0
#define SPDLITE_INFO(...) (void)0
#endif

#if SPDLITE_ACTIVE_LEVEL <= SPDLITE_LEVEL_WARN
#define SPDLITE_LOGGER_WARN(logger, ...) SPDLITE_LOGGER_CALL(logger, spdlog::lite::level::warn, __VA_ARGS__)
#define SPDLITE_WARN(...) SPDLITE_LOGGER_WARN(spdlog::lite::default_logger(), __VA_ARGS__)
#else
#define SPDLITE_LOGGER_WARN(logger, ...) (void)0
#define SPDLITE_WARN(...) (void)0
#endif

#if SPDLITE_ACTIVE_LEVEL <= SPDLITE_LEVEL_ERROR
#define SPDLITE_LOGGER_ERROR(logger, ...) SPDLITE_LOGGER_CALL(logger, spdlog::lite::level::err, __VA_ARGS__)
#define SPDLITE_ERROR(...) SPDLITE_LOGGER_ERROR(spdlog::lite::default_logger(), __VA_ARGS__)
#else
#define SPDLITE_LOGGER_ERROR(logger, ...) (void)0
#define SPDLITE_ERROR(...) (void)0
#endif

#if SPDLITE_ACTIVE_LEVEL <= SPDLITE_LEVEL_CRITICAL
#define SPDLITE_LOGGER_CRITICAL(logger, ...) SPDLITE_LOGGER_CALL(logger, spdlog::lite::level::critical, __VA_ARGS__)
#define SPDLITE_CRITICAL(...) SPDLITE_LOGGER_CRITICAL(spdlog::lite::default_logger(), __VA_ARGS__)
#else
#define SPDLITE_LOGGER_CRITICAL(logger, ...) (void)0
#define SPDLITE_CRITICAL(...) (void)0
#endif

namespace spdlog {
class logger;

namespace lite {
enum class level
{
    trace = SPDLITE_LEVEL_TRACE,
    debug = SPDLITE_LEVEL_DEBUG,
    info = SPDLITE_LEVEL_INFO,
    warn = SPDLITE_LEVEL_WARN,
    err = SPDLITE_LEVEL_ERROR,
    critical = SPDLITE_LEVEL_CRITICAL,
    off = SPDLITE_LEVEL_OFF
};

struct src_loc
{
    const char *filename;
    int line;
    const char *funcname;
};

class logger
{
public:
    logger() = default;

    logger(std::shared_ptr<spdlog::logger> impl);
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

    template<typename... Args>
    void log(const spdlog::lite::src_loc &src, spdlog::lite::level lvl, const char *fmt, const Args &... args)
    {
        if (!should_log(lvl))
        {
            return;
        }
        fmt::memory_buffer formatted_buf;
        fmt::format_to(formatted_buf, fmt, args...);
        log_formatted_src(src, lvl, formatted_buf);
    }

    template<typename... Args>
    void trace(const char *fmt, const Args &... args)
    {
        log(spdlog::lite::level::trace, fmt, args...);
    }

    template<typename... Args>
    void trace(const char *source_file, int source_line, const char *source_func, const char *fmt, const Args &... args)
    {
        spdlog::lite::src_loc src{source_file, source_line, source_func};
        log(src, spdlog::lite::level::trace, fmt, args...);
    }

    template<typename... Args>
    void debug(const char *fmt, const Args &... args)
    {
        log(spdlog::lite::level::debug, fmt, args...);
    }

    template<typename... Args>
    void info(const char *fmt, const Args &... args)
    {
        log(spdlog::lite::level::info, fmt, args...);
    }

    template<typename... Args>
    void warn(const char *fmt, const Args &... args)
    {
        log(spdlog::lite::level::warn, fmt, args...);
    }

    template<typename... Args>
    void error(const char *fmt, const Args &... args)
    {
        log(spdlog::lite::level::err, fmt, args...);
    }

    template<typename... Args>
    void critical(const char *fmt, const Args &... args)
    {
        log(spdlog::lite::level::critical, fmt, args...);
    }

    std::string name() const;

    // level
    void set_level(lite::level level);
    lite::level get_level() const;

    // flush
    void flush();
    void flush_on(spdlog::lite::level log_level);
    spdlog::lite::level flush_level() const;

    // pattern
    void set_pattern(std::string pattern);

protected:
    std::shared_ptr<spdlog::logger> impl_;
    void log_formatted_(spdlog::lite::level lvl, const fmt::memory_buffer &formatted);
    void log_formatted_src(const spdlog::lite::src_loc &src, spdlog::lite::level lvl, const fmt::memory_buffer &formatted);
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