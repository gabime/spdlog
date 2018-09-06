//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/sinks/base_sink.h"
#include "spdlog/spdlog.h"

#include <array>
#include <string>
#include <syslog.h>

namespace spdlog {
namespace sinks {
/**
 * Sink that write to syslog using the `syscall()` library call.
 *
 * Locking is not needed, as `syslog()` itself is thread-safe.
 */
template<typename Mutex>
class syslog_sink : public base_sink<Mutex>
{
public:
    //
    explicit syslog_sink(std::string ident = "", int syslog_option = 0, int syslog_facility = LOG_USER)
        : ident_(std::move(ident))
    {
        priorities_[static_cast<size_t>(level::trace)] = LOG_DEBUG;
        priorities_[static_cast<size_t>(level::debug)] = LOG_DEBUG;
        priorities_[static_cast<size_t>(level::info)] = LOG_INFO;
        priorities_[static_cast<size_t>(level::warn)] = LOG_WARNING;
        priorities_[static_cast<size_t>(level::err)] = LOG_ERR;
        priorities_[static_cast<size_t>(level::critical)] = LOG_CRIT;
        priorities_[static_cast<size_t>(level::off)] = LOG_INFO;

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
        ::syslog(syslog_prio_from_level(msg), "%s", fmt::to_string(msg.raw).c_str());
    }

    void flush_() override {}

private:
    std::array<int, 7> priorities_;
    // must store the ident because the man says openlog might use the pointer as
    // is and not a string copy
    const std::string ident_;

    //
    // Simply maps spdlog's log level to syslog priority level.
    //
    int syslog_prio_from_level(const details::log_msg &msg) const
    {
        return priorities_[static_cast<size_t>(msg.level)];
    }
};

using syslog_sink_mt = syslog_sink<std::mutex>;
using syslog_sink_st = syslog_sink<details::null_mutex>;
} // namespace sinks

// Create and register a syslog logger
template<typename Factory = default_factory>
inline std::shared_ptr<logger> syslog_logger_mt(
    const std::string &logger_name, const std::string &syslog_ident = "", int syslog_option = 0, int syslog_facility = (1 << 3))
{
    return Factory::template create<sinks::syslog_sink_mt>(logger_name, syslog_ident, syslog_option, syslog_facility);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> syslog_logger_st(
    const std::string &logger_name, const std::string &syslog_ident = "", int syslog_option = 0, int syslog_facility = (1 << 3))
{
    return Factory::template create<sinks::syslog_sink_st>(logger_name, syslog_ident, syslog_option, syslog_facility);
}
} // namespace spdlog
