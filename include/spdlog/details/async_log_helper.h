//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

// async log helper :
// Process logs asynchronously using a back thread.
//
// If the internal queue of log messages reaches its max size,
// then the client call will block until there is more room.
//

#pragma once

#include "../common.h"
#include "../details/log_msg.h"
#include "../details/mpmc_blocking_q.h"
#include "../details/os.h"
#include "../formatter.h"
#include "../sinks/sink.h"

#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace spdlog {
namespace details {

class async_log_helper
{
    // Async msg to move to/from the queue
    // Movable only. should never be copied
    enum class async_msg_type
    {
        log,
        flush,
        terminate
    };

    struct async_msg
    {
        level::level_enum level;
        log_clock::time_point time;
        size_t thread_id;
        std::string txt;
        async_msg_type msg_type;
        size_t msg_id;

        async_msg() = default;
        ~async_msg() = default;

        explicit async_msg(async_msg_type m_type)
            : level(level::info)
            , thread_id(0)
            , msg_type(m_type)
            , msg_id(0)
        {
        }

        async_msg(async_msg &&other) = default;
        async_msg &operator=(async_msg &&other) = default;

        // never copy or assign. should only be moved..
        async_msg(const async_msg &) = delete;
        async_msg &operator=(const async_msg &other) = delete;

        // construct from log_msg
        explicit async_msg(const details::log_msg &m)
            : level(m.level)
            , time(m.time)
            , thread_id(m.thread_id)
            , txt(m.raw.data(), m.raw.size())
            , msg_type(async_msg_type::log)
            , msg_id(m.msg_id)
        {
        }

        // copy into log_msg
        void fill_log_msg(log_msg &msg, std::string *logger_name)
        {
            msg.logger_name = logger_name;
            msg.level = level;
            msg.time = time;
            msg.thread_id = thread_id;
            msg.raw.clear();
            msg.raw << txt;
            msg.msg_id = msg_id;
        }
    };

public:
    using item_type = async_msg;
    using q_type = details::mpmc_bounded_queue<item_type>;

    using clock = std::chrono::steady_clock;

    async_log_helper(std::string logger_name, formatter_ptr formatter, std::vector<sink_ptr> sinks, size_t queue_size,
        const log_err_handler err_handler, const async_overflow_policy overflow_policy = async_overflow_policy::block_retry,
        std::function<void()> worker_warmup_cb = nullptr,
        const std::chrono::milliseconds &flush_interval_ms = std::chrono::milliseconds::zero(),
        std::function<void()> worker_teardown_cb = nullptr);

    void log(const details::log_msg &msg);

    // stop logging and join the back thread
    ~async_log_helper();

    async_log_helper(const async_log_helper &) = delete;
    async_log_helper &operator=(const async_log_helper &) = delete;

    void set_formatter(formatter_ptr msg_formatter);

    void flush();

    void set_error_handler(spdlog::log_err_handler err_handler);

private:
    std::string _logger_name;
    formatter_ptr _formatter;
    std::vector<std::shared_ptr<sinks::sink>> _sinks;

    // queue of messages to log
    q_type _q;

    log_err_handler _err_handler;

    std::chrono::time_point<log_clock> _last_flush;

    // overflow policy
    const async_overflow_policy _overflow_policy;

    // worker thread warmup callback - one can set thread priority, affinity, etc
    const std::function<void()> _worker_warmup_cb;

    // auto periodic sink flush parameter
    const std::chrono::milliseconds _flush_interval_ms;

    // worker thread teardown callback
    const std::function<void()> _worker_teardown_cb;

    std::mutex null_mutex_;
    // null_mutex null_mutex_;
    std::condition_variable_any not_empty_cv_;
    std::condition_variable_any not_full_cv_;

    // worker thread
    std::thread _worker_thread;

    void enqueue_msg(async_msg &&new_msg, async_overflow_policy policy);

    // worker thread main loop
    void worker_loop();

    // dequeue next message from the queue and process it.
    // return false if termination of the queue is required
    bool process_next_msg();

    void handle_flush_interval();

    void flush_sinks();
};
} // namespace details
} // namespace spdlog

///////////////////////////////////////////////////////////////////////////////
// async_sink class implementation
///////////////////////////////////////////////////////////////////////////////
inline spdlog::details::async_log_helper::async_log_helper(std::string logger_name, formatter_ptr formatter, std::vector<sink_ptr> sinks,
    size_t queue_size, log_err_handler err_handler, const async_overflow_policy overflow_policy, std::function<void()> worker_warmup_cb,
    const std::chrono::milliseconds &flush_interval_ms, std::function<void()> worker_teardown_cb)
    : _logger_name(std::move(logger_name))
    , _formatter(std::move(formatter))
    , _sinks(std::move(sinks))
    , _q(queue_size)
    , _err_handler(std::move(err_handler))
    , _last_flush(os::now())
    , _overflow_policy(overflow_policy)
    , _worker_warmup_cb(std::move(worker_warmup_cb))
    , _flush_interval_ms(flush_interval_ms)
    , _worker_teardown_cb(std::move(worker_teardown_cb))
{
    _worker_thread = std::thread(&async_log_helper::worker_loop, this);
}

// send to the worker thread terminate message, and join it.
inline spdlog::details::async_log_helper::~async_log_helper()
{
    try
    {
        enqueue_msg(async_msg(async_msg_type::terminate), async_overflow_policy::block_retry);
        _worker_thread.join();
    }
    catch (...) // don't crash in destructor
    {
    }
}

// try to push and block until succeeded (if the policy is not to discard when the queue is full)
inline void spdlog::details::async_log_helper::log(const details::log_msg &msg)
{
    enqueue_msg(async_msg(msg), _overflow_policy);
}

inline void spdlog::details::async_log_helper::enqueue_msg(details::async_log_helper::async_msg &&new_msg, async_overflow_policy policy)
{

    // block until succeeded pushing to the queue
    if (policy == async_overflow_policy::block_retry)
    {
        _q.enqueue(std::move(new_msg));
    }
    else
    {
        _q.enqueue_nowait(std::move(new_msg));
    }
}

// optionally wait for the queue be empty and request flush from the sinks
inline void spdlog::details::async_log_helper::flush()
{
    enqueue_msg(async_msg(async_msg_type::flush), _overflow_policy);
}

inline void spdlog::details::async_log_helper::worker_loop()
{
    if (_worker_warmup_cb)
    {
        _worker_warmup_cb();
    }
    auto active = true;
    while (active)
    {
        try
        {
            active = process_next_msg();
        }
        SPDLOG_CATCH_AND_HANDLE
    }
    if (_worker_teardown_cb)
    {
        _worker_teardown_cb();
    }
}

// process next message in the queue
// return true if this thread should still be active (while no terminate msg was received)
inline bool spdlog::details::async_log_helper::process_next_msg()
{
    async_msg incoming_async_msg;
    bool dequeued = _q.dequeue_for(incoming_async_msg, std::chrono::seconds(2));
    if (!dequeued)
    {
        handle_flush_interval();
        return true;
    }

    switch (incoming_async_msg.msg_type)
    {
    case async_msg_type::flush:
        flush_sinks();
        return true;

    case async_msg_type::terminate:
        flush_sinks();
        return false;

    default:
        log_msg incoming_log_msg;
        incoming_async_msg.fill_log_msg(incoming_log_msg, &_logger_name);
        _formatter->format(incoming_log_msg);
        for (auto &s : _sinks)
        {
            if (s->should_log(incoming_log_msg.level))
            {
                try
                {
                    s->log(incoming_log_msg);
                }
                SPDLOG_CATCH_AND_HANDLE
            }
        }
        handle_flush_interval();
        return true;
    }
    assert(false);
    return true; // should not be reached
}

inline void spdlog::details::async_log_helper::set_formatter(formatter_ptr msg_formatter)
{
    _formatter = std::move(msg_formatter);
}

inline void spdlog::details::async_log_helper::set_error_handler(spdlog::log_err_handler err_handler)
{
    _err_handler = std::move(err_handler);
}

// flush all sinks if _flush_interval_ms has expired.
inline void spdlog::details::async_log_helper::handle_flush_interval()
{
    if (_flush_interval_ms == std::chrono::milliseconds::zero())
    {
        return;
    }
    auto delta = details::os::now() - _last_flush;
    ;
    if (delta >= _flush_interval_ms)
    {
        flush_sinks();
    }
}

// flush all sinks if _flush_interval_ms has expired. only called if queue is empty
inline void spdlog::details::async_log_helper::flush_sinks()
{

    for (auto &s : _sinks)
    {
        try
        {
            s->flush();
        }
        SPDLOG_CATCH_AND_HANDLE
    }
    _last_flush = os::now();
}
