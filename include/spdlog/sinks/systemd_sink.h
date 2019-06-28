// Copyright(c) 2019 ZVYAGIN.Alexander@gmail.com
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/details/synchronous_factory.h"

#include <systemd/sd-journal.h>

namespace spdlog {
namespace sinks {

inline int syslog_level(level::level_enum l)
{
    switch (l)
    {
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
 * Sink that write to systemd journal using the `sd_journal_send()` library call.
 *
 * Locking is not needed, as `sd_journal_send()` itself is thread-safe.
 */
template<typename Mutex>
class systemd_sink : public base_sink<Mutex>
{
public:
    //
    explicit systemd_sink() {}

    ~systemd_sink() override {}

    systemd_sink(const systemd_sink &) = delete;
    systemd_sink &operator=(const systemd_sink &) = delete;

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        const char* key_msg  = "MESSAGE=%.*s";
        const char* key_prio = "PRIORITY=%d";
        const char* key_file = "CODE_FILE=%s";
        const char* key_line = "CODE_LINE=%d";
        const char* key_func = "CODE_FUNC=%s";

        if( !msg.source.filename || !msg.source.funcname || msg.source.line == 0 ) {
            // Do not send source location if not available
            key_file = nullptr;
        }

        // Note: function call inside '()' to avoid macro expansion
        int err = (sd_journal_send)(
                key_msg, static_cast<int>(msg.payload.size()), msg.payload.data(),
                key_prio, syslog_level(msg.level),
                key_file, msg.source.filename,
                key_line, msg.source.line,
                key_func, msg.source.funcname,
                nullptr);

        if( err ) {
            throw spdlog_ex("Failed writing to systemd");
        }
    }

    void flush_() override {}
};

using systemd_sink_mt = systemd_sink<std::mutex>;
using systemd_sink_st = systemd_sink<details::null_mutex>;
} // namespace sinks

// Create and register a syslog logger
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> systemd_logger_mt(const std::string &logger_name)
{
    return Factory::template create<sinks::systemd_sink_mt>(logger_name);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> systemd_logger_st(const std::string &logger_name)
{
    return Factory::template create<sinks::systemd_sink_st>(logger_name);
}
} // namespace spdlog
