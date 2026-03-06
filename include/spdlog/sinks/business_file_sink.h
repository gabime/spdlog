// Copyright(c) 2025-present.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>

#include <mutex>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace spdlog {
namespace sinks {

// 业务类型枚举
enum class BusinessType {
    Screen,     // 录制屏幕
    Desktop,    // 打开桌面
    Keyboard,   // 录制键盘
    Audio,      // 录制声音
    Unknown     // 未知业务
};

// 将业务类型转换为字符串
inline const char* business_type_to_string(BusinessType type) {
    switch (type) {
        case BusinessType::Screen:   return "screen";
        case BusinessType::Desktop:  return "desktop";
        case BusinessType::Keyboard: return "keyboard";
        case BusinessType::Audio:    return "audio";
        default:                     return "unknown";
    }
}

// 将字符串转换为业务类型
inline BusinessType string_to_business_type(const std::string& str) {
    if (str == "screen")   return BusinessType::Screen;
    if (str == "desktop")  return BusinessType::Desktop;
    if (str == "keyboard") return BusinessType::Keyboard;
    if (str == "audio")    return BusinessType::Audio;
    return BusinessType::Unknown;
}

//
// 业务分类文件日志 Sink
// 根据业务类型将日志写入到不同的文件中
//
template <typename Mutex>
class business_file_sink final : public sink {
public:
    business_file_sink(const filename_t& base_dir,
                       std::size_t max_size = 30 * 1024 * 1024,  // 默认30MB
                       std::size_t max_files = 3,                 // 默认保留3个文件
                       bool rotate_on_open = false,
                       const file_event_handlers& event_handlers = {})
        : base_dir_(base_dir),
          max_size_(max_size),
          max_files_(max_files),
          rotate_on_open_(rotate_on_open),
          event_handlers_(event_handlers) {}

    // 获取或创建指定业务类型的 rotating_file_sink
    std::shared_ptr<rotating_file_sink<Mutex>> get_business_sink(BusinessType type) {
        std::lock_guard<Mutex> lock(mutex_);
        
        auto it = business_sinks_.find(type);
        if (it != business_sinks_.end()) {
            return it->second;
        }

        // 创建新的 sink
        filename_t filename = generate_filename_(type);
        auto sink = std::make_shared<rotating_file_sink<Mutex>>(
            filename, max_size_, max_files_, rotate_on_open_, event_handlers_);
        
        business_sinks_[type] = sink;
        return sink;
    }

    // 检查是否支持指定的业务类型
    bool has_business_sink(BusinessType type) const {
        std::lock_guard<Mutex> lock(mutex_);
        return business_sinks_.find(type) != business_sinks_.end();
    }

    // 获取所有已注册的业务类型
    std::vector<BusinessType> get_registered_business_types() const {
        std::lock_guard<Mutex> lock(mutex_);
        std::vector<BusinessType> types;
        for (const auto& pair : business_sinks_) {
            types.push_back(pair.first);
        }
        return types;
    }

    // 设置指定业务类型的日志级别
    void set_business_level(BusinessType type, level::level_enum lvl) {
        std::lock_guard<Mutex> lock(mutex_);
        business_levels_[type] = lvl;
    }

    // 获取指定业务类型的日志级别
    level::level_enum get_business_level(BusinessType type) const {
        std::lock_guard<Mutex> lock(mutex_);
        auto it = business_levels_.find(type);
        if (it != business_levels_.end()) {
            return it->second;
        }
        return level::trace; // 默认级别
    }

    // 主日志写入接口
    void log(const details::log_msg& msg) override {
        // 从消息中提取业务类型（通过前缀标识）
        BusinessType type = extract_business_type_(msg);
        
        auto sink = get_business_sink(type);
        if (sink) {
            // 检查业务级别
            level::level_enum business_lvl = get_business_level(type);
            if (msg.level >= business_lvl) {
                sink->log(msg);
            }
        }
    }

    void flush() override {
        std::lock_guard<Mutex> lock(mutex_);
        for (auto& pair : business_sinks_) {
            pair.second->flush();
        }
    }

    void set_pattern(const std::string& pattern) override {
        std::lock_guard<Mutex> lock(mutex_);
        for (auto& pair : business_sinks_) {
            pair.second->set_pattern(pattern);
        }
    }

    void set_formatter(std::unique_ptr<formatter> sink_formatter) override {
        std::lock_guard<Mutex> lock(mutex_);
        for (auto& pair : business_sinks_) {
            pair.second->set_formatter(sink_formatter->clone());
        }
    }

private:
    // 从日志消息中提取业务类型
    // 约定：日志消息以 [BUSINESS:xxx] 开头，其中 xxx 是业务类型
    BusinessType extract_business_type_(const details::log_msg& msg) const {
        // 默认返回 Unknown，让子类或上层处理
        return BusinessType::Unknown;
    }

    // 生成业务日志文件名
    filename_t generate_filename_(BusinessType type) const {
        filename_t business_name = SPDLOG_FILENAME_T(business_type_to_string(type));
        return fmt_lib::format(SPDLOG_FMT_STRING(SPDLOG_FILENAME_T("{}/{}.log")), 
                               base_dir_, business_name);
    }

    mutable Mutex mutex_;
    filename_t base_dir_;
    std::size_t max_size_;
    std::size_t max_files_;
    bool rotate_on_open_;
    file_event_handlers event_handlers_;
    std::unordered_map<BusinessType, std::shared_ptr<rotating_file_sink<Mutex>>> business_sinks_;
    std::unordered_map<BusinessType, level::level_enum> business_levels_;
};

using business_file_sink_mt = business_file_sink<std::mutex>;
using business_file_sink_st = business_file_sink<details::null_mutex>;

}  // namespace sinks

// 业务类型别名，方便使用
using BusinessType = sinks::BusinessType;

}  // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include <spdlog/sinks/business_file_sink-inl.h>
#endif
