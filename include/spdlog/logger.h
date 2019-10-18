// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

// Thread safe logger (except for set_error_handler())
// Has name, log level, vector of std::shared sink pointers and formatter
// Upon each log write the logger:
// 1. Checks if its log level is enough to log the message and if yes:
// 2. Call the underlying sinks to do the job.
// 3. Each sink use its own private copy of a formatter to format the message
// and send to its destination.
//
// The use of private formatter per sink provides the opportunity to cache some
// formatted data, and support for different format per sink.

#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/details/backtracer.h"

#ifdef SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include "spdlog/details/os.h"
#endif

#include <vector>
#ifndef SPDLOG_NO_EXCEPTIONS
#define SPDLOG_LOGGER_CATCH()                                                                                                              \
    catch (const std::exception &ex)                                                                                                       \
    {                                                                                                                                      \
        err_handler_(ex.what());                                                                                                           \
    }                                                                                                                                      \
    catch (...)                                                                                                                            \
    {                                                                                                                                      \
        err_handler_("Unknown exception in logger");                                                                                       \
    }
#else
#define SPDLOG_LOGGER_CATCH()
#endif

namespace spdlog {

class logger
{
public:
    // Empty logger
    explicit logger(std::string name)
        : name_(std::move(name))
        , sinks_()
    {}

    // Logger with range on sinks
    template<typename It>
    logger(std::string name, It begin, It end)
        : name_(std::move(name))
        , sinks_(begin, end)
    {}

    // Logger with single sink
    logger(std::string name, sink_ptr single_sink)
        : logger(std::move(name), {std::move(single_sink)})
    {}

    // Logger with sinks init list
    logger(std::string name, sinks_init_list sinks)
        : logger(std::move(name), sinks.begin(), sinks.end())
    {}

    virtual ~logger() = default;

    logger(const logger &other);
    logger(logger &&other) SPDLOG_NOEXCEPT;
    logger &operator=(logger other) SPDLOG_NOEXCEPT;

    void swap(spdlog::logger &other) SPDLOG_NOEXCEPT;

    template<typename... Args>
    void log(source_loc loc, level::level_enum lvl, string_view_t fmt, const Args &... args)
    {
        if (!should_log(lvl) && !tracer_.enabled())
        {
            return;
        }
        SPDLOG_TRY
        {
            memory_buf_t buf;
            fmt::format_to(buf, fmt, args...);
            details::log_msg log_msg(loc, name_, lvl, string_view_t(buf.data(), buf.size()));
            log_it_(log_msg);
        }
        SPDLOG_LOGGER_CATCH()
    }

    template<typename... Args>
    void log(level::level_enum lvl, string_view_t fmt, const Args &... args)
    {
        log(source_loc{}, lvl, fmt, args...);
    }

    template<typename... Args>
    void trace(string_view_t fmt, const Args &... args)
    {
        log(level::trace, fmt, args...);
    }

    template<typename... Args>
    void debug(string_view_t fmt, const Args &... args)
    {
        log(level::debug, fmt, args...);
    }

    template<typename... Args>
    void info(string_view_t fmt, const Args &... args)
    {
        log(level::info, fmt, args...);
    }

    template<typename... Args>
    void warn(string_view_t fmt, const Args &... args)
    {
        log(level::warn, fmt, args...);
    }

    template<typename... Args>
    void error(string_view_t fmt, const Args &... args)
    {
        log(level::err, fmt, args...);
    }

    template<typename... Args>
    void critical(string_view_t fmt, const Args &... args)
    {
        log(level::critical, fmt, args...);
    }

    template<typename T>
    void log(level::level_enum lvl, const T &msg)
    {
        log(source_loc{}, lvl, msg);
    }

    // T can be statically converted to string_view
    template<class T, typename std::enable_if<std::is_convertible<const T &, spdlog::string_view_t>::value, T>::type * = nullptr>
    void log(source_loc loc, level::level_enum lvl, const T &msg)
    {
        if (!should_log(lvl) && !tracer_.enabled())
        {
            return;
        }

        details::log_msg log_msg(loc, name_, lvl, msg);
        log_it_(log_msg);
    }

    void log(level::level_enum lvl, string_view_t msg)
    {
        log(source_loc{}, lvl, msg);
    }

    // T cannot be statically converted to string_view or wstring_view
    template<class T, typename std::enable_if<!std::is_convertible<const T &, spdlog::string_view_t>::value &&
                                                  !is_convertible_to_wstring_view<const T &>::value,
                          T>::type * = nullptr>
    void log(source_loc loc, level::level_enum lvl, const T &msg)
    {
        log(loc, lvl, "{}", msg);
    }

    template<typename T>
    void trace(const T &msg)
    {
        log(level::trace, msg);
    }

    template<typename T>
    void debug(const T &msg)
    {
        log(level::debug, msg);
    }

    template<typename T>
    void info(const T &msg)
    {
        log(level::info, msg);
    }

    template<typename T>
    void warn(const T &msg)
    {
        log(level::warn, msg);
    }

    template<typename T>
    void error(const T &msg)
    {
        log(level::err, msg);
    }

    template<typename T>
    void critical(const T &msg)
    {
        log(level::critical, msg);
    }

#ifdef SPDLOG_WCHAR_TO_UTF8_SUPPORT
#ifndef _WIN32
#error SPDLOG_WCHAR_TO_UTF8_SUPPORT only supported on windows
#else

    template<typename... Args>
    void log(source_loc loc, level::level_enum lvl, wstring_view_t fmt, const Args &... args)
    {
        if (!should_log(lvl) && !tracer_.enabled())
        {
            return;
        }
        SPDLOG_TRY
        {
            // format to wmemory_buffer and convert to utf8
            fmt::wmemory_buffer wbuf;
            fmt::format_to(wbuf, fmt, args...);

            memory_buf_t buf;
            details::os::wstr_to_utf8buf(wstring_view_t(wbuf.data(), wbuf.size()), buf);
            details::log_msg log_msg(loc, name_, lvl, string_view_t(buf.data(), buf.size()));
            log_it_(log_msg);
        }
        SPDLOG_LOGGER_CATCH()
    }

    template<typename... Args>
    void log(level::level_enum lvl, wstring_view_t fmt, const Args &... args)
    {
        log(source_loc{}, lvl, fmt, args...);
    }

    template<typename... Args>
    void trace(wstring_view_t fmt, const Args &... args)
    {
        log(level::trace, fmt, args...);
    }

    template<typename... Args>
    void debug(wstring_view_t fmt, const Args &... args)
    {
        log(level::debug, fmt, args...);
    }

    template<typename... Args>
    void info(wstring_view_t fmt, const Args &... args)
    {
        log(level::info, fmt, args...);
    }

    template<typename... Args>
    void warn(wstring_view_t fmt, const Args &... args)
    {
        log(level::warn, fmt, args...);
    }

    template<typename... Args>
    void error(wstring_view_t fmt, const Args &... args)
    {
        log(level::err, fmt, args...);
    }

    template<typename... Args>
    void critical(wstring_view_t fmt, const Args &... args)
    {
        log(level::critical, fmt, args...);
    }

    // T can be statically converted to wstring_view
    template<class T, typename std::enable_if<is_convertible_to_wstring_view<const T &>::value, T>::type * = nullptr>
    void log(source_loc loc, level::level_enum lvl, const T &msg)
    {
        if (!should_log(lvl) && !tracer_.enabled())
        {
            return;
        }

        try
        {
            memory_buf_t buf;
            details::os::wstr_to_utf8buf(msg, buf);
            details::log_msg log_msg(loc, name_, lvl, string_view_t(buf.data(), buf.size()));
            log_it_(log_msg);
        }
        SPDLOG_LOGGER_CATCH()
    }
#endif // _WIN32
#endif // SPDLOG_WCHAR_TO_UTF8_SUPPORT

    // return true logging is enabled for the given level.
    bool should_log(level::level_enum msg_level) const;

    // return true if backtrace logging is enabled.
    bool should_backtrace() const;

    void set_level(level::level_enum log_level);

    level::level_enum level() const;

    const std::string &name() const;

    // set formatting for the sinks in this logger.
    // each sink will get a seperate instance of the formatter object.
    void set_formatter(std::unique_ptr<formatter> f);

    void set_pattern(std::string pattern, pattern_time_type time_type = pattern_time_type::local);

    // backtrace support.
    // efficiently store all debug/trace messages in a circular buffer until needed for debugging.
    void enable_backtrace(size_t n_messages);
    void disable_backtrace();
    void dump_backtrace();

    // flush functions
    void flush();
    void flush_on(level::level_enum log_level);
    level::level_enum flush_level() const;

    // sinks
    const std::vector<sink_ptr> &sinks() const;

    std::vector<sink_ptr> &sinks();

    // error handler
    void set_error_handler(err_handler);

    // create new logger with same sinks and configuration.
    virtual std::shared_ptr<logger> clone(std::string logger_name);

protected:
    std::string name_;
    std::vector<sink_ptr> sinks_;
    spdlog::level_t level_{level::info};
    spdlog::level_t flush_level_{level::off};
    err_handler custom_err_handler_{nullptr};
    details::backtracer tracer_;

    // log the given message (if the given log level is high enough),
    // and save backtrace (if backtrace is enabled).
    void log_it_(const details::log_msg &log_msg);
    virtual void sink_it_(const details::log_msg &msg);
    virtual void flush_();
    void dump_backtrace_();
    bool should_flush_(const details::log_msg &msg);

    // handle errors during logging.
    // default handler prints the error to stderr at max rate of 1 message/sec.
    void err_handler_(const std::string &msg);
};

void swap(logger &a, logger &b);

} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "logger-inl.h"
#endif
