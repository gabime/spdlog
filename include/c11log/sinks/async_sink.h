#pragma once

#include <thread>
#include <chrono>
#include <atomic>
#include <algorithm>

#include "base_sink.h"
#include "../logger.h"
#include "../details/blocking_queue.h"
#include "../details/log_msg.h"

#include<iostream>

namespace c11log
{
namespace sinks
{

template<class Mutex>
class async_sink : public base_sink<Mutex>
{
public:
    using q_type = details::blocking_queue<details::log_msg>;

    explicit async_sink(const q_type::size_type max_queue_size);

    //Stop logging and join the back thread
    // TODO: limit with timeout of the join and kill it afterwards?
    ~async_sink();
    void add_sink(logger::sink_ptr sink);
    void remove_sink(logger::sink_ptr sink_ptr);
    q_type& q();
    //Wait to remaining items (if any) in the queue to be written and shutdown
    void shutdown(const std::chrono::milliseconds& timeout);


protected:
    void _sink_it(const details::log_msg& msg) override;
    void _thread_loop();

private:
    c11log::logger::sinks_vector_t _sinks;
    std::atomic<bool> _active;
    q_type _q;
    std::thread _back_thread;
    //Clear all remaining messages(if any), stop the _back_thread and join it
    void _shutdown();
    std::mutex _shutdown_mutex;
};
}
}

///////////////////////////////////////////////////////////////////////////////
// async_sink class implementation
///////////////////////////////////////////////////////////////////////////////
template<class Mutex>
inline c11log::sinks::async_sink<Mutex>::async_sink(const q_type::size_type max_queue_size)
    :_sinks(),
     _active(true),
     _q(max_queue_size),
     _back_thread(&async_sink::_thread_loop, this)
{}

template<class Mutex>
inline c11log::sinks::async_sink<Mutex>::~async_sink()
{
    _shutdown();
}

template<class Mutex>
inline void c11log::sinks::async_sink<Mutex>::_sink_it(const details::log_msg& msg)
{
    if(!_active || msg.formatted.empty())
        return;
    _q.push(msg);
}

template<class Mutex>
inline void c11log::sinks::async_sink<Mutex>::_thread_loop()
{
    static std::chrono::seconds  pop_timeout { 1 };
    while (_active)
    {
        q_type::item_type msg;
        if (_q.pop(msg, pop_timeout))
        {
            for (auto &sink : _sinks)
            {
                sink->log(msg);
                if(!_active)
                    break;
            }
        }
    }
}

template<class Mutex>
inline void c11log::sinks::async_sink<Mutex>::add_sink(logger::sink_ptr sink)
{
    _sinks.push_back(sink);
}

template<class Mutex>
inline void c11log::sinks::async_sink<Mutex>::remove_sink(logger::sink_ptr sink)
{
    _sinks.erase(std::remove(_sinks.begin(), _sinks.end(), sink), _sinks.end());
}
/*
template<class Mutex>
inline c11log::sinks::async_sink::q_type& c11log::sinks::async_sink<Mutex>::q()
{
    return _q;
}*/


template<class Mutex>
inline void c11log::sinks::async_sink<Mutex>::shutdown(const std::chrono::milliseconds& timeout)
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


template<class Mutex>
inline void c11log::sinks::async_sink<Mutex>::_shutdown()
{
    std::lock_guard<std::mutex> guard(_shutdown_mutex);
    if(_active)
    {
        _active = false;
        if (_back_thread.joinable())
            _back_thread.join();
    }
}

