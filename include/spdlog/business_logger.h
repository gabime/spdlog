// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <unordered_map>
#include <string>

namespace spdlog {

// 业务类型枚举
enum class BusinessType {
    SCREEN_RECORD,    // 录制屏幕
    DESKTOP_OPEN,     // 打开桌面
    KEYBOARD_RECORD,  // 录制键盘
    AUDIO_RECORD      // 录制声音
};

// 业务日志管理器类
class BusinessLogger {
private:
    // 日志文件大小限制：30MB
    static constexpr size_t MAX_FILE_SIZE = 30 * 1024 * 1024;
    // 日志文件数量限制：3个
    static constexpr size_t MAX_FILES = 3;
    // 日志文件路径前缀
    static constexpr const char* LOG_DIR = "logs/";
    
    // 业务日志记录器映射
    std::unordered_map<BusinessType, std::shared_ptr<logger>> loggers_;
    
    // 单例实例
    static BusinessLogger* instance_;
    
    // 私有构造函数
    BusinessLogger() {
        init_loggers();
    }
    
    // 初始化所有业务日志记录器
    void init_loggers();
    
    // 根据业务类型获取日志文件名
    std::string get_log_filename(BusinessType type);
    
    // 根据业务类型获取日志记录器名称
    std::string get_logger_name(BusinessType type);
    
public:
    // 获取单例实例
    static BusinessLogger* get_instance();
    
    // 写入日志
    template<typename... Args>
    void log(BusinessType type, level::level_enum level, const char* fmt, Args&&... args);
    
    // 写入不同级别的日志
    template<typename... Args>
    void trace(BusinessType type, const char* fmt, Args&&... args);
    
    template<typename... Args>
    void debug(BusinessType type, const char* fmt, Args&&... args);
    
    template<typename... Args>
    void info(BusinessType type, const char* fmt, Args&&... args);
    
    template<typename... Args>
    void warn(BusinessType type, const char* fmt, Args&&... args);
    
    template<typename... Args>
    void error(BusinessType type, const char* fmt, Args&&... args);
    
    template<typename... Args>
    void critical(BusinessType type, const char* fmt, Args&&... args);
};

// 实现部分
template<typename... Args>
void BusinessLogger::log(BusinessType type, level::level_enum level, const char* fmt, Args&&... args) {
    auto it = loggers_.find(type);
    if (it != loggers_.end()) {
        it->second->log(level, fmt, std::forward<Args>(args)...);
    }
}

template<typename... Args>
void BusinessLogger::trace(BusinessType type, const char* fmt, Args&&... args) {
    log(type, level::trace, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void BusinessLogger::debug(BusinessType type, const char* fmt, Args&&... args) {
    log(type, level::debug, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void BusinessLogger::info(BusinessType type, const char* fmt, Args&&... args) {
    log(type, level::info, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void BusinessLogger::warn(BusinessType type, const char* fmt, Args&&... args) {
    log(type, level::warn, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void BusinessLogger::error(BusinessType type, const char* fmt, Args&&... args) {
    log(type, level::err, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void BusinessLogger::critical(BusinessType type, const char* fmt, Args&&... args) {
    log(type, level::critical, fmt, std::forward<Args>(args)...);
}

} // namespace spdlog
