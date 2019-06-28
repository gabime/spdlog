// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/null_mutex.h"

#include <array>
#include <string>
#include <syslog.h>

namespace spdlog {
namespace sinks {
/**
 * Sink that write to syslog using the `syscall()` library call.
 */
template<typename Mutex>
class syslog_sink : public base_sink<Mutex>
{

public:
    syslog_sink(std::string ident, int syslog_option, int syslog_facility, bool enable_formatting)
        : enable_formatting_{enable_formatting}
        , ident_{std::move(ident)}
    {
        // set ident to be program name if empty
        ::openlog(ident_.empty() ? nullptr : ident_.c_str(), syslog_option, syslog_facility);
    }

    ~syslog_sink() override
    {
        ::closelog();
    }

    syslog_sink(const syslog_sink &) = delete;
    syslog_sink &operator=(const syslog_sink &) = delete;

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        string_view_t payload;

        if (enable_formatting_)
        {
            fmt::memory_buffer formatted;
            base_sink<Mutex>::formatter_->format(msg, formatted);
            payload = string_view_t(formatted.data(), formatted.size());
        }
        else
        {
            payload = msg.payload;
        }

        ::syslog(syslog_prio_from_level(msg), "%s", payload.data());
    }

    void flush_() override {}
    bool enable_formatting_ = false;

private:
    std::array<int, 7> syslog_levels_ {
            /* level::trace      */ LOG_DEBUG,
            /* level::debug      */ LOG_DEBUG,
            /* level::info       */ LOG_INFO,
            /* level::warn       */ LOG_WARNING,
            /* level::err        */ LOG_ERR,
            /* level::critical   */ LOG_CRIT,
            /* level::off        */ LOG_INFO
    };
    // must store the ident because the man says openlog might use the pointer as
    // is and not a string copy
    const std::string ident_;

    //
    // Simply maps spdlog's log level to syslog priority level.
    //
    int syslog_prio_from_level(const details::log_msg &msg) const
    {
        return syslog_levels_.at(static_cast<int>(msg.level));
    }
};

using syslog_sink_mt = syslog_sink<std::mutex>;
using syslog_sink_st = syslog_sink<details::null_mutex>;
} // namespace sinks

// Create and register a syslog logger
template<typename Factory = default_factory>
inline std::shared_ptr<logger> syslog_logger_mt(const std::string &logger_name, const std::string &syslog_ident = "", int syslog_option = 0,
    int syslog_facility = LOG_USER, bool enable_formatting = false)
{
    return Factory::template create<sinks::syslog_sink_mt>(logger_name, syslog_ident, syslog_option, syslog_facility, enable_formatting);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> syslog_logger_st(const std::string &logger_name, const std::string &syslog_ident = "", int syslog_option = 0,
    int syslog_facility = LOG_USER, bool enable_formatting = false)
{
    return Factory::template create<sinks::syslog_sink_st>(logger_name, syslog_ident, syslog_option, syslog_facility, enable_formatting);
}
} // namespace spdlog
