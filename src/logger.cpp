// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/logger.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/pattern_formatter.h>

#include <memory>
#include <mutex>
#include <cstdio>

namespace spdlog {

// public methods
logger::logger(const logger &other)
    : name_(other.name_)
    , sinks_(other.sinks_)
    , level_(other.level_.load(std::memory_order_relaxed))
    , flush_level_(other.flush_level_.load(std::memory_order_relaxed))
    , custom_err_handler_(other.custom_err_handler_)
{}

logger::logger(logger &&other) noexcept
    : name_(std::move(other.name_))
    , sinks_(std::move(other.sinks_))
    , level_(other.level_.load(std::memory_order_relaxed))
    , flush_level_(other.flush_level_.load(std::memory_order_relaxed))
    , custom_err_handler_(std::move(other.custom_err_handler_))
{}

logger &logger::operator=(logger other) noexcept
{
    this->swap(other);
    return *this;
}

void logger::swap(spdlog::logger &other) noexcept
{
    name_.swap(other.name_);
    sinks_.swap(other.sinks_);

    // swap level_
    auto other_level = other.level_.load();
    auto my_level = level_.exchange(other_level);
    other.level_.store(my_level);

    other_level = other.flush_level_.load();
    my_level = flush_level_.exchange(other_level);
    other.flush_level_.store(my_level);
    custom_err_handler_.swap(other.custom_err_handler_);
}

void swap(logger &a, logger &b)
{
    a.swap(b);
}

void logger::set_level(level::level_enum log_level)
{
    level_.store(log_level);
}

level::level_enum logger::level() const
{
    return static_cast<level::level_enum>(level_.load(std::memory_order_relaxed));
}

const std::string &logger::name() const
{
    return name_;
}

// set formatting for the sinks in this logger.
// each sink will get a separate instance of the formatter object.
void logger::set_formatter(std::unique_ptr<formatter> f)
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

void logger::set_pattern(std::string pattern, pattern_time_type time_type)
{
    auto new_formatter = std::make_unique<pattern_formatter>(std::move(pattern), time_type);
    set_formatter(std::move(new_formatter));
}

// flush functions
void logger::flush()
{
    flush_();
}

void logger::flush_on(level::level_enum log_level)
{
    flush_level_.store(log_level);
}

level::level_enum logger::flush_level() const
{
    return static_cast<level::level_enum>(flush_level_.load(std::memory_order_relaxed));
}

// sinks
const std::vector<sink_ptr> &logger::sinks() const
{
    return sinks_;
}

std::vector<sink_ptr> &logger::sinks()
{
    return sinks_;
}

// error handler
void logger::set_error_handler(err_handler handler)
{
    custom_err_handler_ = std::move(handler);
}

// create new logger with same sinks and configuration.
std::shared_ptr<logger> logger::clone(std::string logger_name)
{
    auto cloned = std::make_shared<logger>(*this);
    cloned->name_ = std::move(logger_name);
    return cloned;
}

void logger::sink_it_(const details::log_msg &msg)
{
    for (auto &sink : sinks_)
    {
        if (sink->should_log(msg.level))
        {
            SPDLOG_TRY
            {
                sink->log(msg);
            }
            SPDLOG_LOGGER_CATCH()
        }
    }

    if (should_flush_(msg))
    {
        flush_();
    }
}

void logger::flush_()
{
    for (auto &sink : sinks_)
    {
        SPDLOG_TRY
        {
            sink->flush();
        }
        SPDLOG_LOGGER_CATCH()
    }
}

bool logger::should_flush_(const details::log_msg &msg)
{
    auto flush_level = flush_level_.load(std::memory_order_relaxed);
    return (msg.level >= flush_level) && (msg.level != level::off);
}

void logger::err_handler_(const std::string &msg)
{
    if (custom_err_handler_)
    {
        custom_err_handler_(msg);
    }
    else
    {
        using std::chrono::system_clock;
        static std::mutex mutex;
        static std::chrono::system_clock::time_point last_report_time;
        static size_t err_counter = 0;
        std::lock_guard<std::mutex> lk{mutex};
        auto now = system_clock::now();
        err_counter++;
        if (now - last_report_time < std::chrono::seconds(1))
        {
            return;
        }
        last_report_time = now;
        auto tm_time = details::os::localtime(system_clock::to_time_t(now));
        char date_buf[64];
        std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", &tm_time);
        fprintf(stderr, "[*** LOG ERROR #%04zu ***] [%s] [%s] {%s}\n", err_counter, date_buf, name().c_str(), msg.c_str());
    }
}
} // namespace spdlog
