//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
//
// base sink templated over a mutex (either dummy or real)
// concrete implementation should override the sink_it_() and flush_()  methods.
// locking is taken care of in this class - no locking needed by the implementers..
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
