//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/details/null_mutex.h"
#include "spdlog/details/traits.h"
#include "spdlog/spdlog.h"

#include <cstdio>
#include <memory>
#include <mutex>

namespace spdlog {

namespace sinks {

template<class StdoutTrait, class ConsoleMutexTrait>
class stdout_sink : public sink
{
public:
    using mutex_t = typename ConsoleMutexTrait::mutex_t;
    stdout_sink()
        : mutex_(ConsoleMutexTrait::console_mutex())
        , file_(StdoutTrait::stream())
    {
    }
    ~stdout_sink() = default;

    stdout_sink(const stdout_sink &other) = delete;
    stdout_sink &operator=(const stdout_sink &other) = delete;

    void log(const details::log_msg &msg) override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        fwrite(msg.formatted.data(), sizeof(char), msg.formatted.size(), file_);
        fflush(StdoutTrait::stream());
    }

    void flush() override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        fflush(StdoutTrait::stream());
    }

private:
    mutex_t &mutex_;
    FILE *file_;
};

using stdout_sink_mt = stdout_sink<details::console_stdout_trait, details::console_mutex_trait>;
using stdout_sink_st = stdout_sink<details::console_stdout_trait, details::console_null_mutex_trait>;
using stderr_sink_mt = stdout_sink<details::console_stderr_trait, details::console_mutex_trait>;
using stderr_sink_st = stdout_sink<details::console_stderr_trait, details::console_null_mutex_trait>;

} // namespace sinks

// factory methods
template<typename Factory = default_factory>
inline std::shared_ptr<logger> stdout_logger_mt(const std::string &logger_name)
{
    return Factory::template create<sinks::stdout_sink_mt>(logger_name);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> stdout_logger_st(const std::string &logger_name)
{
    return Factory::template create<sinks::stdout_sink_st>(logger_name);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> stderr_logger_mt(const std::string &logger_name)
{
    return Factory::template create<sinks::stderr_sink_mt>(logger_name);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> stderr_logger_st(const std::string &logger_name)
{
    return Factory::template create<sinks::stderr_sink_st>(logger_name);
}
} // namespace spdlog
