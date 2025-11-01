// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/sinks/level_based_sink.h>
#endif

#include <algorithm>
#include <mutex>

namespace spdlog {
namespace sinks {

void level_based_sink::add_sink_for_level(level::level_enum level, sink_ptr sink) {
    std::lock_guard<std::mutex> lock(mutex_);
    sinks_[level].push_back(std::move(sink));
}

void level_based_sink::add_sink_for_levels(std::initializer_list<level::level_enum> levels, sink_ptr sink) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto level : levels) {
        sinks_[level].push_back(sink);
    }
}

void level_based_sink::add_sink_for_all_levels(sink_ptr sink) {
    std::lock_guard<std::mutex> lock(mutex_);
    all_level_sinks_.push_back(std::move(sink));
}

void level_based_sink::remove_sink_for_level(level::level_enum level, sink_ptr sink) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sinks_.find(level);
    if (it != sinks_.end()) {
        auto &level_sinks = it->second;
        level_sinks.erase(std::remove(level_sinks.begin(), level_sinks.end(), sink), level_sinks.end());
    }
}

void level_based_sink::remove_all_sinks_for_level(level::level_enum level) {
    std::lock_guard<std::mutex> lock(mutex_);
    sinks_.erase(level);
}

void level_based_sink::remove_all_sinks() {
    std::lock_guard<std::mutex> lock(mutex_);
    sinks_.clear();
    all_level_sinks_.clear();
}

void level_based_sink::set_formatter(std::unique_ptr<formatter> formatter) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &[level, level_sinks] : sinks_) {
        for (auto &sink : level_sinks) {
            sink->set_formatter(formatter->clone());
        }
    }
    for (auto &sink : all_level_sinks_) {
        sink->set_formatter(std::move(formatter));
    }
}

void level_based_sink::set_level(level::level_enum level) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

level::level_enum level_based_sink::level() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return level_;
}

bool level_based_sink::should_log(level::level_enum level) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return level >= level_;
}

void level_based_sink::log(const details::log_msg &msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 首先发送到所有级别的sink
    for (auto &sink : all_level_sinks_) {
        if (sink->should_log(msg.level)) {
            try {
                sink->log(msg);
            } catch (const std::exception &ex) {
                // 忽略sink的异常，继续处理其他sink
            }
        }
    }
    
    // 然后发送到对应级别的sink
    auto it = sinks_.find(msg.level);
    if (it != sinks_.end()) {
        for (auto &sink : it->second) {
            if (sink->should_log(msg.level)) {
                try {
                    sink->log(msg);
                } catch (const std::exception &ex) {
                    // 忽略sink的异常，继续处理其他sink
                }
            }
        }
    }
}

void level_based_sink::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 刷新所有级别的sink
    for (auto &sink : all_level_sinks_) {
        try {
            sink->flush();
        } catch (const std::exception &ex) {
            // 忽略sink的异常，继续处理其他sink
        }
    }
    
    // 刷新对应级别的sink
    for (auto &[level, level_sinks] : sinks_) {
        for (auto &sink : level_sinks) {
            try {
                sink->flush();
            } catch (const std::exception &ex) {
                // 忽略sink的异常，继续处理其他sink
            }
        }
    }
}

} // namespace sinks
} // namespace spdlog