// Copyright(c) 2026.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "business_logger_manager.h"
#include <spdlog/common.h>

#include <cassert>
#include <filesystem>

namespace spdlog {
namespace sinks {

// 单例模式获取实例
inline BusinessLoggerManager& BusinessLoggerManager::get_instance() {
    static BusinessLoggerManager instance;
    return instance;
}

// 初始化日志管理器
inline void BusinessLoggerManager::initialize(const std::string& base_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    base_path_ = base_path;
    
    // 确保日志目录存在
    std::filesystem::path log_dir(base_path);
    if (!std::filesystem::exists(log_dir)) {
        std::filesystem::create_directories(log_dir);
    }
    
    initialized_ = true;
}

// 获取指定业务的logger
inline std::shared_ptr<logger> BusinessLoggerManager::get_logger(BusinessType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        throw spdlog_ex("BusinessLoggerManager not initialized. Call initialize() first.");
    }
    
    auto it = loggers_.find(type);
    if (it != loggers_.end()) {
        return it->second;
    }
    
    // 创建新的logger
    auto new_logger = create_business_logger(type);
    loggers_[type] = new_logger;
    return new_logger;
}

// 记录日志的便捷方法
inline void BusinessLoggerManager::log(BusinessType type, level::level_enum lvl, const std::string& msg) {
    auto logger = get_logger(type);
    if (logger) {
        logger->log(lvl, msg);
    }
}

// 录制屏幕日志
inline void BusinessLoggerManager::screen_record_log(level::level_enum lvl, const std::string& msg) {
    log(BusinessType::ScreenRecord, lvl, msg);
}

// 打开桌面日志
inline void BusinessLoggerManager::desktop_open_log(level::level_enum lvl, const std::string& msg) {
    log(BusinessType::DesktopOpen, lvl, msg);
}

// 录制键盘日志
inline void BusinessLoggerManager::keyboard_record_log(level::level_enum lvl, const std::string& msg) {
    log(BusinessType::KeyboardRecord, lvl, msg);
}

// 录制声音日志
inline void BusinessLoggerManager::sound_record_log(level::level_enum lvl, const std::string& msg) {
    log(BusinessType::SoundRecord, lvl, msg);
}

// 设置所有业务logger的日志级别
inline void BusinessLoggerManager::set_level(level::level_enum lvl) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& [type, logger] : loggers_) {
        if (logger) {
            logger->set_level(lvl);
        }
    }
}

// 刷新所有日志
inline void BusinessLoggerManager::flush_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& [type, logger] : loggers_) {
        if (logger) {
            logger->flush();
        }
    }
}

// 关闭所有logger并清理资源
inline void BusinessLoggerManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    loggers_.clear();
    initialized_ = false;
}

// 创建业务logger
inline std::shared_ptr<logger> BusinessLoggerManager::create_business_logger(BusinessType type) {
    std::string business_name = get_business_name(type);
    std::string filename = get_business_filename(type);
    
    // 创建轮转文件sink：每个文件最大30MB，最多保留3个文件
    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        filename, MAX_FILE_SIZE, MAX_FILES, false);
    
    // 创建logger
    auto new_logger = std::make_shared<logger>(business_name, sink);
    
    // 设置日志格式：[时间] [日志级别] 消息
    new_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    
    // 设置默认日志级别为info
    new_logger->set_level(level::info);
    
    // 注册到spdlog全局注册表，便于管理
    register_logger(new_logger);
    
    return new_logger;
}

// 获取业务类型对应的名称
inline std::string BusinessLoggerManager::get_business_name(BusinessType type) {
    switch (type) {
        case BusinessType::ScreenRecord:
            return "ScreenRecord";
        case BusinessType::DesktopOpen:
            return "DesktopOpen";
        case BusinessType::KeyboardRecord:
            return "KeyboardRecord";
        case BusinessType::SoundRecord:
            return "SoundRecord";
        default:
            return "Unknown";
    }
}

// 获取业务类型对应的日志文件名
inline std::string BusinessLoggerManager::get_business_filename(BusinessType type) {
    std::filesystem::path base(base_path_);
    std::string filename;
    
    switch (type) {
        case BusinessType::ScreenRecord:
            filename = "screen_record.log";
            break;
        case BusinessType::DesktopOpen:
            filename = "desktop_open.log";
            break;
        case BusinessType::KeyboardRecord:
            filename = "keyboard_record.log";
            break;
        case BusinessType::SoundRecord:
            filename = "sound_record.log";
            break;
        default:
            filename = "unknown.log";
            break;
    }
    
    return (base / filename).string();
}

}  // namespace sinks
}  // namespace spdlog
