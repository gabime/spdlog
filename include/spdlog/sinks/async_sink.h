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

// async sink:
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
#include <algorithm>

#include "./base_sink.h"
#include "../logger.h"
#include "../details/blocking_queue.h"
#include "../details/null_mutex.h"
#include "../details/log_msg.h"

#include<iostream>

namespace spdlog
{
namespace sinks
{

class async_sink : public base_sink < details::null_mutex >
{
public:
    using q_type = details::blocking_queue < details::log_msg > ;

    explicit async_sink(const q_type::size_type max_queue_size);

    //Stop logging and join the back thread
    ~async_sink();
    void add_sink(sink_ptr sink);
    void remove_sink(sink_ptr sink_ptr);
    q_type& q();
    //Wait to remaining items (if any) in the queue to be written and shutdown
    void shutdown(const std::chrono::milliseconds& timeout);



protected:
    void _sink_it(const details::log_msg& msg) override;
    void _thread_loop();

private:
    std::vector<std::shared_ptr<sink>> _sinks;
    std::atomic<bool> _active;
    q_type _q;
    std::thread _back_thread;
    std::mutex _mutex;
    //Last exception thrown from the back thread
    std::shared_ptr<spdlog_ex> _last_backthread_ex;


    //will throw last back thread exception or if backthread no active
    void _push_sentry();

    //Clear all remaining messages(if any), stop the _back_thread and join it
    void _join();

};
}
}

///////////////////////////////////////////////////////////////////////////////
// async_sink class implementation
///////////////////////////////////////////////////////////////////////////////
inline spdlog::sinks::async_sink::async_sink(const q_type::size_type max_queue_size)
    :_sinks(),
     _active(true),
     _q(max_queue_size),
     _back_thread(&async_sink::_thread_loop, this)
{}

inline spdlog::sinks::async_sink::~async_sink()
{
    _join();
}

inline void spdlog::sinks::async_sink::_sink_it(const details::log_msg& msg)
{
    _push_sentry();
    _q.push(msg);
}


inline void spdlog::sinks::async_sink::_thread_loop()
{
    std::chrono::seconds  pop_timeout { 1 };

    while (_active)
    {
        q_type::item_type msg;
        if (_q.pop(msg, pop_timeout))
        {
            if (!_active)
                return;
            for (auto &s : _sinks)
            {
                try
                {
                    s->log(msg);
                }

                catch (const std::exception& ex)
                {
                    _last_backthread_ex = std::make_shared<spdlog_ex>(ex.what());
                }
                catch (...)
                {
                    _last_backthread_ex = std::make_shared<spdlog_ex>("Unknown exception");
                }

            }
        }
    }
}

inline void spdlog::sinks::async_sink::add_sink(spdlog::sink_ptr s)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _sinks.push_back(s);
}


inline void spdlog::sinks::async_sink::remove_sink(spdlog::sink_ptr s)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _sinks.erase(std::remove(_sinks.begin(), _sinks.end(), s), _sinks.end());
}


inline spdlog::sinks::async_sink::q_type& spdlog::sinks::async_sink::q()
{
    return _q;
}


inline void spdlog::sinks::async_sink::shutdown(const std::chrono::milliseconds& timeout)
{
    if (timeout > std::chrono::milliseconds::zero())
    {
        auto until = log_clock::now() + timeout;
        while (_q.size() > 0 && log_clock::now() < until)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    _join();
}


inline void spdlog::sinks::async_sink::_push_sentry()
{
    std::shared_ptr<spdlog_ex> ex_copy = std::move(_last_backthread_ex);
    if (ex_copy)
        throw *ex_copy;
    if (!_active)
        throw(spdlog_ex("async_sink not active"));
}


inline void spdlog::sinks::async_sink::_join()
{
    _active = false;
    if (_back_thread.joinable())
    {
        try
        {
            _back_thread.join();
        }
        catch (const std::system_error&) //Dont crash if thread not joinable
        {}
    }

}

