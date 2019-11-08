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
    buf=details::circular_q<std::string>(buf_size);
}

template<typename Mutex>
SPDLOG_INLINE void ringbuffer_sink<Mutex>::sink_it_(const details::log_msg &msg)
{
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    buf.push_back(fmt::to_string(formatted));
}

template<typename Mutex>
SPDLOG_INLINE std::vector<std::string> ringbuffer_sink<Mutex>::last(size_t lim)
{
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    std::vector<std::string> ret;
    ret.reserve(lim);
    size_t num=0;
    for(size_t i=0; i<buf.size(); i++){
        num++;
        ret.push_back(buf.at(i));
        if(lim>0 && num==lim) break;
    }
    return ret;
}

} // namespace sinks
} // namespace spdlog
