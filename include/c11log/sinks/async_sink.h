#pragma once

#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>

#include "base_sink.h"
#include "../logger.h"
#include "../details/blocking_queue.h"

namespace c11log {
namespace sinks {
class async_sink : public base_sink {
    
public:
    using size_type = c11log::details::blocking_queue<std::string>::size_type;    
    explicit async_sink(const std::size_t max_queue_size, const std::chrono::seconds& timeout = std::chrono::seconds::max());        
    ~async_sink();
    void add_sink(logger::sink_ptr_t sink);
    void remove_sink(logger::sink_ptr_t sink_ptr);    
    

protected:    
    void sink_it_(const std::string& msg) override;
    void thread_loop_();
    
private:    
    c11log::logger::sinks_vector_t sinks_;
    bool active_ = true;    
    const std::chrono::seconds timeout_;
    c11log::details::blocking_queue<std::string> q_;
    std::thread back_thread_; 
    void shutdown_();
};
}
}

//
// async_sink inline impl
//

inline c11log::sinks::async_sink::async_sink(const std::size_t max_queue_size, const std::chrono::seconds& timeout)
    :q_(max_queue_size),
    timeout_(timeout),
    back_thread_(&async_sink::thread_loop_, this)
{}

inline c11log::sinks::async_sink::~async_sink()
{
    shutdown_();
}
inline void c11log::sinks::async_sink::sink_it_(const std::string& msg)
{
    q_.push(msg, timeout_);
}

inline void c11log::sinks::async_sink::thread_loop_()
{   
    std::string msg;
    while (active_) 
    {        
        if (q_.pop(msg, timeout_))
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto &sink : sinks_)
            {
                if (active_)
                    sink->log(msg, _level);
                else
                    break;
            }
        }
    }
}

inline void c11log::sinks::async_sink::add_sink(logger::sink_ptr_t sink)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sinks_.push_back(sink);
}

inline void c11log::sinks::async_sink::remove_sink(logger::sink_ptr_t sink_ptr)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), sink_ptr), sinks_.end());
}

inline void c11log::sinks::async_sink::shutdown_()
{
    {        
        std::lock_guard<std::mutex> lock(mutex_);
        active_ = false;        
    }
    q_.clear();
    back_thread_.join();
}

