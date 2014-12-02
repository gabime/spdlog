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
#include <chrono>
#include <atomic>

#include "../sinks/sink.h"
#include "../logger.h"
#include "../details/mpcs_q.h"
#include "../details/log_msg.h"
#include "../details/format.h"


namespace spdlog
{
namespace details
{

class async_log_helper
{
    struct async_msg
    {
        std::string logger_name;
        level::level_enum level;
        log_clock::time_point time;
        std::tm tm_time;
        std::string raw_msg_str;

        async_msg() = default;

        async_msg(const details::log_msg& m) :
            logger_name(m.logger_name),
            level(m.level),
            time(m.time),
            tm_time(m.tm_time),
            raw_msg_str(m.raw.data(), m.raw.size())
        {

        }


        log_msg to_log_msg()
        {
            log_msg msg;
            msg.logger_name = logger_name;
            msg.level = level;
            msg.time = time;
            msg.tm_time = tm_time;
            msg.raw << raw_msg_str;
            return msg;
        }
    };

public:

    using q_type = details::mpsc_q < std::unique_ptr<async_msg> >;
    using clock = std::chrono::monotonic_clock;


    explicit async_log_helper(size_t max_queue_size);
    void log(const details::log_msg& msg);

    //Stop logging and join the back thread
    ~async_log_helper();
    void add_sink(sink_ptr sink);
    void remove_sink(sink_ptr sink_ptr);
    void set_formatter(formatter_ptr);
    //Wait to remaining items (if any) in the queue to be written and shutdown
    void shutdown(const log_clock::duration& timeout);





private:
    std::vector<std::shared_ptr<sinks::sink>> _sinks;
    std::atomic<bool> _active;
    q_type _q;
    std::thread _worker_thread;
    std::mutex _mutex;

    // last exception thrown from the worker thread
    std::shared_ptr<spdlog_ex> _last_workerthread_ex;

    // worker thread formatter
    formatter_ptr _formatter;


    // will throw last back thread exception or if worker hread no active
    void _push_sentry();

    // worker thread loop
    void _thread_loop();

    // guess how much to sleep if queue is empty
    static clock::duration spdlog::details::async_log_helper::_calc_pop_sleep(const clock::time_point& last_pop);


    // clear all remaining messages(if any), stop the _worker_thread and join it
    void _join();

};
}
}

///////////////////////////////////////////////////////////////////////////////
// async_sink class implementation
///////////////////////////////////////////////////////////////////////////////
inline spdlog::details::async_log_helper::async_log_helper(size_t max_queue_size)
    :_sinks(),
     _active(true),
     _q(max_queue_size),
     _worker_thread(&async_log_helper::_thread_loop, this)
{}

inline spdlog::details::async_log_helper::~async_log_helper()
{
    _join();
}

inline void spdlog::details::async_log_helper::log(const details::log_msg& msg)
{
    _push_sentry();

    _q.push(std::unique_ptr<async_msg>(new async_msg(msg)));
}

inline void spdlog::details::async_log_helper::_thread_loop()
{

    clock::time_point last_pop = clock::now();
    while (_active)
    {
        q_type::item_type async_msg;

        if (_q.pop(async_msg))
        {

            last_pop = clock::now();

            try
            {
                details::log_msg log_msg = async_msg->to_log_msg();

                _formatter->format(log_msg);
                for (auto &s : _sinks)
                    s->log(log_msg);

            }
            catch (const std::exception& ex)
            {
                _last_workerthread_ex = std::make_shared<spdlog_ex>(ex.what());
            }
            catch (...)
            {
                _last_workerthread_ex = std::make_shared<spdlog_ex>("Unknown exception");
            }
        }
        //Sleep for a while if empty.
        else
        {
            std::this_thread::sleep_for(_calc_pop_sleep(last_pop));
        }
    }
}

//Try to guess sleep duration according to the time passed since last message
inline spdlog::details::async_log_helper::clock::duration spdlog::details::async_log_helper::_calc_pop_sleep(const clock::time_point& last_pop)
{
    using std::chrono::milliseconds;
    using std::chrono::microseconds;
    auto time_since_pop = clock::now() - last_pop;


    if (time_since_pop > milliseconds(1000))
        return milliseconds(500);
    if (time_since_pop > microseconds(0))
        return(time_since_pop / 2);
    return microseconds(0);


}

inline void spdlog::details::async_log_helper::add_sink(spdlog::sink_ptr s)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _sinks.push_back(s);
}


inline void spdlog::details::async_log_helper::remove_sink(spdlog::sink_ptr s)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _sinks.erase(std::remove(_sinks.begin(), _sinks.end(), s), _sinks.end());
}


inline void spdlog::details::async_log_helper::set_formatter(formatter_ptr msg_formatter)
{
    _formatter = msg_formatter;
}



inline void spdlog::details::async_log_helper::shutdown(const log_clock::duration& timeout)
{
    if (timeout > std::chrono::milliseconds::zero())
    {
        auto until = log_clock::now() + timeout;
        while (_q.approx_size() > 0 && log_clock::now() < until)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    _join();
}


inline void spdlog::details::async_log_helper::_push_sentry()
{
    if (_last_workerthread_ex)
    {
        auto ex = std::move(_last_workerthread_ex);
        _last_workerthread_ex.reset();
        throw *ex;
    }
    if (!_active)
        throw(spdlog_ex("async_sink not active"));
}


inline void spdlog::details::async_log_helper::_join()
{
    _active = false;
    if (_worker_thread.joinable())
    {
        try
        {
            _worker_thread.join();
        }
        catch (const std::system_error&) //Dont crash if thread not joinable
        {
        }
    }

}

