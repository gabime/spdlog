// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

//#ifndef SPDLOG_HEADER_ONLY
//#include "spdlog/sinks/ringbuffer_sink.h"
//#endif

#include "spdlog/common.h"

namespace spdlog {
namespace sinks {

template<typename Mutex>
SPDLOG_INLINE ringbuffer_sink<Mutex>::ringbuffer_sink(size_t n_items)
{
    buf_ = details::circular_q<details::log_msg_buffer>(n_items);
}

template<typename Mutex>
SPDLOG_INLINE void ringbuffer_sink<Mutex>::sink_it_(const details::log_msg &msg)
{
    buf_.push_back(details::log_msg_buffer{msg});
}

template<typename Mutex>
SPDLOG_INLINE std::vector<std::string> ringbuffer_sink<Mutex>::formatted_messages(size_t lim)
{
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    auto n_items = lim > 0 ? (std::min)(lim, buf_.size()) : buf_.size();
    std::vector<std::string> ret;
    ret.reserve(n_items);
    for (size_t i = 0; i < n_items; i++)
    {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(buf_.at(i), formatted);
        ret.push_back(fmt::to_string(formatted));
    }
    return ret;
}

template<typename Mutex>
SPDLOG_INLINE std::vector<details::log_msg_buffer> ringbuffer_sink<Mutex>::raw_messages(size_t lim)
{
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    auto n_items = lim > 0 ? (std::min)(lim, buf_.size()) : buf_.size();
    std::vector<details::log_msg_buffer> ret;
    ret.reserve(n_items);
    for (size_t i = 0; i < n_items; i++)
    {
        ret.push_back(buf_.at(i));
    }
    return ret;
}

} // namespace sinks
} // namespace spdlog
