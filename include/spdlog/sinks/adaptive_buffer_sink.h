// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/log_msg_buffer.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/sinks/sink.h>

#include <mutex>
#include <vector>
#include <memory>
#include <utility>

namespace spdlog {
namespace sinks {

template <typename Mutex>
class adaptive_buffer_sink final : public sink {
public:
    explicit adaptive_buffer_sink(std::shared_ptr<sink> target_sink, size_t buffer_size = 8192)
        : target_sink_(std::move(target_sink)),
          buffer_size_(buffer_size) {
        if (buffer_size_ == 0) {
            throw_spdlog_ex("adaptive_buffer_sink: buffer_size cannot be zero");
        }
        std::lock_guard<Mutex> lock(mutex_);
        buffer_.reserve(buffer_size_);
    }

    adaptive_buffer_sink(const adaptive_buffer_sink &) = delete;
    adaptive_buffer_sink(adaptive_buffer_sink &&) = delete;
    adaptive_buffer_sink &operator=(const adaptive_buffer_sink &) = delete;
    adaptive_buffer_sink &operator=(adaptive_buffer_sink &&) = delete;

    ~adaptive_buffer_sink() override {
        try {
            flush();
        } catch (...) {
        }
    }

    void log(const details::log_msg &msg) override {
        std::unique_lock<Mutex> lock(mutex_);
        buffer_.emplace_back(msg);
        if (buffer_.size() >= buffer_size_) {
            std::vector<details::movable_log_msg_buffer> local_buffer = std::move(buffer_);
            buffer_.reserve(buffer_size_);
            lock.unlock();
            flush_local_buffer_(local_buffer);
        }
    }

    void flush() override {
        std::vector<details::movable_log_msg_buffer> local_buffer;
        {
            std::lock_guard<Mutex> lock(mutex_);
            if (buffer_.empty()) {
                return;
            }
            local_buffer = std::move(buffer_);
            buffer_.reserve(buffer_size_);
        }
        flush_local_buffer_(local_buffer);
        target_sink_->flush();
    }

    void set_pattern(const std::string &pattern) override {
        target_sink_->set_pattern(pattern);
    }

    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override {
        target_sink_->set_formatter(std::move(sink_formatter));
    }

private:
    void flush_local_buffer_(std::vector<details::movable_log_msg_buffer> &local_buffer) {
        for (const auto &msg : local_buffer) {
            target_sink_->log(msg);
        }
    }

    std::shared_ptr<sink> target_sink_;
    size_t buffer_size_;
    std::vector<details::movable_log_msg_buffer> buffer_;
    Mutex mutex_;
};

using adaptive_buffer_sink_mt = adaptive_buffer_sink<std::mutex>;
using adaptive_buffer_sink_st = adaptive_buffer_sink<details::null_mutex>;

}  // namespace sinks

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> adaptive_buffer_logger_mt(const std::string &logger_name,
                                                          std::shared_ptr<sinks::sink> target_sink,
                                                          size_t buffer_size = 8192) {
    return Factory::template create<sinks::adaptive_buffer_sink_mt>(logger_name, std::move(target_sink),
                                                                      buffer_size);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> adaptive_buffer_logger_st(const std::string &logger_name,
                                                          std::shared_ptr<sinks::sink> target_sink,
                                                          size_t buffer_size = 8192) {
    return Factory::template create<sinks::adaptive_buffer_sink_st>(logger_name, std::move(target_sink),
                                                                      buffer_size);
}

}  // namespace spdlog
