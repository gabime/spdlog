//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

// async logger implementation
// uses a thread pool to perform the actual logging

#include "spdlog/details/thread_pool.h"

#include <chrono>
#include <memory>
#include <string>

template<class It>
inline spdlog::async_logger::async_logger(const std::string &logger_name, const It &begin, const It &end,
    std::weak_ptr<details::thread_pool> tp, async_overflow_policy overflow_policy)
    : logger(logger_name, begin, end)
    , _thread_pool(tp)
    , _overflow_policy(overflow_policy)
{
}

inline spdlog::async_logger::async_logger(const std::string &logger_name, sinks_init_list sinks_list,
    std::weak_ptr<details::thread_pool> tp, async_overflow_policy overflow_policy)
    : async_logger(logger_name, sinks_list.begin(), sinks_list.end(), tp, overflow_policy)
{
}

inline spdlog::async_logger::async_logger(
    const std::string &logger_name, sink_ptr single_sink, std::weak_ptr<details::thread_pool> tp, async_overflow_policy overflow_policy)
    : async_logger(logger_name, {single_sink}, tp, overflow_policy)
{
}

// send the log message to the thread pool
inline void spdlog::async_logger::_sink_it(details::log_msg &msg)
{
#if defined(SPDLOG_ENABLE_MESSAGE_COUNTER)
    _incr_msg_counter(msg);
#endif
    if (auto pool_ptr = _thread_pool.lock())
    {
        pool_ptr->post_log(shared_from_this(), std::move(msg), _overflow_policy);
    }
    else
    {
        throw spdlog_ex("async log: thread pool doens't exist anymore");
    }
}

// send flush request to the thread pool
inline void spdlog::async_logger::_flush()
{
    if (auto pool_ptr = _thread_pool.lock())
    {
        pool_ptr->post_flush(shared_from_this(), _overflow_policy);
    }
    else
    {
        throw spdlog_ex("async flush: thread pool doens't exist anymore");
    }
}

//
// backend functions - called from the thread pool to do the actual job
//
inline void spdlog::async_logger::_backend_log(details::log_msg &incoming_log_msg)
{
    try
    {
        _formatter->format(incoming_log_msg);
        for (auto &s : _sinks)
        {
            if (s->should_log(incoming_log_msg.level))
            {
                s->log(incoming_log_msg);
            }
        }
    }
	SPDLOG_CATCH_AND_HANDLE

    if (_should_flush(incoming_log_msg))
    {
        _backend_flush();
    }
}

inline void spdlog::async_logger::_backend_flush()
{
    try
    {
        for (auto &sink : _sinks)
        {
            sink->flush();
        }
    }
	SPDLOG_CATCH_AND_HANDLE
}
