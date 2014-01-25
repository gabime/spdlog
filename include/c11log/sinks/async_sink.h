#pragma once
#include <mutex>
#include <queue>
#include <thread>
#include <iostream>

#include "../logger.h"
#include "base_sink.h"
namespace c11log {
namespace sinks {
class async_sink : base_sink {
    enum class fullq_policy {
        BLOCK=0,
        DROP_MSG
    };
public:
    async_sink(std::size_t max_queue_size, fullq_policy q_policy) :_fullq_policy(q_policy), _back_thread(&_thread_loop)
    {

    }
protected:
    void _sink_it(const std::string& msg) override
    {
        _msgs_mutex.unlock();
        _msgs.push(msg);
    }
    void _thread_loop()
    {
        while (_active) {
            _msgs_mutex.lock();
            std::string &msg = _msgs.front();
            _msgs.pop();
            _msgs_mutex.unlock();
            std::cout << "Popped: " << msg << std::endl;
        }
    }

private:
    c11log::logger::sinks_vector_t _sinks;
    fullq_policy _fullq_policy;
    std::queue<std::string> _msgs;
    std::thread _back_thread;
    bool _active = true;
    std::mutex _msgs_mutex;



};
}
}

void c11log::sinks::async_sink::_sink_it(const std::string& msg)
{

}