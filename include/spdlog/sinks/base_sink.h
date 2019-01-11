//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
//
// base sink templated over a mutex (either dummy or real)
// concrete implementation should override the sink_it_() and flush_()  methods.
// locking is taken care of in this class - no locking needed by the
// implementers..
//

#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/formatter.h"
#include "spdlog/sinks/sink.h"

namespace spdlog {
namespace sinks {
template<typename Mutex>
class base_sink : public sink
{
public:
    base_sink() = default;
    base_sink(const base_sink &) = delete;
    base_sink &operator=(const base_sink &) = delete;

    void log(const details::log_msg &msg) final
    {
        std::lock_guard<Mutex> lock(mutex_);
        sink_it_(msg);
    }

    void flush() final
    {
        std::lock_guard<Mutex> lock(mutex_);
        flush_();
    }

    void set_pattern(const std::string &pattern) final
    {
        std::lock_guard<Mutex> lock(mutex_);
        set_pattern_(pattern);
    }

    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) final
    {
        std::lock_guard<Mutex> lock(mutex_);
        set_formatter_(std::move(sink_formatter));
    }

protected:
    virtual void sink_it_(const details::log_msg &msg) = 0;
    virtual void flush_() = 0;

    virtual void set_pattern_(const std::string &pattern)
    {
        set_formatter_(details::make_unique<spdlog::pattern_formatter>(pattern));
    }

    virtual void set_formatter_(std::unique_ptr<spdlog::formatter> sink_formatter)
    {
        formatter_ = std::move(sink_formatter);
    }
    Mutex mutex_;
};
} // namespace sinks
} // namespace spdlog
