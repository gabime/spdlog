// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "dist_sink.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/details/log_msg.h"

#include <string>
#include <chrono>

// Duplicate remove sink.
// Filter the message if previous one is identical and less than max max_skip_duration have passed
//
// Example:
//     auto d = std::make_shared<dup_filter_sink_st>(std::chrono::seconds(5));
//     d->add_sink(std::make_shared<stdout_color_sink_mt>());
//     spdlog::logger l("logger", d);
//     l.info("Hello");
//     l.info("Hello");
//     l.info("Hello");
//     l.info("Different Hello");
//
// Will produce:
//       [2019-06-25 17:50:56.511] [logger] [info] Hello
//       [2019-06-25 17:50:56.512] [logger] [info] 3 duplicate messages..
//       [2019-06-25 17:50:56.512] [logger] [info] Different Hello



namespace spdlog {
namespace sinks {
template<typename Mutex>
class dup_filter_sink : public dist_sink<Mutex>
{
public:
    template<class Rep, class Period>
    explicit dup_filter_sink(std::chrono::duration<Rep, Period> max_ignore_duration)
        : max_skip_duration_{max_ignore_duration}
        , last_msg_time_{log_clock::now()}
        , skip_counter_{0}
    {}

protected:
    std::chrono::microseconds max_skip_duration_;
    log_clock::time_point last_msg_time_;
    size_t skip_counter_;
    std::string last_msg_payload_;

    void sink_it_(const details::log_msg &msg) override
    {
        auto msg_time = msg.time;
        auto delta_time = msg_time - last_msg_time_;
        last_msg_time_ = msg_time;

        if (delta_time < max_skip_duration_ && last_msg_payload_ == msg.payload)
        {
            skip_counter_++;
            return;
        }

        // got different message from the previous
        if (skip_counter_ > 0)
        {
            fmt::basic_memory_buffer<char, 80> buf;
            fmt::format_to(buf, "{} duplicate messages..", skip_counter_);
            details::log_msg skipped_msg{msg.logger_name, msg.level, string_view_t{buf.data(), buf.size()}};

            // log the skip notification
            for (auto &sink : dist_sink<Mutex>::sinks_)
            {
                if (sink->should_log(msg.level))
                {
                    sink->log(skipped_msg);
                }
            }
            skip_counter_ = 0;
        }

        last_msg_payload_.assign(msg.payload.data(), msg.payload.data() + msg.payload.size());
        dist_sink<Mutex>::sink_it_(msg);
    }
};

using dup_filter_sink_mt = dup_filter_sink<std::mutex>;
using dup_filter_sink_st = dup_filter_sink<details::null_mutex>;
} // namespace sinks
} // namespace spdlog
