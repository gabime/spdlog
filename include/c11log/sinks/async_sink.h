#pragma once

#include <thread>
#include <chrono>
#include <atomic>

#include "base_sink.h"
#include "../logger.h"
#include "../details/blocking_queue.h"

namespace c11log
{
namespace sinks
{

class async_sink : public base_sink
{
public:
    using size_type = c11log::details::blocking_queue<std::string>::size_type;

    explicit async_sink(const size_type max_queue_size);
    ~async_sink();
    void add_sink(logger::sink_ptr sink);
    void remove_sink(logger::sink_ptr sink_ptr);

    //Wait to remaining items (if any) in the queue to be written and shutdown
    void shutdown(const std::chrono::seconds& timeout);


protected:
    void _sink_it(const bufpair_t& msg) override;
    void _thread_loop();

private:
    c11log::logger::sinks_vector_t _sinks;
    std::atomic<bool> _active;
    c11log::details::blocking_queue<std::string> _q;
    std::thread _back_thread;
    //Clear all remaining messages(if any), stop the _back_thread and join it
    void _shutdown();
};
}
}

///////////////////////////////////////////////////////////////////////////////
// async_sink class implementation
///////////////////////////////////////////////////////////////////////////////

inline c11log::sinks::async_sink::async_sink(const std::size_t max_queue_size)
    :_sinks(),
     _active(true),
     _q(max_queue_size),
     _back_thread(&async_sink::_thread_loop, this)
{}

inline c11log::sinks::async_sink::~async_sink()
{
    _shutdown();
}
inline void c11log::sinks::async_sink::_sink_it(const bufpair_t& msg)
{
    std::string s {msg.first, msg.first+msg.second};
    _q.push(s);
}

inline void c11log::sinks::async_sink::_thread_loop()
{
    static std::chrono::seconds  pop_timeout { 1 };
    std::string msg;

    while (_active)
    {
        if (_q.pop(msg, pop_timeout))
        {
            bufpair_t buf(msg.data(), msg.size());
            for (auto &sink : _sinks)
            {
                sink->log(buf, static_cast<level::level_enum>(_level.load()));
                if (!_active)
                    return;
            }
        }
    }
}

inline void c11log::sinks::async_sink::add_sink(logger::sink_ptr sink)
{
    _sinks.push_back(sink);
}

inline void c11log::sinks::async_sink::remove_sink(logger::sink_ptr sink_ptr)
{
    _sinks.erase(std::remove(_sinks.begin(), _sinks.end(), sink_ptr), _sinks.end());
}


inline void c11log::sinks::async_sink::shutdown(const std::chrono::seconds &timeout)
{
    auto until = std::chrono::system_clock::now() + timeout;
    while (_q.size() > 0 && std::chrono::system_clock::now() < until)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    _shutdown();
}

inline void c11log::sinks::async_sink::_shutdown()
{
    if(_active)
    {
        _active = false;
        if (_back_thread.joinable())
            _back_thread.join();
    }
}

