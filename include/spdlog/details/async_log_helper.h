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

// async log helper :
// Process logs asynchronously using a back thread.
//
// If the internal queue of log messages reaches its max size,
// then the client call will block until there is more room.
//
// If the back thread throws during logging, a spdlog::spdlog_ex exception
// will be thrown in client's thread when tries to log the next message

#pragma once
#include <thread>
#include <vector>

#include "../common.h"
#include "./mpmc_bounded_q.h"

namespace spdlog
{
namespace details
{
class log_msg;

class async_log_helper
{
    // Async msg to move to/from the queue
    // Movable only. should never be copied
    struct async_msg
    {
        std::string logger_name;
        level::level_enum level;
        log_clock::time_point time;
        size_t thread_id;
        std::string txt;

        async_msg() = default;
        ~async_msg() = default;

        async_msg(async_msg&& other) SPDLOG_NOEXCEPT;

        async_msg& operator=(async_msg&& other) SPDLOG_NOEXCEPT;
        // never copy or assign. should only be moved..
        async_msg(const async_msg&) = delete;
        async_msg& operator=(async_msg& other) = delete;

        // construct from log_msg
        async_msg(const details::log_msg& m);


        // copy into log_msg
        void fill_log_msg(log_msg &msg);
    };

public:

    using item_type = async_msg;
    using q_type = details::mpmc_bounded_queue<item_type>;

    using clock = std::chrono::steady_clock;


    async_log_helper(formatter_ptr formatter,
                     const std::vector<sink_ptr>& sinks,
                     size_t queue_size,
                     const async_overflow_policy overflow_policy = async_overflow_policy::block_retry,
                     const std::function<void()>& worker_warmup_cb = nullptr,
                     const std::chrono::milliseconds& flush_interval_ms = std::chrono::milliseconds::zero());

    void log(const details::log_msg& msg);

    // stop logging and join the back thread
    ~async_log_helper();

    void set_formatter(formatter_ptr);


private:
    formatter_ptr _formatter;
    std::vector<std::shared_ptr<sinks::sink>> _sinks;

    // queue of messages to log
    q_type _q;

    // last exception thrown from the worker thread
    std::shared_ptr<spdlog_ex> _last_workerthread_ex;

    // overflow policy
    const async_overflow_policy _overflow_policy;

    // worker thread warmup callback - one can set thread priority, affinity, etc
    const std::function<void()> _worker_warmup_cb;

    // auto periodic sink flush parameter
    const std::chrono::milliseconds _flush_interval_ms;

    // worker thread
    std::thread _worker_thread;

    // throw last worker thread exception or if worker thread is not active
    void throw_if_bad_worker();

    // worker thread main loop
    void worker_loop();

    // pop next message from the queue and process it
    // return true if a message was available (queue was not empty), will set the last_pop to the pop time
    bool process_next_msg(log_clock::time_point& last_pop, log_clock::time_point& last_flush);

    void handle_flush_interval(log_clock::time_point& now, log_clock::time_point& last_flush);

    // sleep,yield or return immediatly using the time passed since last message as a hint
    static void sleep_or_yield(const spdlog::log_clock::time_point& now, const log_clock::time_point& last_op_time);



};
}
}

#ifndef SPDLOG_LIBRARY
#include "./async_log_helper.cc"
#endif
