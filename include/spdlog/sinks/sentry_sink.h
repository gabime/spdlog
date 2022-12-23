#pragma once

#include "base_sink.h"
#include "spdlog/logger.h"
#include "spdlog/details/synchronous_factory.h"

#include <sentry.h>

namespace spdlog {
namespace sinks {

template<typename Mutex>
class sentry_sink : public base_sink<Mutex>
{
public:
    explicit sentry_sink(const std::string &dsn)
    {
        sentry_options_t *options = sentry_options_new();

        sentry_options_set_dsn(options, dsn.c_str());
        sentry_options_set_auto_session_tracking(options, false);
        sentry_options_set_symbolize_stacktraces(options, true);
        sentry_options_set_backend(options, nullptr);

        sentry_init(options);
    }

    ~sentry_sink()
    {
        sentry_close();
    }

    sentry_sink(const sentry_sink& ) = delete;
    sentry_sink(sentry_sink&& ) = delete;

    sentry_sink& operator=(const sentry_sink&) = delete;
    sentry_sink& operator=(sentry_sink&&) = delete;

protected:
    void flush_() override {};

    void sink_it_(const details::log_msg &msg) override
    {
        auto crumb = sentry_value_new_breadcrumb(msg.logger_name.data(), msg.payload.data());
        auto location = sentry_value_new_object();

        sentry_value_set_by_key(location, "file", sentry_value_new_string(msg.source.filename));
        sentry_value_set_by_key(location, "line", sentry_value_new_int32(msg.source.line));
        sentry_value_set_by_key(location, "func", sentry_value_new_int32(msg.source.funcname));
        sentry_value_set_by_key(crumb, "data", location);
        sentry_add_breadcrumb(crumb);

        sentry_level_e sentry_level {};
        switch (msg.level)
        {
        case level::trace:
            sentry_level = sentry_level_e::SENTRY_LEVEL_DEBUG;
            break;
        case level::debug:
            sentry_level = sentry_level_e::SENTRY_LEVEL_DEBUG;
            break;
        case level::info:
            sentry_level = sentry_level_e::SENTRY_LEVEL_INFO;
            break;
        case level::warn:
            sentry_level = sentry_level_e::SENTRY_LEVEL_WARNING;
            break;
        case level::err:
            sentry_level = sentry_level_e::SENTRY_LEVEL_ERROR;
            break;
        case level::critical:
            sentry_level = sentry_level_e::SENTRY_LEVEL_FATAL;
            break;
        case level::off:
            break;
        case level::n_levels:
            break;
        }

        sentry_capture_event(sentry_value_new_message_event(sentry_level, msg.logger_name.data(), msg.payload.data()));
    }
};

#include "spdlog/details/null_mutex.h"
#include <mutex>
using sentry_sink_mt = sentry_sink<std::mutex>;
using sentry_sink_st = sentry_sink<spdlog::details::null_mutex>;

} // namespace sinks

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> sentry_logger_mt(const std::string &logger_name, const std::string &dsn)
{
    return Factory::template create<sinks::sentry_sink_mt>(logger_name, dsn);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> sentry_logger_st(const std::string &logger_name, const std::string &dsn)
{
    return Factory::template create<sinks::sentry_sink_st>(logger_name, dsn);
}
} // namespace spdlog
