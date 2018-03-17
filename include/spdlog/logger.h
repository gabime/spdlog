//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

// Thread safe logger (except for set_pattern(..), set_formatter(..) and set_error_handler())
// Has name, log level, vector of std::shared sink pointers and formatter
// Upon each log write the logger:
// 1. Checks if its log level is enough to log the message
// 2. Format the message using the formatter function
// 3. Pass the formatted message to its sinks to performa the actual logging

#include "common.h"
#include "sinks/base_sink.h"

#include <memory>
#include <string>
#include <vector>

namespace spdlog {

class logger
{
public:
    logger(const std::string &name, sink_ptr single_sink);
    logger(const std::string &name, sinks_init_list sinks);

    template<class It>
    logger(std::string name, const It &begin, const It &end);

    virtual ~logger();

    logger(const logger &) = delete;
    logger &operator=(const logger &) = delete;

    template<typename... Args>
    void log(level::level_enum lvl, const char *fmt, const Args &... args);

    template<typename... Args>
    void log(level::level_enum lvl, const char *msg);

    template<typename Arg1, typename... Args>
    void trace(const char *fmt, const Arg1 &, const Args &... args);

    template<typename Arg1, typename... Args>
    void debug(const char *fmt, const Arg1 &, const Args &... args);

    template<typename Arg1, typename... Args>
    void info(const char *fmt, const Arg1 &, const Args &... args);

    template<typename Arg1, typename... Args>
    void warn(const char *fmt, const Arg1 &, const Args &... args);

    template<typename Arg1, typename... Args>
    void error(const char *fmt, const Arg1 &, const Args &... args);

    template<typename Arg1, typename... Args>
    void critical(const char *fmt, const Arg1 &, const Args &... args);

#ifdef SPDLOG_WCHAR_TO_UTF8_SUPPORT
    template<typename... Args>
    void log(level::level_enum lvl, const wchar_t *msg);

    template<typename... Args>
    void log(level::level_enum lvl, const wchar_t *fmt, const Args &... args);

    template<typename... Args>
    void trace(const wchar_t *fmt, const Args &... args);

    template<typename... Args>
    void debug(const wchar_t *fmt, const Args &... args);

    template<typename... Args>
    void info(const wchar_t *fmt, const Args &... args);

    template<typename... Args>
    void warn(const wchar_t *fmt, const Args &... args);

    template<typename... Args>
    void error(const wchar_t *fmt, const Args &... args);

    template<typename... Args>
    void critical(const wchar_t *fmt, const Args &... args);
#endif // SPDLOG_WCHAR_TO_UTF8_SUPPORT

    template<typename T>
    void log(level::level_enum lvl, const T &);

    template<typename T>
    void trace(const T &msg);

    template<typename T>
    void debug(const T &msg);

    template<typename T>
    void info(const T &msg);

    template<typename T>
    void warn(const T &msg);

    template<typename T>
    void error(const T &msg);

    template<typename T>
    void critical(const T &msg);

    bool should_log(level::level_enum msg_level) const;
    void set_level(level::level_enum log_level);
    level::level_enum level() const;
    const std::string &name() const;
    void set_pattern(const std::string &pattern, pattern_time_type pattern_time = pattern_time_type::local);
    void set_formatter(formatter_ptr msg_formatter);

    // automatically call flush() if message level >= log_level
    void flush_on(level::level_enum log_level);

    virtual void flush();

    const std::vector<sink_ptr> &sinks() const;

    // error handler
    virtual void set_error_handler(log_err_handler err_handler);
    virtual log_err_handler error_handler();

protected:
    virtual void _sink_it(details::log_msg &msg);
    virtual void _set_pattern(const std::string &pattern, pattern_time_type pattern_time);
    virtual void _set_formatter(formatter_ptr msg_formatter);

    // default error handler: print the error to stderr with the max rate of 1 message/minute
    virtual void _default_err_handler(const std::string &msg);

    // return true if the given message level should trigger a flush
    bool _should_flush_on(const details::log_msg &msg);

    // increment the message count (only if defined(SPDLOG_ENABLE_MESSAGE_COUNTER))
    void _incr_msg_counter(details::log_msg &msg);

    const std::string _name;
    std::vector<sink_ptr> _sinks;
    formatter_ptr _formatter;
    spdlog::level_t _level;
    spdlog::level_t _flush_level;
    log_err_handler _err_handler;
    std::atomic<time_t> _last_err_time;
    std::atomic<size_t> _msg_counter;
};
} // namespace spdlog

#include "details/logger_impl.h"
