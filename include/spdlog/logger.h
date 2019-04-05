//
// Copyright(c) 2015-2108 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

// Thread safe logger (except for set_pattern(..), set_formatter(..) and
// set_error_handler())
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

//#include "spdlog/formatter.h"
//#include "spdlog/sinks/sink.h"

#include <memory>
#include <string>
#include <vector>

namespace spdlog
{
    class logger
    {
    public:
        template<typename It>
        logger(std::string name, It begin, It end)
            : name_(std::move(name))
            , sinks_(begin, end)
        {
        }

        logger(std::string name, sink_ptr single_sink)
            : logger(std::move(name), {std::move(single_sink)})
        {
        }
        logger(std::string name, sinks_init_list sinks)
            : logger(std::move(name), sinks.begin(), sinks.end())
        {
        }

        virtual ~logger() = default;

        logger(const logger &) = delete;
        logger &operator=(const logger &) = delete;

        template<typename... Args>
        void log(source_loc loc, level::level_enum lvl, const char *fmt, const Args &... args)
        {
            if (!should_log(lvl))
            {
                return;
            }

            try
            {                
                fmt::memory_buffer buf;
                fmt::format_to(buf, fmt, args...);
                details::log_msg log_msg(loc, &name_, lvl, string_view_t(buf.data(), buf.size()));
                sink_it_(log_msg);
            }
            catch (const std::exception &ex)
            {
                err_handler_(ex.what());
            }
            catch (...)
            {
                err_handler_("Unknown exception in logger");
            }
        }

        template<typename... Args>
        void log(level::level_enum lvl, const char *fmt, const Args &... args)
        {
            log(source_loc{}, lvl, fmt, args...);
        }
        void log(source_loc loc, level::level_enum lvl, const char *msg);
        void log(level::level_enum lvl, const char *msg);

        template<typename... Args>
        void trace(const char *fmt, const Args &... args)
        {
            log(level::trace, fmt, args...);
        }

        template<typename... Args>
        void debug(const char *fmt, const Args &... args)
        {
            log(level::debug, fmt, args...);
        }

        template<typename... Args>
        void info(const char *fmt, const Args &... args)
        {
            log(level::info, fmt, args...);
        }

        template<typename... Args>
        void warn(const char *fmt, const Args &... args)
        {
            log(level::warn, fmt, args...);
        }

        template<typename... Args>
        void error(const char *fmt, const Args &... args)
        {
            log(level::err, fmt, args...);
        }

        template<typename... Args>
        void critical(const char *fmt, const Args &... args)
        {
            log(level::critical, fmt, args...);
        }

#ifdef SPDLOG_WCHAR_TO_UTF8_SUPPORT
#ifndef _WIN32
#error SPDLOG_WCHAR_TO_UTF8_SUPPORT only supported on windows
#else
        inline void wbuf_to_utf8buf(const fmt::wmemory_buffer &wbuf, fmt::memory_buffer &target)
        {
            int wbuf_size = static_cast<int>(wbuf.size());
            if (wbuf_size == 0)
            {
                return;
            }

            auto result_size = ::WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), wbuf_size, NULL, 0, NULL, NULL);

            if (result_size > 0)
            {
                target.resize(result_size);
                ::WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), wbuf_size, &target.data()[0], result_size, NULL, NULL);
            }
            else
            {
                throw spdlog::spdlog_ex(fmt::format("WideCharToMultiByte failed. Last error: {}", ::GetLastError()));
            }
        }

        template<typename... Args>
        void log(source_loc source, level::level_enum lvl, const wchar_t *fmt, const Args &... args)
        {
            if (!should_log(lvl))
            {
                return;
            }

            try
            {
                // format to wmemory_buffer and convert to utf8
                using details::fmt_helper::to_string_view;
                fmt::wmemory_buffer wbuf;
                fmt::format_to(wbuf, fmt, args...);
                fmt::memory_buffer buf;
                wbuf_to_utf8buf(wbuf, buf);
                details::log_msg log_msg(source, &name_, lvl, to_string_view(buf));
                sink_it_(log_msg);
            }
            SPDLOG_CATCH_AND_HANDLE
        }

        template<typename... Args>
        void log(level::level_enum lvl, const wchar_t *fmt, const Args &... args)
        {
            log(source_loc{}, lvl, fmt, args...);
        }

        template<typename... Args>
        void trace(const wchar_t *fmt, const Args &... args)
        {
            log(level::trace, fmt, args...);
        }

        template<typename... Args>
        void debug(const wchar_t *fmt, const Args &... args)
        {
            log(level::debug, fmt, args...);
        }

        template<typename... Args>
        void info(const wchar_t *fmt, const Args &... args)
        {
            log(level::info, fmt, args...);
        }

        template<typename... Args>
        void warn(const wchar_t *fmt, const Args &... args)
        {
            log(level::warn, fmt, args...);
        }

        template<typename... Args>
        void error(const wchar_t *fmt, const Args &... args)
        {
            log(level::err, fmt, args...);
        }

        template<typename... Args>
        void critical(const wchar_t *fmt, const Args &... args)
        {
            log(level::critical, fmt, args...);
        }
#endif // _WIN32
#endif // SPDLOG_WCHAR_TO_UTF8_SUPPORT

        template<typename T>
        void log(level::level_enum lvl, const T &msg)
        {
            log(source_loc{}, lvl, msg);
        }

        // T can be statically converted to string_view
        template<class T, typename std::enable_if<std::is_convertible<T, spdlog::string_view_t>::value, T>::type * = nullptr>
        void log(source_loc loc, level::level_enum lvl, const T &msg)
        {
            if (!should_log(lvl))
            {
                return;
            }
            try
            {
                details::log_msg log_msg(loc, &name_, lvl, msg);
                sink_it_(log_msg);
            }
            catch (const std::exception &ex)
            {
                err_handler_(ex.what());
            }
            catch (...)
            {
                err_handler_("Unknown exception in logger");
            }
        }

        // T cannot be statically converted to string_view
        template<class T, typename std::enable_if<!std::is_convertible<T, spdlog::string_view_t>::value, T>::type * = nullptr>
        void log(source_loc loc, level::level_enum lvl, const T &msg)
        {
            if (!should_log(lvl))
            {
                return;
            }
            try
            {
                using details::fmt_helper::to_string_view;
                fmt::memory_buffer buf;
                fmt::format_to(buf, "{}", msg);
                details::log_msg log_msg(loc, &name_, lvl, to_string_view(buf));
                sink_it_(log_msg);
            }
            catch (const std::exception &ex)
            {
                err_handler_(ex.what());
            }
            catch (...)
            {
                err_handler_("Unknown exception in logger");
            }
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

        bool should_log(level::level_enum msg_level) const;

        void set_level(level::level_enum log_level);

        static level::level_enum default_level();

        level::level_enum level() const;

        const std::string &name() const;

        // set formatting for the sinks in this logger.
        // each sink will get a seperate instance of the formatter object.
        void set_formatter(std::unique_ptr<formatter> f);

        void set_pattern(std::string pattern, pattern_time_type time_type = pattern_time_type::local);

        // flush functions
        void flush();
        void flush_on(level::level_enum log_level);
        level::level_enum flush_level() const;

        // sinks
        const std::vector<sink_ptr> &sinks() const;

        std::vector<sink_ptr> &sinks();

        // error handler
        void set_error_handler(log_err_handler err_handler);

        log_err_handler error_handler() const;

        // create new logger with same sinks and configuration.
        virtual std::shared_ptr<logger> clone(std::string logger_name);

    protected:
        virtual void sink_it_(details::log_msg &msg);

        virtual void flush_();
        bool should_flush_(const details::log_msg &msg);

        // default error handler.
        // print the error to stderr with the max rate of 1 message/minute.
        void default_err_handler_(const std::string &msg);

        // increment the message count (only if defined(SPDLOG_ENABLE_MESSAGE_COUNTER))
        void incr_msg_counter_(details::log_msg &msg);

        const std::string name_;
        std::vector<sink_ptr> sinks_;
        spdlog::level_t level_{spdlog::logger::default_level()};
        spdlog::level_t flush_level_{level::off};
        log_err_handler err_handler_{[this](const std::string &msg) { this->default_err_handler_(msg); }};
        std::atomic<time_t> last_err_time_{0};
        std::atomic<size_t> msg_counter_{1};
    };
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "../src/logger.cpp"
#endif // SPDLOG_HEADER_ONLY
