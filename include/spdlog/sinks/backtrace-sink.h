// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "dist_sink.h"
#include "spdlog/common.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/details/log_msg_buffer.h"
#include "spdlog/details/circular_q.h"

#include <mutex>
#include <string>
#include <chrono>

// Store log messages in circular buffer
// If it encounters a message with high enough level, it will send all previous message to it child sinks
// Useful for storing debug data in case of error/warning happens

namespace spdlog {
namespace sinks {
template<typename Mutex>
class backtrace_sink : public dist_sink<Mutex>
{
public:
    explicit backtrace_sink(spdlog::level::level_enum filter_level, size_t n_messages)
        : filter_level_{filter_level}
        , traceback_msgs_{n_messages}
    {}

    void set_filter_level(spdlog::level::level_enum filter_level)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        filter_level_ = filter_level;
    }

    spdlog::level::level_enum get_filter_level()
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return filter_level_;
    }

    void dump_backtrace(string_view_t logger_name)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        dump_backtrace_(logger_name);
    }

protected:
    spdlog::level::level_enum filter_level_;
    details::circular_q<details::log_msg_buffer> traceback_msgs_;

    // save the messages in a circular queue
    void sink_it_(const details::log_msg &msg) override
    {
        traceback_msgs_.push_back(details::log_msg_buffer(msg));

        if (msg.level >= filter_level_)
        {
            dist_sink<Mutex>::sink_it_(msg);
        }
    }

    void dump_backtrace_(string_view_t logger_name)
    {
        if (traceback_msgs_.empty())
        {
            return;
        }
        dist_sink<Mutex>::sink_it_(
            details::log_msg{logger_name, level::info, "********************* Backtrace Start *********************"});

        do
        {
            details::log_msg_buffer popped;
            traceback_msgs_.pop_front(popped);
            dist_sink<Mutex>::sink_it_(popped);
        } while (!traceback_msgs_.empty());

        dist_sink<Mutex>::sink_it_(
            details::log_msg{logger_name, level::info, "********************* Backtrace End ***********************"});
    }
};

using backtrace_sink_mt = backtrace_sink<std::mutex>;
using backtrace_sink_st = backtrace_sink<details::null_mutex>;

} // namespace sinks
} // namespace spdlog
