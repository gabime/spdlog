// Copyright(c) 2015-present Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/sinks/sink.h"
#include "spdlog/details/pattern_formatter.h"

// public methods
SPDLOG_INLINE void spdlog::logger::log(spdlog::source_loc loc, spdlog::level::level_enum lvl, const char *msg)
{
    if (!should_log(lvl))
    {
        return;
    }

    try
    {
        details::log_msg log_msg(loc, &name_, lvl, spdlog::string_view_t(msg));
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

SPDLOG_INLINE void spdlog::logger::log(level::level_enum lvl, const char *msg)
{
    log(source_loc{}, lvl, msg);
}

SPDLOG_INLINE bool spdlog::logger::should_log(spdlog::level::level_enum msg_level) const
{
    return msg_level >= level_.load(std::memory_order_relaxed);
}

SPDLOG_INLINE void spdlog::logger::set_level(spdlog::level::level_enum log_level)
{
    level_.store(log_level);
}

SPDLOG_INLINE spdlog::level::level_enum spdlog::logger::default_level()
{
    return static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL);
}

SPDLOG_INLINE spdlog::level::level_enum spdlog::logger::level() const
{
    return static_cast<spdlog::level::level_enum>(level_.load(std::memory_order_relaxed));
}

SPDLOG_INLINE const std::string &spdlog::logger::name() const
{
    return name_;
}

// set formatting for the sinks in this logger.
// each sink will get a seperate instance of the formatter object.
SPDLOG_INLINE void spdlog::logger::set_formatter(std::unique_ptr<spdlog::formatter> f)
{
    for (auto &sink : sinks_)
    {
        sink->set_formatter(f->clone());
    }
}

SPDLOG_INLINE void spdlog::logger::set_pattern(std::string pattern, spdlog::pattern_time_type time_type)
{
    auto new_formatter = details::make_unique<spdlog::pattern_formatter>(std::move(pattern), time_type);
    set_formatter(std::move(new_formatter));
}

// flush functions
SPDLOG_INLINE void spdlog::logger::flush()
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

SPDLOG_INLINE void spdlog::logger::flush_on(level::level_enum log_level)
{
    flush_level_.store(log_level);
}

SPDLOG_INLINE spdlog::level::level_enum spdlog::logger::flush_level() const
{
    return static_cast<spdlog::level::level_enum>(flush_level_.load(std::memory_order_relaxed));
}

// sinks
SPDLOG_INLINE const std::vector<spdlog::sink_ptr> &spdlog::logger::sinks() const
{
    return sinks_;
}

SPDLOG_INLINE std::vector<spdlog::sink_ptr> &spdlog::logger::sinks()
{
    return sinks_;
}

// error handler
SPDLOG_INLINE void spdlog::logger::set_error_handler(err_handler handler)
{
    custom_err_handler_ = handler;
}

// create new logger with same sinks and configuration.
SPDLOG_INLINE std::shared_ptr<spdlog::logger> spdlog::logger::clone(std::string logger_name)
{
    auto cloned = std::make_shared<spdlog::logger>(std::move(logger_name), sinks_.begin(), sinks_.end());
    cloned->set_level(this->level());
    cloned->flush_on(this->flush_level());
    cloned->set_error_handler(this->custom_err_handler_);
    return cloned;
}

// protected methods
SPDLOG_INLINE void spdlog::logger::sink_it_(spdlog::details::log_msg &msg)
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

SPDLOG_INLINE void spdlog::logger::flush_()
{
    for (auto &sink : sinks_)
    {
        sink->flush();
    }
}

SPDLOG_INLINE bool spdlog::logger::should_flush_(const spdlog::details::log_msg &msg)
{
    auto flush_level = flush_level_.load(std::memory_order_relaxed);
    return (msg.level >= flush_level) && (msg.level != level::off);
}

SPDLOG_INLINE void spdlog::logger::err_handler_(const std::string &msg)
{
    if (custom_err_handler_)
    {
        custom_err_handler_(msg);
    }
    else
    {
        auto tm_time = spdlog::details::os::localtime();
        char date_buf[64];
        std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", &tm_time);
        fmt::print(stderr, "[*** LOG ERROR ***] [{}] [{}] {}\n", date_buf, name(), msg);
    }
}
