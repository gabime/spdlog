// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#    include <spdlog/logger.h>
#endif

#include <spdlog/sinks/sink.h>

#include <cstdio>

namespace spdlog {

// public methods
SPDLOG_INLINE logger::logger(const logger &other)
    : name_(other.name_)
    , sinks_(other.sinks_)
    , level_(other.level_.load(std::memory_order_relaxed))
    , flush_level_(other.flush_level_.load(std::memory_order_relaxed))
    , custom_err_handler_(other.custom_err_handler_)
{}

SPDLOG_INLINE logger::logger(logger &&other) SPDLOG_NOEXCEPT : name_(std::move(other.name_)),
                                                               sinks_(std::move(other.sinks_)),
                                                               level_(other.level_.load(std::memory_order_relaxed)),
                                                               flush_level_(other.flush_level_.load(std::memory_order_relaxed)),
                                                               custom_err_handler_(std::move(other.custom_err_handler_))

{}

SPDLOG_INLINE logger &logger::operator=(logger other) SPDLOG_NOEXCEPT
{
    this->swap(other);
    return *this;
}

SPDLOG_INLINE void logger::swap(spdlog::logger &other) SPDLOG_NOEXCEPT
{
    name_.swap(other.name_);
    sinks_.swap(other.sinks_);

    // swap level_
    auto other_level = other.level_.load();
    auto my_level = level_.exchange(other_level);
    other.level_.store(my_level);

    // swap flush level_
    other_level = other.flush_level_.load();
    my_level = flush_level_.exchange(other_level);
    other.flush_level_.store(my_level);

    custom_err_handler_.swap(other.custom_err_handler_);
}

SPDLOG_INLINE void swap(logger &a, logger &b)
{
    a.swap(b);
}

SPDLOG_INLINE void logger::set_level(level::level_enum log_level)
{
    level_.store(log_level);
}

SPDLOG_INLINE level::level_enum logger::level() const
{
    return static_cast<level::level_enum>(level_.load(std::memory_order_relaxed));
}

SPDLOG_INLINE const std::string &logger::name() const
{
    return name_;
}

// set formatting for the sinks in this logger.
// each sink will get a separate instance of the formatter object.
SPDLOG_INLINE void logger::set_formatter(std::unique_ptr<formatter> f)
{
    for (auto it = sinks_.begin(); it != sinks_.end(); ++it)
    {
        if (std::next(it) == sinks_.end())
        {
            // last element - we can be move it.
            (*it)->set_formatter(std::move(f));
            break; // to prevent clang-tidy warning
        }
        else
        {
            (*it)->set_formatter(f->clone());
        }
    }
}

// flush functions
SPDLOG_INLINE void logger::flush()
{
    flush_();
}

SPDLOG_INLINE void logger::flush_on(level::level_enum log_level)
{
    flush_level_.store(log_level);
}

SPDLOG_INLINE level::level_enum logger::flush_level() const
{
    return static_cast<level::level_enum>(flush_level_.load(std::memory_order_relaxed));
}

// sinks
SPDLOG_INLINE const std::vector<sink_ptr> &logger::sinks() const
{
    return sinks_;
}

SPDLOG_INLINE std::vector<sink_ptr> &logger::sinks()
{
    return sinks_;
}

// error handler
SPDLOG_INLINE void logger::set_error_handler(err_handler handler)
{
    custom_err_handler_ = std::move(handler);
}

// create new logger with same sinks and configuration.
SPDLOG_INLINE std::shared_ptr<logger> logger::clone(std::string logger_name)
{
    auto cloned = std::make_shared<logger>(*this);
    cloned->name_ = std::move(logger_name);
    return cloned;
}

SPDLOG_INLINE void logger::sink_it_(const details::log_msg &msg)
{
    for (auto &sink : sinks_)
    {
        if (sink->should_log(msg.level))
        {
            SPDLOG_TRY
            {
                sink->log(msg);
            }
            SPDLOG_LOGGER_CATCH(msg.source)
        }
    }

    if (should_flush_(msg))
    {
        flush_();
    }
}

SPDLOG_INLINE void logger::flush_()
{
    for (auto &sink : sinks_)
    {
        SPDLOG_TRY
        {
            sink->flush();
        }
        SPDLOG_LOGGER_CATCH(source_loc())
    }
}

SPDLOG_INLINE bool logger::should_flush_(const details::log_msg &msg)
{
    auto flush_level = flush_level_.load(std::memory_order_relaxed);
    return (msg.level >= flush_level) && (msg.level != level::off);
}

SPDLOG_INLINE void logger::err_handler_(const std::string &msg)
{
    if (custom_err_handler_)
    {
        custom_err_handler_(msg);
    }
    else
    {
        auto now = log_clock::now();
        auto tm_time = details::os::localtime(log_clock::to_time_t(now));
        char date_buf[64];
        std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", &tm_time);
#if defined(USING_R) && defined(R_R_H) // if in R environment
        REprintf("[*** LOG ERROR ***] [%s] [%s] {%s}\n", date_buf, name().c_str(), msg.c_str());
#else
        std::fprintf(stderr, "[*** LOG ERROR ***] [%s] [%s] {%s}\n", date_buf, name().c_str(), msg.c_str());
#endif
    }
}
} // namespace spdlog
