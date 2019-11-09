// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include "spdlog/sinks/ringbuffer_sink.h"
#endif

#include "spdlog/common.h"
#include "spdlog/details/os.h"

namespace spdlog {
namespace sinks {

template<typename Mutex>
SPDLOG_INLINE ringbuffer_sink<Mutex>::ringbuffer_sink(size_t buf_size)
{
    buf_=details::circular_q<details::log_msg_buffer>(buf_size);
}

template<typename Mutex>
SPDLOG_INLINE void ringbuffer_sink<Mutex>::sink_it_(const details::log_msg &msg)
{
    buf_.push_back(details::log_msg_buffer{msg});
}

template<typename Mutex>
SPDLOG_INLINE std::vector<std::string> ringbuffer_sink<Mutex>::last(size_t lim)
{
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    std::vector<std::string> ret;
    ret.reserve(lim);
    size_t num=0;
    for(size_t i=0; i<buf_.size(); i++){
        num++;
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(buf_.at(i), formatted);
        ret.push_back(fmt::to_string(formatted));
        if(lim>0 && num==lim) break;
    }
    return ret;
}

} // namespace sinks
} // namespace spdlog
