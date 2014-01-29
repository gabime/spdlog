#pragma once

#include <thread>
#include <chrono>
#include <atomic>

#include "base_sink.h"
#include "../logger.h"
#include "../details/blocking_queue.h"

namespace c11log {
namespace sinks {

class async_sink : public base_sink
{
public:
    using size_type = c11log::details::blocking_queue<std::string>::size_type;

    explicit async_sink(const size_type max_queue_size);
    ~async_sink();
    void add_sink(logger::sink_ptr_t sink);
    void remove_sink(logger::sink_ptr_t sink_ptr);

    //Wait to remaining items (if any) in the queue to be written and shutdown
    void shutdown(const std::chrono::seconds& timeout);


protected:
    void sink_it_(const std::string& msg) override;
    void thread_loop_();

private:
    c11log::logger::sinks_vector_t sinks_;
    std::atomic<bool> active_ { true };
    c11log::details::blocking_queue<std::string> q_;
    std::thread back_thread_;
    //Clear all remaining messages(if any), stop the back_thread_ and join it
    void shutdown_();
};
}
}

///////////////////////////////////////////////////////////////////////////////
// async_sink class implementation
///////////////////////////////////////////////////////////////////////////////

inline c11log::sinks::async_sink::async_sink(const std::size_t max_queue_size)
    :q_(max_queue_size),
     back_thread_(&async_sink::thread_loop_, this)
{}

inline c11log::sinks::async_sink::~async_sink()
{
    shutdown_();
}
inline void c11log::sinks::async_sink::sink_it_(const std::string& msg)
{
    q_.push(msg);
}

inline void c11log::sinks::async_sink::thread_loop_()
{
	constexpr auto pop_timeout = std::chrono::seconds(1);
    std::string msg;
    
    while (active_)
    {
        if (q_.pop(msg, pop_timeout))
        {
            for (auto &sink : sinks_)
            {
                sink->log(msg, static_cast<level::level_enum>(_level.load()));
                if (!active_)
                    return;
            }
        }
    }
}

inline void c11log::sinks::async_sink::add_sink(logger::sink_ptr_t sink)
{
    sinks_.push_back(sink);
}

inline void c11log::sinks::async_sink::remove_sink(logger::sink_ptr_t sink_ptr)
{
    sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), sink_ptr), sinks_.end());
}


inline void c11log::sinks::async_sink::shutdown(const std::chrono::seconds &timeout)
{
    auto until = std::chrono::system_clock::now() + timeout;
    while (q_.size() > 0 && std::chrono::system_clock::now() < until)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    shutdown_();
}

inline void c11log::sinks::async_sink::shutdown_()
{
    if(active_)
    {
        active_ = false;
        if (back_thread_.joinable())
            back_thread_.join();
    }
}

