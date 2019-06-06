// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include "spdlog/logger.h"
#endif

#include "spdlog/sinks/sink.h"
#include "spdlog/details/pattern_formatter.h"

namespace spdlog {
// public methods
SPDLOG_INLINE void logger::log(source_loc loc, level::level_enum lvl, const char *msg)
{
    if (!should_log(lvl))
    {
        return;
    }

    try
    {
        details::log_msg log_msg(loc, string_view_t(name_), lvl, string_view_t(msg));
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

SPDLOG_INLINE void logger::log(level::level_enum lvl, const char *msg)
{
    log(source_loc{}, lvl, msg);
}

SPDLOG_INLINE bool logger::should_log(level::level_enum msg_level) const
{
    return msg_level >= level_.load(std::memory_order_relaxed);
}

SPDLOG_INLINE void logger::set_level(level::level_enum log_level)
{
    level_.store(log_level);
}

SPDLOG_INLINE level::level_enum logger::default_level()
{
    return static_cast<level::level_enum>(SPDLOG_ACTIVE_LEVEL);
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
// each sink will get a seperate instance of the formatter object.
SPDLOG_INLINE void logger::set_formatter(std::unique_ptr<formatter> f)
{
    for (auto it = sinks_.begin(); it != sinks_.end(); ++it)
    {
        if (std::next(it) == sinks_.end())
        {
            // last element - we can be move it.
            (*it)->set_formatter(std::move(f));
        }
        else
        {
            (*it)->set_formatter(f->clone());
        }
    }
}

SPDLOG_INLINE void logger::set_pattern(std::string pattern, pattern_time_type time_type)
{
    auto new_formatter = details::make_unique<pattern_formatter>(std::move(pattern), time_type);
    set_formatter(std::move(new_formatter));
}

// flush functions
SPDLOG_INLINE void logger::flush()
{
    try
    {
        flush_();
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
    custom_err_handler_ = handler;
}

// create new logger with same sinks and configuration.
SPDLOG_INLINE std::shared_ptr<logger> logger::clone(std::string logger_name)
{
    auto cloned = std::make_shared<logger>(std::move(logger_name), sinks_.begin(), sinks_.end());
    cloned->set_level(this->level());
    cloned->flush_on(this->flush_level());
    cloned->set_error_handler(this->custom_err_handler_);
    return cloned;
}

// protected methods
SPDLOG_INLINE void logger::sink_it_(details::log_msg &msg)
{
    for (auto &sink : sinks_)
    {
        if (sink->should_log(msg.level))
        {
            sink->log(msg);
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
        sink->flush();
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
        auto tm_time = details::os::localtime();
        char date_buf[64];
        std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", &tm_time);
        fmt::print(stderr, "[*** LOG ERROR ***] [{}] [{}] {}\n", date_buf, name(), msg);
    }
}
} // namespace spdlog