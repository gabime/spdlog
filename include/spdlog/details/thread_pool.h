#pragma once

#include "spdlog/details/log_msg.h"
#include "spdlog/details/mpmc_blocking_q.h"
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
    fmt::basic_memory_buffer<char, 176> raw;

    size_t msg_id;
    async_logger_ptr worker_ptr;

    async_msg() = default;
    ~async_msg() = default;

    // should only be moved in or out of the queue..
    async_msg(const async_msg &) = delete;

    // support for vs2013 move
#if defined(_MSC_VER) && _MSC_VER <= 1800
    async_msg(async_msg &&other) SPDLOG_NOEXCEPT : msg_type(other.msg_type),
                                                   level(other.level),
                                                   time(other.time),
                                                   thread_id(other.thread_id),
                                                   raw(move(other.raw)),
                                                   msg_id(other.msg_id),
                                                   worker_ptr(std::move(other.worker_ptr))
    {
    }

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
#else // (_MSC_VER) && _MSC_VER <= 1800
    async_msg(async_msg &&other) = default;
    async_msg &operator=(async_msg &&other) = default;
#endif

    // construct from log_msg with given type
    async_msg(async_logger_ptr &&worker, async_msg_type the_type, details::log_msg &&m)
        : msg_type(the_type)
        , level(m.level)
        , time(m.time)
        , thread_id(m.thread_id)
        , msg_id(m.msg_id)
        , worker_ptr(std::forward<async_logger_ptr>(worker))
    {
        fmt_helper::append_buf(m.raw, raw);
    }

    async_msg(async_logger_ptr &&worker, async_msg_type the_type)
        : async_msg(std::forward<async_logger_ptr>(worker), the_type, details::log_msg())
    {
    }

    async_msg(async_msg_type the_type)
        : async_msg(nullptr, the_type, details::log_msg())
    {
    }

    // copy into log_msg
    void to_log_msg(log_msg &msg)
    {
        msg.logger_name = &worker_ptr->name();
        msg.level = level;
        msg.time = time;
        msg.thread_id = thread_id;
        fmt_helper::append_buf(raw, msg.raw);
        msg.msg_id = msg_id;
        msg.color_range_start = 0;
        msg.color_range_end = 0;
    }
};

class thread_pool
{
public:
    using item_type = async_msg;
    using q_type = details::mpmc_blocking_queue<item_type>;
    using clock_type = std::chrono::steady_clock;

    thread_pool(size_t q_max_items, size_t threads_n)
        : q_(q_max_items)
    {
        // std::cout << "thread_pool()  q_size_bytes: " << q_size_bytes << "\tthreads_n: " << threads_n << std::endl;
        if (threads_n == 0 || threads_n > 1000)
        {
            throw spdlog_ex("spdlog::thread_pool(): invalid threads_n param (valid range is 1-1000)");
        }
        for (size_t i = 0; i < threads_n; i++)
        {
            threads_.emplace_back(std::bind(&thread_pool::worker_loop_, this));
        }
    }

    // message all threads to terminate gracefully join them
    ~thread_pool()
    {
        try
        {
            for (size_t i = 0; i < threads_.size(); i++)
            {
                post_async_msg_(async_msg(async_msg_type::terminate), async_overflow_policy::block);
            }

            for (auto &t : threads_)
            {
                t.join();
            }
        }
        catch (...)
        {
        }
    }

    void post_log(async_logger_ptr &&worker_ptr, details::log_msg &&msg, async_overflow_policy overflow_policy)
    {
        async_msg async_m(std::forward<async_logger_ptr>(worker_ptr), async_msg_type::log, std::forward<log_msg>(msg));
        post_async_msg_(std::move(async_m), overflow_policy);
    }

    void post_flush(async_logger_ptr &&worker_ptr, async_overflow_policy overflow_policy)
    {
        post_async_msg_(async_msg(std::move(worker_ptr), async_msg_type::flush), overflow_policy);
    }

private:
    q_type q_;

    std::vector<std::thread> threads_;

    void post_async_msg_(async_msg &&new_msg, async_overflow_policy overflow_policy)
    {
        if (overflow_policy == async_overflow_policy::block)
        {
            q_.enqueue(std::move(new_msg));
        }
        else
        {
            q_.enqueue_nowait(std::move(new_msg));
        }
    }

    void worker_loop_()
    {
        while (process_next_msg_()) {};
    }

    // process next message in the queue
    // return true if this thread should still be active (while no terminate msg was received)
    bool process_next_msg_()
    {
        async_msg incoming_async_msg;
        bool dequeued = q_.dequeue_for(incoming_async_msg, std::chrono::seconds(10));
        if (!dequeued)
        {
            return true;
        }

        switch (incoming_async_msg.msg_type)
        {
        case async_msg_type::flush:
        {
            incoming_async_msg.worker_ptr->backend_flush_();
            return true;
        }

        case async_msg_type::terminate:
        {
            return false;
        }

        default:
        {
            log_msg msg;
            incoming_async_msg.to_log_msg(msg);
            incoming_async_msg.worker_ptr->backend_log_(msg);
            return true;
        }
        }
        assert(false);
        return true; // should not be reached
    }
};

} // namespace details
} // namespace spdlog