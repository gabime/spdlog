//
// Copyright(c) 2019 ZVYAGIN.Alexander@gmail.com
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#ifndef SPDLOG_H
#include "spdlog/spdlog.h"
#endif

#include "spdlog/sinks/base_sink.h"

#include <array>
#include <string>
#include <systemd/sd-journal.h>

namespace spdlog {
namespace sinks {

inline int syslog_level(level::level_enum l) {
    switch(l) {
        case level::off:
        case level::trace:
        case level::debug:
            return LOG_DEBUG;
        case level::info:
            return LOG_INFO;
        case level::warn:
            return LOG_WARNING;
        case level::err:
            return LOG_ERR;
        case level::critical:
            return LOG_CRIT;
        default:
            throw std::invalid_argument("systemd_sink.h syslog_level()");
    }
}

/**
 * Sink that write to systemd using the `sd_journal_print()` library call.
 *
 * Locking is not needed, as `sd_journal_print()` itself is thread-safe.
 */
template<typename Mutex>
class systemd_sink : public base_sink<Mutex>
{
public:
    //
    explicit systemd_sink(void) {}

    ~systemd_sink() override {}

    systemd_sink(const systemd_sink &) = delete;
    systemd_sink &operator=(const systemd_sink &) = delete;

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        if( sd_journal_print(
                syslog_level(msg.level),
                "%.*s",
                static_cast<int>(msg.payload.size()),
                msg.payload.data()
            )
        )
            throw spdlog_ex("Failed writing to systemd");
    }

    void flush_() override {}
};

using systemd_sink_mt = systemd_sink<std::mutex>;
using systemd_sink_st = systemd_sink<details::null_mutex>;
} // namespace sinks

// Create and register a syslog logger
template<typename Factory = default_factory>
inline std::shared_ptr<logger> systemd_logger_mt(
    const std::string &logger_name)
{
    return Factory::template create<sinks::systemd_sink_mt>(logger_name);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> systemd_logger_st(
    const std::string &logger_name)
{
    return Factory::template create<sinks::systemd_sink_st>(logger_name);
}
} // namespace spdlog
