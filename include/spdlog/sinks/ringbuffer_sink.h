// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include "../details/circular_q.h"
#include "../details/log_msg_buffer.h"
#include "../details/null_mutex.h"
#include "./base_sink.h"

namespace spdlog {
namespace sinks {
/*
 * Ring buffer sink. Holds fixed amount of log messages in memory. When the buffer is full, new
 * messages override the old ones. Useful for storing debug data in memory in case of error.
 * Example: auto rb_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(128); spdlog::logger
 * logger("rb_logger", rb_sink); rb->drain([](const std::string_view msg) { process(msg);});
 */
template <typename Mutex>
class ringbuffer_sink final : public base_sink<Mutex> {
public:
    explicit ringbuffer_sink(size_t n_items)
        : q_{n_items} {}

    void drain_raw(std::function<void(const details::log_msg_buffer &)> callback) {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        while (!q_.empty()) {
            callback(q_.front());
            q_.pop_front();
        }
    }

    void drain(std::function<void(std::string_view)> callback) {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        memory_buf_t formatted;
        while (!q_.empty()) {
            formatted.clear();
            base_sink<Mutex>::formatter_->format(q_.front(), formatted);
            callback(std::string_view(formatted.data(), formatted.size()));
            q_.pop_front();
        }
    }

protected:
    void sink_it_(const details::log_msg &msg) override { q_.push_back(details::log_msg_buffer{msg}); }
    void flush_() override {}

private:
    details::circular_q<details::log_msg_buffer> q_;
};

using ringbuffer_sink_mt = ringbuffer_sink<std::mutex>;
using ringbuffer_sink_st = ringbuffer_sink<details::null_mutex>;

}  // namespace sinks
}  // namespace spdlog
