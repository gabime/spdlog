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
// If the back thread throws during logging, a spdlog::spdlog_ex exception
// will be thrown in client's thread when tries to log the next message

#pragma once

#include <chrono>
#include <thread>
#include <functional>

#include "../common.h"
#include "../sinks/sink.h"
#include "./mpmc_bounded_q.h"
#include "./log_msg.h"
#include "./format.h"
#include "./os.h"


namespace spdlog
{
namespace details
{

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
        std::string logger_name;
        level::level_enum level;
        log_clock::time_point time;
        size_t thread_id;
        std::string txt;
        async_msg_type msg_type;

        async_msg() = default;
        ~async_msg() = default;

async_msg(async_msg&& other) SPDLOG_NOEXCEPT:
        logger_name(std::move(other.logger_name)),
                    level(std::move(other.level)),
                    time(std::move(other.time)),
                    txt(std::move(other.txt)),
                    msg_type(std::move(other.msg_type))
        {}

        async_msg(async_msg_type m_type) :msg_type(m_type)
        {};

        async_msg& operator=(async_msg&& other) SPDLOG_NOEXCEPT
        {
            logger_name = std::move(other.logger_name);
            level = other.level;
            time = std::move(other.time);
            thread_id = other.thread_id;
            txt = std::move(other.txt);
            msg_type = other.msg_type;
            return *this;
        }
        // never copy or assign. should only be moved..
        async_msg(const async_msg&) = delete;
        async_msg& operator=(async_msg& other) = delete;

        // construct from log_msg
        async_msg(const details::log_msg& m) :
            logger_name(m.logger_name),
            level(m.level),
            time(m.time),
            thread_id(m.thread_id),
            txt(m.raw.data(), m.raw.size()),
            msg_type(async_msg_type::log)
        {}



        // copy into log_msg
        void fill_log_msg(log_msg &msg)
        {
            msg.clear();
            msg.logger_name = logger_name;
            msg.level = level;
            msg.time = time;
            msg.thread_id = thread_id;
            msg.raw << txt;
        }
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

    void flush();


private:
    formatter_ptr _formatter;
    std::vector<std::shared_ptr<sinks::sink>> _sinks;

    // queue of messages to log
    q_type _q;

    bool _flush_requested;

    bool _terminate_requested;


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

    void push_msg(async_msg&& new_msg);
    // throw last worker thread exception or if worker thread is not active

    void throw_if_bad_worker();

    // worker thread main loop
    void worker_loop();

    // pop next message from the queue and process it. will set the last_pop to the pop time
    // return false if termination of the queue is required
    bool process_next_msg(log_clock::time_point& last_pop, log_clock::time_point& last_flush);

    void handle_flush_interval(log_clock::time_point& now, log_clock::time_point& last_flush);

    // sleep,yield or return immediatly using the time passed since last message as a hint
    static void sleep_or_yield(const spdlog::log_clock::time_point& now, const log_clock::time_point& last_op_time);

};
}
}

///////////////////////////////////////////////////////////////////////////////
// async_sink class implementation
///////////////////////////////////////////////////////////////////////////////
inline spdlog::details::async_log_helper::async_log_helper(
    formatter_ptr formatter,
    const std::vector<sink_ptr>& sinks,
    size_t queue_size,
    const async_overflow_policy overflow_policy,
    const std::function<void()>& worker_warmup_cb,
    const std::chrono::milliseconds& flush_interval_ms):
    _formatter(formatter),
    _sinks(sinks),
    _q(queue_size),
    _flush_requested(false),
    _terminate_requested(false),
    _overflow_policy(overflow_policy),
    _worker_warmup_cb(worker_warmup_cb),
    _flush_interval_ms(flush_interval_ms),
    _worker_thread(&async_log_helper::worker_loop, this)
{}

// Send to the worker thread termination message(level=off)
// and wait for it to finish gracefully
inline spdlog::details::async_log_helper::~async_log_helper()
{
    try
    {
        push_msg(async_msg(async_msg_type::terminate));
        _worker_thread.join();
    }
    catch (...) // don't crash in destructor
    {}
}


//Try to push and block until succeeded
inline void spdlog::details::async_log_helper::log(const details::log_msg& msg)
{
    push_msg(async_msg(msg));


}

//Try to push and block until succeeded
inline void spdlog::details::async_log_helper::push_msg(details::async_log_helper::async_msg&& new_msg)
{
    throw_if_bad_worker();
    if (!_q.enqueue(std::move(new_msg)) && _overflow_policy != async_overflow_policy::discard_log_msg)
    {
        auto last_op_time = details::os::now();
        auto now = last_op_time;
        do
        {
            now = details::os::now();
            sleep_or_yield(now, last_op_time);
        }
        while (!_q.enqueue(std::move(new_msg)));
    }

}

inline void spdlog::details::async_log_helper::flush()
{
    push_msg(async_msg(async_msg_type::flush));
}

inline void spdlog::details::async_log_helper::worker_loop()
{
    try
    {
        if (_worker_warmup_cb) _worker_warmup_cb();
        auto last_pop = details::os::now();
        auto last_flush = last_pop;
        while(process_next_msg(last_pop, last_flush));
    }
    catch (const std::exception& ex)
    {
        _last_workerthread_ex = std::make_shared<spdlog_ex>(std::string("async_logger worker thread exception: ") + ex.what());
    }
    catch (...)
    {
        _last_workerthread_ex = std::make_shared<spdlog_ex>("async_logger worker thread exception");
    }
}

// process next message in the queue
// return true if this thread should still be active (no msg with level::off was received)
inline bool spdlog::details::async_log_helper::process_next_msg(log_clock::time_point& last_pop, log_clock::time_point& last_flush)
{

    async_msg incoming_async_msg;
    log_msg incoming_log_msg;

    if (_q.dequeue(incoming_async_msg))
    {
        last_pop = details::os::now();
        switch (incoming_async_msg.msg_type)
        {
        case async_msg_type::flush:
            _flush_requested = true;
            break;

        case async_msg_type::terminate:
            _flush_requested = true;
            _terminate_requested = true;
            break;

        default:
            incoming_async_msg.fill_log_msg(incoming_log_msg);
            _formatter->format(incoming_log_msg);
            for (auto &s : _sinks)
                s->log(incoming_log_msg);
        }
        return true;
    }

    // Handle empty queue..
    // This is the only place where the queue can terminate or flush to avoid losing messages already in the queue
    else
    {
        auto now = details::os::now();
        handle_flush_interval(now, last_flush);
        sleep_or_yield(now, last_pop);
        return !_terminate_requested;

    }
}

inline void spdlog::details::async_log_helper::handle_flush_interval(log_clock::time_point& now, log_clock::time_point& last_flush)
{
    auto should_flush = _flush_requested || (_flush_interval_ms != std::chrono::milliseconds::zero() && now - last_flush >= _flush_interval_ms);
    if (should_flush)
    {
        for (auto &s : _sinks)
            s->flush();
        now = last_flush = details::os::now();
        _flush_requested = false;
    }
}
inline void spdlog::details::async_log_helper::set_formatter(formatter_ptr msg_formatter)
{
    _formatter = msg_formatter;
}


// sleep,yield or return immediatly using the time passed since last message as a hint
inline void spdlog::details::async_log_helper::sleep_or_yield(const spdlog::log_clock::time_point& now, const spdlog::log_clock::time_point& last_op_time)
{
    using std::chrono::milliseconds;
    using namespace std::this_thread;

    auto time_since_op = now - last_op_time;

    // spin upto 1 ms
    if (time_since_op <= milliseconds(1))
        return;

    // yield upto 10ms
    if (time_since_op <= milliseconds(10))
        return yield();


    // sleep for half of duration since last op
    if (time_since_op <= milliseconds(100))
        return sleep_for(time_since_op / 2);

    return sleep_for(milliseconds(100));
}

// throw if the worker thread threw an exception or not active
inline void spdlog::details::async_log_helper::throw_if_bad_worker()
{
    if (_last_workerthread_ex)
    {
        auto ex = std::move(_last_workerthread_ex);
        throw *ex;
    }
}







