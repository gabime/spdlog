//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

// Very fast asynchronous logger (millions of logs per second on an average desktop)
// Uses pre allocated lockfree queue for maximum throughput even under large number of threads.
// Creates a single back thread to pop messages from the queue and log them.
//
// Upon each log write the logger:
//    1. Checks if its log level is enough to log the message
//    2. Push a new copy of the message to a queue (or block the caller until space is available in the queue)
//    3. will throw spdlog_ex upon log exceptions
// Upon destruction, logs all remaining messages in the queue before destructing..

#include "common.h"
#include "logger.h"

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
namespace spdlog {
namespace details {
class thread_pool;
}

class async_logger SPDLOG_FINAL : public std::enable_shared_from_this<async_logger>, public logger
{
    friend class details::thread_pool;

public:
    template<class It>
    async_logger(const std::string &logger_name, const It &begin, const It &end, std::weak_ptr<details::thread_pool> tp,
        async_overflow_policy overflow_policy = async_overflow_policy::block_retry);

    async_logger(const std::string &logger_name, sinks_init_list sinks, std::weak_ptr<details::thread_pool> tp,
        async_overflow_policy overflow_policy = async_overflow_policy::block_retry);

    async_logger(const std::string &logger_name, sink_ptr single_sink, std::weak_ptr<details::thread_pool> tp,
        async_overflow_policy overflow_policy = async_overflow_policy::block_retry);

protected:
    void _sink_it(details::log_msg &msg) override;
    void _flush() override;

    void _backend_log(details::log_msg &incoming_log_msg);
    void _backend_flush();

private:
    std::weak_ptr<details::thread_pool> _thread_pool;
    async_overflow_policy _overflow_policy;
};
} // namespace spdlog

#include "details/async_logger_impl.h"
