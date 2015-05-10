/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2014 Gabi Melman.                                       */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

// Very fast asynchronous logger (millions of logs per second on an average desktop)
// Uses pre allocated lockfree queue for maximum throughput even under large number of threads.
// Creates a single back thread to pop messages from the queue and log them.
//
// Upon each log write the logger:
//    1. Checks if its log level is enough to log the message
//    2. Push a new copy of the message to a queue (or block the caller until space is available in the queue)
//    3. will throw spdlog_ex upon log exceptions
// Upong destruction, logs all remaining messages in the queue before destructing..

#include <chrono>
#include <functional>
#include "common.h"
#include "logger.h"
#include "spdlog.h"


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
                 const std::chrono::milliseconds& flush_interval_ms = std::chrono::milliseconds::zero());

    async_logger(const std::string& logger_name,
                 sinks_init_list sinks,
                 size_t queue_size,
                 const async_overflow_policy overflow_policy = async_overflow_policy::block_retry,
                 const std::function<void()>& worker_warmup_cb = nullptr,
                 const std::chrono::milliseconds& flush_interval_ms = std::chrono::milliseconds::zero());

    async_logger(const std::string& logger_name,
                 sink_ptr single_sink,
                 size_t queue_size,
                 const async_overflow_policy overflow_policy =  async_overflow_policy::block_retry,
                 const std::function<void()>& worker_warmup_cb = nullptr,
                 const std::chrono::milliseconds& flush_interval_ms = std::chrono::milliseconds::zero());


protected:
    void _log_msg(details::log_msg& msg) override;
    void _set_formatter(spdlog::formatter_ptr msg_formatter) override;
    void _set_pattern(const std::string& pattern) override;

private:
    std::unique_ptr<details::async_log_helper> _async_log_helper;
};
}


#include "./details/async_logger_impl.h"
