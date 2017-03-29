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

#include <spdlog/common.h>
#include <spdlog/logger.h>

#include <chrono>
#include <functional>
#include <string>
#include <memory>

namespace spdlog
{

namespace details
{
class async_log_helper;
}

class async_logger :public logger
{
public:
    template<class It>
    async_logger(const std::string& name,
                 const It& begin,
                 const It& end,
                 size_t queue_size,
                 const async_overflow_policy overflow_policy =  async_overflow_policy::block_retry,
                 const std::function<void()>& worker_warmup_cb = nullptr,
                 const std::chrono::milliseconds& flush_interval_ms = std::chrono::milliseconds::zero(),
                 const std::function<void()>& worker_teardown_cb = nullptr);

    async_logger(const std::string& logger_name,
                 sinks_init_list sinks,
                 size_t queue_size,
                 const async_overflow_policy overflow_policy = async_overflow_policy::block_retry,
                 const std::function<void()>& worker_warmup_cb = nullptr,
                 const std::chrono::milliseconds& flush_interval_ms = std::chrono::milliseconds::zero(),
                 const std::function<void()>& worker_teardown_cb = nullptr);

    async_logger(const std::string& logger_name,
                 sink_ptr single_sink,
                 size_t queue_size,
                 const async_overflow_policy overflow_policy =  async_overflow_policy::block_retry,
                 const std::function<void()>& worker_warmup_cb = nullptr,
                 const std::chrono::milliseconds& flush_interval_ms = std::chrono::milliseconds::zero(),
                 const std::function<void()>& worker_teardown_cb = nullptr);

    //Wait for the queue to be empty, and flush synchronously
    //Warning: this can potentialy last forever as we wait it to complete
    void flush() override;

    // Error handler
    virtual void set_error_handler(log_err_handler) override;
    virtual log_err_handler error_handler() override;

protected:
    void _sink_it(details::log_msg& msg) override;
    void _set_formatter(spdlog::formatter_ptr msg_formatter) override;
    void _set_pattern(const std::string& pattern) override;

private:
    std::unique_ptr<details::async_log_helper> _async_log_helper;
};
}


#include <spdlog/details/async_logger_impl.h>
