#pragma once

#include "spdlog/details/log_msg.h"
#include "spdlog/details/mpmc_bounded_q.h"
#include "spdlog/details/os.h"

#include <chrono>
#include <memory>
#include <thread>
#include <vector>

namespace spdlog {
namespace details {

using async_logger_ptr = std::shared_ptr<spdlog::async_logger>;

enum class async_msg_type
{
    log,
    flush,
    terminate
};

// Async msg to move to/from the queue
// Movable only. should never be copied
struct async_msg
{
    async_msg_type msg_type;
    level::level_enum level;
    log_clock::time_point time;
    size_t thread_id;
    fmt::MemoryWriter raw;

    size_t msg_id;
    async_logger_ptr worker_ptr;

    async_msg() = default;
    ~async_msg() = default;

    // never copy or assign. should only be move assigned in to the queue..
    async_msg(const async_msg &) = delete;
    async_msg &operator=(const async_msg &other) = delete;
    async_msg(async_msg &&other) = delete;

    // construct from log_msg with given type
    async_msg(async_logger_ptr &&worker, async_msg_type the_type, details::log_msg &&m)
        : msg_type(the_type)
        , level(m.level)
        , time(m.time)
        , thread_id(m.thread_id)
        , raw(std::move(m.raw))
        , msg_id(m.msg_id)
        , worker_ptr(std::forward<async_logger_ptr>(worker))
    {
    }

    async_msg(async_logger_ptr &&worker, async_msg_type the_type)
        : async_msg(std::forward<async_logger_ptr>(worker), the_type, details::log_msg())
    {
    }

    async_msg(async_msg_type the_type)
        : async_msg(nullptr, the_type, details::log_msg())
    {
    }

    // used to move to the message queue
    async_msg &operator=(async_msg &&other) SPDLOG_NOEXCEPT
    {
        msg_type = other.msg_type;
        level = other.level;
        time = other.time;
        thread_id = other.thread_id;
        raw = std::move(other.raw);
        msg_id = other.msg_id;
        worker_ptr = std::move(other.worker_ptr);
        return *this;
    }

    // copy into log_msg
    void to_log_msg(log_msg &&msg)
    {
        msg.logger_name = &worker_ptr->name();
        msg.level = level;
        msg.time = time;
        msg.thread_id = thread_id;
        msg.raw = std::move(raw);
        msg.formatted.clear();
        msg.msg_id = msg_id;
        msg.color_range_start = 0;
        msg.color_range_end = 0;
    }
};

class thread_pool
{
public:
    using item_type = async_msg;
    using q_type = details::mpmc_bounded_queue<item_type>;
    using clock_type = std::chrono::steady_clock;

    thread_pool(size_t q_size_bytes, size_t threads_n)
        : _msg_counter(0)
        , _q(q_size_bytes)
    {
        // std::cout << "thread_pool()  q_size_bytes: " << q_size_bytes << "\tthreads_n: " << threads_n << std::endl;
        if (threads_n == 0 || threads_n > 1000)
        {
            throw spdlog_ex("spdlog::thread_pool(): invalid threads_n param (valid range is 1-1000)");
        }
        for (size_t i = 0; i < threads_n; i++)
        {
            _threads.emplace_back(std::bind(&thread_pool::_worker_loop, this));
        }
    }

    // message all threads to terminate gracefully join them
    ~thread_pool()
    {
        try
        {
            for (size_t i = 0; i < _threads.size(); i++)
            {
                _post_async_msg(async_msg(async_msg_type::terminate), async_overflow_policy::block_retry);
            }

            for (auto &t : _threads)
            {
                t.join();
            }
            // std::cout << "~thread_pool()  _msg_counter: " << _msg_counter << std::endl;
        }
        catch (...)
        {
        }
    }

    void post_log(async_logger_ptr &&worker_ptr, details::log_msg &&msg, async_overflow_policy overflow_policy)
    {
        async_msg as_m(std::forward<async_logger_ptr>(worker_ptr), async_msg_type::log, std::forward<log_msg>(msg));
        _post_async_msg(std::move(as_m), overflow_policy);
    }

    void post_flush(async_logger_ptr &&worker_ptr, async_overflow_policy overflow_policy)
    {
        _post_async_msg(async_msg(std::forward<async_logger_ptr>(worker_ptr), async_msg_type::flush), overflow_policy);
    }

    size_t msg_counter()
    {
        return _msg_counter.load(std::memory_order_relaxed);
    }

private:
    std::atomic<size_t> _msg_counter; // total # of messages processed in this pool
    q_type _q;

    std::vector<std::thread> _threads;

    void _post_async_msg(async_msg &&new_msg, async_overflow_policy overflow_policy)
    {

        if (!_q.enqueue(std::forward<async_msg>(new_msg)) && overflow_policy == async_overflow_policy::block_retry)
        {
            auto last_op_time = clock_type::now();
            auto now = last_op_time;
            do
            {
                now = clock_type::now();
                sleep_or_yield(now, last_op_time);
            } while (!_q.enqueue(std::move(new_msg)));
        }
    }

    // pop log messages from the queue and send to the logger worker
    void _worker_loop()
    {
        async_msg popped_async_msg;
        log_msg msg;
        bool active = true;
        auto last_pop_time = clock_type::now();
        while (active)
        {
            if (_q.dequeue(popped_async_msg))
            {
                last_pop_time = clock_type::now();
                switch (popped_async_msg.msg_type)
                {
                case async_msg_type::flush:
                {
                    auto worker = std::move(popped_async_msg.worker_ptr);
                    worker->_backend_flush();
                    break;
                }

                case async_msg_type::terminate:
                    active = false;
                    break;

                default:
                {
                    popped_async_msg.to_log_msg(std::move(msg));
                    auto worker = std::move(popped_async_msg.worker_ptr);
                    worker->_backend_log(msg);
                    _msg_counter.fetch_add(1, std::memory_order_relaxed);
                }
                }
            }
            else // queue is empty - the only place we can terminate the thread if needed.
            {
                sleep_or_yield(clock_type::now(), last_pop_time);
            }
        }
    }

    // spin, yield or sleep. use the time passed since last message as a hint
    static void sleep_or_yield(const clock_type::time_point &now, const clock_type::time_point &last_op_time)
    {
        using std::chrono::microseconds;
        using std::chrono::milliseconds;

        auto time_since_op = now - last_op_time;

        // yield upto 150 micros
        if (time_since_op <= microseconds(150))
        {
            return std::this_thread::yield();
        }

        // sleep for 20 ms upto 200 ms
        if (time_since_op <= milliseconds(200))
        {
            return details::os::sleep_for_millis(20);
        }

        // sleep for 500 ms
        return details::os::sleep_for_millis(500);
    }
};

} // namespace details
} // namespace spdlog
