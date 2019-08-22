// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "dist_sink.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/details/log_msg_buffer.h"
#include "spdlog/details/circular_q.h"

#include <mutex>
#include <string>
#include <chrono>

// Store log messages in circular buffer
// If it encounters a message with high enough level, it will send all pervious message to its child sinks
// Useful for storing debug data in case of error/warning happens

//
// Example:
//
//     #include "spdlog/spdlog.h"
//     #include "spdlog/sinks/backtrace_sink.h
//
//     int main() {
//         auto backtrace_sink = std::make_shared<backtrace_sink>();
//         backtrace_sink ->add_sink(std::make_shared<stdout_color_sink_mt>());
//         spdlog::logger l("logger", backtrace_sink);
//         logger.set_level(spdlog::level::trace);
//         l.trace("Hello");
//         l.debug("Hello");
//         l.info("Hello");
//         l.warn("This will trigger the log of all prev messages in the queue");
//     }

namespace spdlog {
namespace sinks {
template<typename Mutex>
class backtrace_sink : public dist_sink<Mutex>
{
public:
    explicit backtrace_sink(level::level_enum trigger_level = spdlog::level::err, size_t n_messages = 16)
        : trigger_level_{trigger_level}
        , traceback_msgs_{n_messages}
    {}

protected:
    level::level_enum trigger_level_;

    details::circular_q<details::log_msg_buffer> traceback_msgs_;

    // if current message is high enough, trigger a backtrace log,
    // otherwise save the message in the queue for future trigger.
    void sink_it_(const details::log_msg &msg) override
    {
        if (msg.level < trigger_level_)
        {
            traceback_msgs_.push_back(details::log_msg_buffer(msg));
        }
        if (msg.level > level::debug)
        {
            dist_sink<Mutex>::sink_it_(msg);
        }
        if (msg.level >= trigger_level_)
        {
            log_backtrace_(msg.logger_name);
        }
    }

    void log_backtrace_(const string_view_t &logger_name)
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
