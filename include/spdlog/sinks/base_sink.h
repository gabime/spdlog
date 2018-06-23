//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
//
// base sink templated over a mutex (either dummy or real)
// concrete implementation should only override the sink_it_ method.
// all locking is taken care of here so no locking needed by the implementers..
//

#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/formatter.h"
#include "spdlog/sinks/sink.h"

namespace spdlog {
namespace sinks {
template<class Mutex>
class base_sink : public sink
{
public:
    base_sink()
        : sink()
    {
    }

    base_sink(const std::string &formatter_pattern)
        : sink(formatter_pattern)
    {
    }

    base_sink(std::unique_ptr<spdlog::formatter> sink_formatter)
        : sink(std::move(sink_formatter))
    {
    }

    base_sink(const base_sink &) = delete;
    base_sink &operator=(const base_sink &) = delete;

    void log(const details::log_msg &msg) SPDLOG_FINAL override
    {
        std::lock_guard<Mutex> lock(mutex_);
        fmt::memory_buffer formatted;
        formatter_->format(msg, formatted);
        sink_it_(msg, formatted);
    }

    void flush() SPDLOG_FINAL override
    {
        std::lock_guard<Mutex> lock(mutex_);
        flush_();
    }

protected:
    virtual void sink_it_(const details::log_msg &msg, const fmt::memory_buffer &formatted) = 0;
    virtual void flush_() = 0;
    Mutex mutex_;
};
} // namespace sinks
} // namespace spdlog
