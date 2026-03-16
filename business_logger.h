#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <unordered_map>
#include <string>
#include <memory>

namespace spdlog {
namespace business {

// 业务类型枚举
enum class BusinessType {
    SCREEN_RECORD,    // 录制屏幕
    DESKTOP_OPEN,     // 打开桌面
    KEYBOARD_RECORD,  // 录制键盘
    AUDIO_RECORD      // 录制声音
};

// 业务日志管理类
class BusinessLogger {
private:
    static constexpr size_t MAX_FILE_SIZE = 30 * 1024 * 1024; // 30MB
    static constexpr size_t MAX_FILES = 3;                   // 最多保留3个文件
    
    std::unordered_map<BusinessType, std::shared_ptr<spdlog::logger>> loggers_;
    
    // 私有构造函数，单例模式
    BusinessLogger() {
        init_loggers();
    }
    
    // 初始化所有业务日志器
    void init_loggers() {
        // 录制屏幕日志
        auto screen_logger = spdlog::rotating_logger_mt(
            "screen_record", 
            "logs/screen_record.log", 
            MAX_FILE_SIZE, 
            MAX_FILES
        );
        screen_logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
        loggers_[BusinessType::SCREEN_RECORD] = screen_logger;
        
        // 打开桌面日志
        auto desktop_logger = spdlog::rotating_logger_mt(
            "desktop_open", 
            "logs/desktop_open.log", 
            MAX_FILE_SIZE, 
            MAX_FILES
        );
        desktop_logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
        loggers_[BusinessType::DESKTOP_OPEN] = desktop_logger;
        
        // 录制键盘日志
        auto keyboard_logger = spdlog::rotating_logger_mt(
            "keyboard_record", 
            "logs/keyboard_record.log", 
            MAX_FILE_SIZE, 
            MAX_FILES
        );
        keyboard_logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
        loggers_[BusinessType::KEYBOARD_RECORD] = keyboard_logger;
        
        // 录制声音日志
        auto audio_logger = spdlog::rotating_logger_mt(
            "audio_record", 
            "logs/audio_record.log", 
            MAX_FILE_SIZE, 
            MAX_FILES
        );
        audio_logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
        loggers_[BusinessType::AUDIO_RECORD] = audio_logger;
    }
    
public:
    // 获取单例实例
    static BusinessLogger& instance() {
        static BusinessLogger instance;
        return instance;
    }
    
    // 禁止拷贝构造和赋值
    BusinessLogger(const BusinessLogger&) = delete;
    BusinessLogger& operator=(const BusinessLogger&) = delete;
    
    // 获取指定业务的日志器
    std::shared_ptr<spdlog::logger> get_logger(BusinessType business_type) {
        auto it = loggers_.find(business_type);
        if (it != loggers_.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    // 快捷日志函数
    template <typename... Args>
    static void log(BusinessType business_type, level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        auto logger = instance().get_logger(business_type);
        if (logger) {
            logger->log(lvl, fmt, std::forward<Args>(args)...);
        }
    }
    
    template <typename... Args>
    static void trace(BusinessType business_type, format_string_t<Args...> fmt, Args&&... args) {
        log(business_type, level::trace, fmt, std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    static void debug(BusinessType business_type, format_string_t<Args...> fmt, Args&&... args) {
        log(business_type, level::debug, fmt, std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    static void info(BusinessType business_type, format_string_t<Args...> fmt, Args&&... args) {
        log(business_type, level::info, fmt, std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    static void warn(BusinessType business_type, format_string_t<Args...> fmt, Args&&... args) {
        log(business_type, level::warn, fmt, std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    static void error(BusinessType business_type, format_string_t<Args...> fmt, Args&&... args) {
        log(business_type, level::error, fmt, std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    static void critical(BusinessType business_type, format_string_t<Args...> fmt, Args&&... args) {
        log(business_type, level::critical, fmt, std::forward<Args>(args)...);
    }
};

} // namespace business
} // namespace spdlog