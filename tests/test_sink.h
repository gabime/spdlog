//
// Copyright(c) 2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <chrono>
#include <exception>
#include <iterator>
#include <mutex>
#include <thread>

#include "spdlog/details/null_mutex.h"
#include "spdlog/details/os.h"
#include "spdlog/sinks/base_sink.h"

namespace spdlog {
namespace sinks {

template <class Mutex>
class test_sink : public base_sink<Mutex> {
    const size_t lines_to_save = 100;

public:
    size_t msg_counter() {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return msg_counter_;
    }

    size_t flush_counter() {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return flush_counter_;
    }

    void set_delay(std::chrono::milliseconds delay) {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        delay_ = delay;
    }

    void set_exception(const std::runtime_error& ex) { exception_ptr_ = std::make_exception_ptr(ex); }

    void clear_exception() { exception_ptr_ = nullptr; }

    // return last output without the eol
    std::vector<std::string> lines() {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return lines_;
    }

protected:
    void sink_it_(const details::log_msg& msg) override {
        if (exception_ptr_) {
            std::rethrow_exception(exception_ptr_);
        }
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        // save the line without the eol
        auto eol_len = strlen(details::os::default_eol);
        if (lines_.size() < lines_to_save) {
            using diff_t = typename std::iterator_traits<decltype(formatted.end())>::difference_type;
            lines_.emplace_back(formatted.begin(), formatted.end() - static_cast<diff_t>(eol_len));
        }
        msg_counter_++;
        std::this_thread::sleep_for(delay_);
    }

    void flush_() override {
        if (exception_ptr_) {
            std::rethrow_exception(exception_ptr_);
        }
        flush_counter_++;
    }

    size_t msg_counter_{0};
    size_t flush_counter_{0};
    std::chrono::milliseconds delay_{std::chrono::milliseconds::zero()};
    std::vector<std::string> lines_;
    std::exception_ptr exception_ptr_;  // will be thrown on next log or flush if not null
};

using test_sink_mt = test_sink<std::mutex>;
using test_sink_st = test_sink<details::null_mutex>;

}  // namespace sinks
}  // namespace spdlog
