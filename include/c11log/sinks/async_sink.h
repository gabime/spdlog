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

namespace c11log
{
namespace sinks
{

class async_sink : public base_sink<details::null_mutex>
{
public:
    using q_type = details::blocking_queue<details::log_msg>;

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
    //Clear all remaining messages(if any), stop the _back_thread and join it
    void _shutdown();
    std::mutex _mutex;
};
}
}

///////////////////////////////////////////////////////////////////////////////
// async_sink class implementation
///////////////////////////////////////////////////////////////////////////////
inline c11log::sinks::async_sink::async_sink(const q_type::size_type max_queue_size)
    :_sinks(),
     _active(true),
     _q(max_queue_size),
     _back_thread(&async_sink::_thread_loop, this)
{}

inline c11log::sinks::async_sink::~async_sink()
{
    _shutdown();
}

inline void c11log::sinks::async_sink::_sink_it(const details::log_msg& msg)
{
    if(!_active)
        return;
    _q.push(msg);
}

inline void c11log::sinks::async_sink::_thread_loop()
{
    static std::chrono::seconds  pop_timeout { 1 };
    while (_active)
    {
        q_type::item_type msg;
        if (_q.pop(msg, pop_timeout))
        {
            for (auto &s : _sinks)
            {
                s->log(msg);
                if(!_active)
                    break;
            }
        }
    }
}

inline void c11log::sinks::async_sink::add_sink(c11log::sink_ptr s)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _sinks.push_back(s);
}


inline void c11log::sinks::async_sink::remove_sink(c11log::sink_ptr s)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _sinks.erase(std::remove(_sinks.begin(), _sinks.end(), s), _sinks.end());
}


inline c11log::sinks::async_sink::q_type& c11log::sinks::async_sink::q()
{
    return _q;
}


inline void c11log::sinks::async_sink::shutdown(const std::chrono::milliseconds& timeout)
{
    if(timeout > std::chrono::milliseconds::zero())
    {
        auto until = log_clock::now() + timeout;
        while (_q.size() > 0 && log_clock::now() < until)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
    _shutdown();
}


inline void c11log::sinks::async_sink::_shutdown()
{
    std::lock_guard<std::mutex> guard(_mutex);
    if(_active)
    {
        _active = false;
        if (_back_thread.joinable())
            _back_thread.join();
    }
}

