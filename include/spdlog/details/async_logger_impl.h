//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

// Async Logger implementation
// Use an async_sink (queue per logger) to perform the logging in a worker thread

#include "./async_log_helper.h"


template<class It>
inline spdlog::async_logger::async_logger(const std::string& logger_name,
        const It& begin,
        const It& end,
        size_t queue_size,
        const  async_overflow_policy overflow_policy,
        const std::function<void()>& worker_warmup_cb,
        const std::chrono::milliseconds& flush_interval_ms) :
    logger(logger_name, begin, end),
    _async_log_helper(new details::async_log_helper(_formatter, _sinks, queue_size, overflow_policy, worker_warmup_cb, flush_interval_ms))
{
}

inline spdlog::async_logger::async_logger(const std::string& logger_name,
        sinks_init_list sinks,
        size_t queue_size,
        const  async_overflow_policy overflow_policy,
        const std::function<void()>& worker_warmup_cb,
        const std::chrono::milliseconds& flush_interval_ms) :
    async_logger(logger_name, sinks.begin(), sinks.end(), queue_size, overflow_policy, worker_warmup_cb, flush_interval_ms) {}

inline spdlog::async_logger::async_logger(const std::string& logger_name,
        sink_ptr single_sink,
        size_t queue_size,
        const  async_overflow_policy overflow_policy,
        const std::function<void()>& worker_warmup_cb,
        const std::chrono::milliseconds& flush_interval_ms) :
    async_logger(logger_name,
{
    single_sink
}, queue_size, overflow_policy, worker_warmup_cb, flush_interval_ms) {}


inline void spdlog::async_logger::flush()
{

    _async_log_helper->flush();
}

inline void spdlog::async_logger::_set_formatter(spdlog::formatter_ptr msg_formatter)
{
    _formatter = msg_formatter;
    _async_log_helper->set_formatter(_formatter);
}

inline void spdlog::async_logger::_set_pattern(const std::string& pattern)
{
    _formatter = std::make_shared<pattern_formatter>(pattern);
    _async_log_helper->set_formatter(_formatter);
}


inline void spdlog::async_logger::_log_msg(details::log_msg& msg)
{
    _async_log_helper->log(msg);
}
