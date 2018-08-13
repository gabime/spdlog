//
// Copyright(c) 2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/null_mutex.h"


namespace spdlog {
namespace sinks {

template<class Mutex>
class test_sink : public base_sink<Mutex>
{
public:
    size_t msg_counter()
    {
        return msg_counter_;
    }

    size_t flush_counter()
    {
        return flush_counter_;
    }

    void set_delay(spdlog::chrono::milliseconds delay)
    {
        delay_ = delay;
    }

protected:
    void sink_it_(const details::log_msg &) override
    {
        msg_counter_++;
        spdlog::details::os::sleep_for_millis(delay_.count());
    }

    void flush_() override
    {
        flush_counter_++;
    }
    size_t msg_counter_{0};
    size_t flush_counter_{0};
    spdlog::chrono::milliseconds delay_{spdlog::chrono::milliseconds::zero()};
};

using test_sink_mt = test_sink<spdlog::mutex>;
using test_sink_st = test_sink<details::null_mutex>;

} // namespace sinks
} // namespace spdlog
