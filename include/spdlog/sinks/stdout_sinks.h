//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/details/console_globals.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/spdlog.h"

#include <cstdio>
#include <memory>
#include <mutex>
#include <spdlog/details/console_globals.h>

namespace spdlog {

namespace sinks {

template<typename TargetStream, typename ConsoleMutex>
class stdout_sink SPDLOG_FINAL : public sink
{
public:
    using mutex_t = typename ConsoleMutex::mutex_t;
    stdout_sink()
        : mutex_(ConsoleMutex::mutex())
        , file_(TargetStream::stream())
    {
    }
    ~stdout_sink() override = default;

    stdout_sink(const stdout_sink &other) = delete;
    stdout_sink &operator=(const stdout_sink &other) = delete;

    void log(const details::log_msg &msg) override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        fmt::memory_buffer formatted;
        formatter_->format(msg, formatted);
        fwrite(formatted.data(), sizeof(char), formatted.size(), file_);
        fflush(TargetStream::stream());
    }

    void flush() override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        fflush(file_);
    }

    void set_pattern(const std::string &pattern) override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        formatter_ = std::unique_ptr<spdlog::formatter>(new pattern_formatter(pattern));
    }

    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        formatter_ = std::move(sink_formatter);
    }

private:
    mutex_t &mutex_;
    FILE *file_;
};

using stdout_sink_mt = stdout_sink<details::console_stdout, details::console_mutex>;
using stdout_sink_st = stdout_sink<details::console_stdout, details::console_nullmutex>;

using stderr_sink_mt = stdout_sink<details::console_stderr, details::console_mutex>;
using stderr_sink_st = stdout_sink<details::console_stderr, details::console_nullmutex>;

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
