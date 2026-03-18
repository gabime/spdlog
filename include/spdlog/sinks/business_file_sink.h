// Copyright(c) 2025-present, Business Logger Extension for spdlog.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace spdlog {
namespace sinks {

// Business type enumeration
enum class business_type {
    screen_recorder,    // 录制屏幕
    desktop_open,       // 打开桌面
    keyboard_recorder,  // 录制键盘
    sound_recorder      // 录制声音
};

// Convert business_type to string
inline std::string business_type_to_string(business_type type) {
    switch (type) {
        case business_type::screen_recorder:
            return "screen";
        case business_type::desktop_open:
            return "desktop";
        case business_type::keyboard_recorder:
            return "keyboard";
        case business_type::sound_recorder:
            return "sound";
        default:
            return "unknown";
    }
}

// Business rotating file sink - manages multiple rotating file sinks for different business types
template <typename Mutex>
class business_file_sink final : public base_sink<Mutex> {
public:
    // max_size: maximum size of each file in bytes
    // max_files: maximum number of rotated files to keep
    business_file_sink(const filename_t &base_dir,
                       std::size_t max_size = 30 * 1024 * 1024,  // 30MB default
                       std::size_t max_files = 3,
                       bool rotate_on_open = false,
                       const file_event_handlers &event_handlers = {})
        : base_dir_(base_dir),
          max_size_(max_size),
          max_files_(max_files),
          rotate_on_open_(rotate_on_open),
          event_handlers_(event_handlers) {
        // Initialize sinks for all business types
        init_business_sinks();
    }

    // Log with specific business type
    void log_with_business(business_type biz_type,
                           const details::log_msg &msg) {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        auto it = business_sinks_.find(biz_type);
        if (it != business_sinks_.end()) {
            it->second->log(msg);
        }
    }

    // Get the filename for a specific business type
    filename_t get_filename(business_type biz_type) {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        auto it = business_sinks_.find(biz_type);
        if (it != business_sinks_.end()) {
            return it->second->filename();
        }
        return {};
    }

    // Force rotation for a specific business type
    void rotate_now(business_type biz_type) {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        auto it = business_sinks_.find(biz_type);
        if (it != business_sinks_.end()) {
            it->second->rotate_now();
        }
    }

    // Get all business types
    std::vector<business_type> get_business_types() const {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        std::vector<business_type> types;
        for (const auto &pair : business_sinks_) {
            types.push_back(pair.first);
        }
        return types;
    }

protected:
    void sink_it_(const details::log_msg &msg) override {
        // Default behavior: log to all business sinks
        // This is called when using the standard log interface
        for (auto &pair : business_sinks_) {
            pair.second->log(msg);
        }
    }

    void flush_() override {
        for (auto &pair : business_sinks_) {
            pair.second->flush();
        }
    }

private:
    void init_business_sinks() {
        // Create rotating file sink for each business type
        std::vector<business_type> all_types = {
            business_type::screen_recorder,
            business_type::desktop_open,
            business_type::keyboard_recorder,
            business_type::sound_recorder
        };

        for (auto biz_type : all_types) {
            filename_t filename = build_filename(biz_type);
            auto sink = std::make_shared<rotating_file_sink_mt>(
                filename, max_size_, max_files_, rotate_on_open_, event_handlers_);
            business_sinks_[biz_type] = std::move(sink);
        }
    }

    filename_t build_filename(business_type biz_type) {
        std::string biz_name = business_type_to_string(biz_type);
        #ifdef _WIN32
        return base_dir_ + SPDLOG_FILENAME_T("\\") + SPDLOG_FILENAME_T(biz_name.c_str()) + SPDLOG_FILENAME_T(".log");
        #else
        return base_dir_ + SPDLOG_FILENAME_T("/") + SPDLOG_FILENAME_T(biz_name.c_str()) + SPDLOG_FILENAME_T(".log");
        #endif
    }

    filename_t base_dir_;
    std::size_t max_size_;
    std::size_t max_files_;
    bool rotate_on_open_;
    file_event_handlers event_handlers_;
    std::map<business_type, std::shared_ptr<rotating_file_sink_mt>> business_sinks_;
};

using business_file_sink_mt = business_file_sink<std::mutex>;
using business_file_sink_st = business_file_sink<details::null_mutex>;

}  // namespace sinks

//
// factory functions
//
template <typename Factory = spdlog::synchronous_factory>
std::shared_ptr<logger> business_logger_mt(const std::string &logger_name,
                                           const filename_t &base_dir,
                                           size_t max_file_size = 30 * 1024 * 1024,
                                           size_t max_files = 3,
                                           bool rotate_on_open = false,
                                           const file_event_handlers &event_handlers = {}) {
    return Factory::template create<sinks::business_file_sink_mt>(
        logger_name, base_dir, max_file_size, max_files, rotate_on_open, event_handlers);
}

template <typename Factory = spdlog::synchronous_factory>
std::shared_ptr<logger> business_logger_st(const std::string &logger_name,
                                           const filename_t &base_dir,
                                           size_t max_file_size = 30 * 1024 * 1024,
                                           size_t max_files = 3,
                                           bool rotate_on_open = false,
                                           const file_event_handlers &event_handlers = {}) {
    return Factory::template create<sinks::business_file_sink_st>(
        logger_name, base_dir, max_file_size, max_files, rotate_on_open, event_handlers);
}

}  // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
// No inline implementation needed for template class
#endif
