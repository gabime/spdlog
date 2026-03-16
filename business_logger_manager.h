// Copyright(c) 2026.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/logger.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace spdlog {
namespace sinks {

// 业务类型枚举
enum class BusinessType {
    ScreenRecord,    // 录制屏幕
    DesktopOpen,     // 打开桌面
    KeyboardRecord,  // 录制键盘
    SoundRecord      // 录制声音
};

// 业务日志管理器
// 支持根据不同业务类型写入不同的日志文件
// 每个业务最多保留3个日志文件，每个文件最大30MB
class BusinessLoggerManager {
public:
    // 单例模式获取实例
    static BusinessLoggerManager& get_instance();

    // 初始化日志管理器，设置日志文件基础路径
    void initialize(const std::string& base_path);

    // 获取指定业务的logger
    std::shared_ptr<logger> get_logger(BusinessType type);

    // 记录日志的便捷方法
    void log(BusinessType type, level::level_enum lvl, const std::string& msg);

    // 各业务类型的便捷日志方法
    void screen_record_log(level::level_enum lvl, const std::string& msg);
    void desktop_open_log(level::level_enum lvl, const std::string& msg);
    void keyboard_record_log(level::level_enum lvl, const std::string& msg);
    void sound_record_log(level::level_enum lvl, const std::string& msg);

    // 设置所有业务logger的日志级别
    void set_level(level::level_enum lvl);

    // 刷新所有日志
    void flush_all();

    // 关闭所有logger并清理资源
    void shutdown();

private:
    BusinessLoggerManager() = default;
    ~BusinessLoggerManager() = default;

    // 禁止拷贝和赋值
    BusinessLoggerManager(const BusinessLoggerManager&) = delete;
    BusinessLoggerManager& operator=(const BusinessLoggerManager&) = delete;

    // 创建业务logger
    std::shared_ptr<logger> create_business_logger(BusinessType type);

    // 获取业务类型对应的名称
    std::string get_business_name(BusinessType type);

    // 获取业务类型对应的日志文件名
    std::string get_business_filename(BusinessType type);

private:
    std::string base_path_;  // 日志文件基础路径
    std::unordered_map<BusinessType, std::shared_ptr<logger>> loggers_;  // 业务logger映射
    std::mutex mutex_;  // 保护loggers_的互斥锁
    bool initialized_ = false;  // 是否已初始化

    // 默认配置
    static constexpr size_t MAX_FILE_SIZE = 30 * 1024 * 1024;  // 30MB
    static constexpr size_t MAX_FILES = 3;  // 最多保留3个文件
};

}  // namespace sinks

// 便捷的全局函数
namespace business_log {

// 初始化业务日志管理器
inline void initialize(const std::string& base_path) {
    sinks::BusinessLoggerManager::get_instance().initialize(base_path);
}

// 录制屏幕日志
inline void screen_record(level::level_enum lvl, const std::string& msg) {
    sinks::BusinessLoggerManager::get_instance().screen_record_log(lvl, msg);
}

// 打开桌面日志
inline void desktop_open(level::level_enum lvl, const std::string& msg) {
    sinks::BusinessLoggerManager::get_instance().desktop_open_log(lvl, msg);
}

// 录制键盘日志
inline void keyboard_record(level::level_enum lvl, const std::string& msg) {
    sinks::BusinessLoggerManager::get_instance().keyboard_record_log(lvl, msg);
}

// 录制声音日志
inline void sound_record(level::level_enum lvl, const std::string& msg) {
    sinks::BusinessLoggerManager::get_instance().sound_record_log(lvl, msg);
}

// 模板函数，支持格式化字符串
template<typename... Args>
void screen_record_fmt(level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
    auto& manager = sinks::BusinessLoggerManager::get_instance();
    auto logger = manager.get_logger(sinks::BusinessType::ScreenRecord);
    if (logger) {
        logger->log(lvl, fmt, std::forward<Args>(args)...);
    }
}

template<typename... Args>
void desktop_open_fmt(level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
    auto& manager = sinks::BusinessLoggerManager::get_instance();
    auto logger = manager.get_logger(sinks::BusinessType::DesktopOpen);
    if (logger) {
        logger->log(lvl, fmt, std::forward<Args>(args)...);
    }
}

template<typename... Args>
void keyboard_record_fmt(level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
    auto& manager = sinks::BusinessLoggerManager::get_instance();
    auto logger = manager.get_logger(sinks::BusinessType::KeyboardRecord);
    if (logger) {
        logger->log(lvl, fmt, std::forward<Args>(args)...);
    }
}

template<typename... Args>
void sound_record_fmt(level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
    auto& manager = sinks::BusinessLoggerManager::get_instance();
    auto logger = manager.get_logger(sinks::BusinessType::SoundRecord);
    if (logger) {
        logger->log(lvl, fmt, std::forward<Args>(args)...);
    }
}

}  // namespace business_log

}  // namespace spdlog

// Include implementation
#include "business_logger_manager-inl.h"
