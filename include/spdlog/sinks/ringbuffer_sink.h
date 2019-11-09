// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/details/null_mutex.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/synchronous_factory.h"
#include "spdlog/details/circular_q.h"
#include "spdlog/details/log_msg_buffer.h"

#include <mutex>
#include <string>
#include <vector>

namespace spdlog {
namespace sinks {
/*
 * Ring buffer sink
 */
template<typename Mutex>
class ringbuffer_sink final : public base_sink<Mutex>
{
public:
    explicit ringbuffer_sink(size_t buf_size);
    std::vector<std::string> last(size_t lim=0);

protected:
    void sink_it_(const details::log_msg &msg) override;
    void flush_() override {};

private:
    details::circular_q<details::log_msg_buffer> buf_;
};

using ringbuffer_sink_mt = ringbuffer_sink<std::mutex>;
using ringbuffer_sink_st = ringbuffer_sink<details::null_mutex>;

} // namespace sinks

//
// factory functions
//
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> basic_logger_mt(const std::string &logger_name, size_t buf_size)
{
    return Factory::template create<sinks::ringbuffer_sink_mt>(logger_name, buf_size);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> basic_logger_st(const std::string &logger_name, size_t buf_size)
{
    return Factory::template create<sinks::ringbuffer_sink_st>(logger_name, buf_size);
}

} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "ringbuffer_sink-inl.h"
#endif
