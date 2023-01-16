// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/synchronous_factory.h>

#include <mutex>
#include <string>

namespace spdlog {

// callbacks struct
struct custom_log_callbacks
{
    custom_log_callbacks()
        : on_log(nullptr)
        , on_log_formatted(nullptr)
    {}

    std::function<void(const details::log_msg &msg)> on_log;
    std::function<void(const std::string &mag_str)> on_log_formatted;
};

namespace sinks {
/*
 * Trivial callback sink, gets a callback function and calls it on each log
 */
template<typename Mutex>
class callback_sink final : public base_sink<Mutex>
{
public:
    explicit callback_sink(const custom_log_callbacks &callbacks)
        : callbacks_{callbacks}
    {}

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        if (callbacks_.on_log)
            callbacks_.on_log(msg);
        if (callbacks_.on_log_formatted)
        {
            memory_buf_t formatted;
            base_sink<Mutex>::formatter_->format(msg, formatted);
            auto eol_len = strlen(details::os::default_eol);
            std::string str(formatted.data(), formatted.size() - eol_len);
            callbacks_.on_log_formatted(str);
        }
    }
    void flush_() override{};

private:
    custom_log_callbacks callbacks_;
};

using callback_sink_mt = callback_sink<std::mutex>;
using callback_sink_st = callback_sink<details::null_mutex>;

} // namespace sinks

//
// factory functions
//
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> callback_logger_mt(const std::string &logger_name, const custom_log_callbacks &callbacks)
{
    return Factory::template create<sinks::callback_sink_mt>(logger_name, callbacks);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> callback_logger_st(const std::string &logger_name, const custom_log_callbacks &callbacks)
{
    return Factory::template create<sinks::callback_sink_st>(logger_name, callbacks);
}

} // namespace spdlog
