// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/common.h"
#include "spdlog/details/log_msg_buffer.h"
#include "spdlog/details/circular_q.h"
#include "spdlog/sinks/sink.h"
#include <mutex>

// Store log messages in circular buffer.
// Useful for storing debug data in case of error/warning happens.

namespace spdlog {
namespace details {
class backtracer
{
    std::mutex mutex_;
    size_t n_messages_;
    circular_q<log_msg_buffer> messages_;

public:
    explicit backtracer(size_t n_messages)
        : n_messages_{n_messages}
        , messages_{n_messages}
    {}

    backtracer(const backtracer &other)
        : n_messages_{other.n_messages_}
        , messages_{other.messages_}
    {}

    size_t n_messages() const
    {
        return n_messages_;
    }

    void add(const log_msg &msg)
    {
        std::lock_guard<std::mutex> lock{mutex_};
        messages_.push_back(log_msg_buffer{msg});
    }

    // pop all items in the q and apply the give fun on each of them.
    void foreach_pop(std::function<void(const details::log_msg)> fun)
    {
        std::lock_guard<std::mutex> lock{mutex_};
        while (!messages_.empty())
        {
            log_msg_buffer popped;
            messages_.pop_front(popped);
            fun(popped);
        }
    }
};
} // namespace details
} // namespace spdlog