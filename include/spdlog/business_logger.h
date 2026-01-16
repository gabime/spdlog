#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <memory>
#include <unordered_map>
#include <mutex>

namespace spdlog {
namespace business {

enum class BusinessType {
    SCREEN_RECORD,   // 录制屏幕
    DESKTOP_OPEN,    // 打开桌面
    KEYBOARD_RECORD, // 录制键盘
    AUDIO_RECORD     // 录制声音
};

struct BusinessTypeHash {
    std::size_t operator()(BusinessType type) const {
        return static_cast<std::size_t>(type);
    }
};

class BusinessLoggerManager {
private:
    static constexpr size_t MAX_FILE_SIZE = 30 * 1024 * 1024; // 30MB
    static constexpr size_t MAX_FILES = 3;
    
    std::unordered_map<BusinessType, std::shared_ptr<logger>, BusinessTypeHash> loggers_;
    std::mutex mutex_;
    
    BusinessLoggerManager() = default;
    
    std::string get_logger_name(BusinessType type) {
        switch (type) {
            case BusinessType::SCREEN_RECORD:
                return "screen_record";
            case BusinessType::DESKTOP_OPEN:
                return "desktop_open";
            case BusinessType::KEYBOARD_RECORD:
                return "keyboard_record";
            case BusinessType::AUDIO_RECORD:
                return "audio_record";
            default:
                return "unknown";
        }
    }
    
    std::string get_log_file_name(BusinessType type) {
        return "logs/" + get_logger_name(type) + ".log";
    }
    
public:
    static BusinessLoggerManager& instance() {
        static BusinessLoggerManager instance;
        return instance;
    }
    
    template<typename... Args>
    void log(BusinessType type, level::level_enum log_level, const char* fmt, Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = loggers_.find(type);
        if (it == loggers_.end()) {
            auto logger = rotating_logger_mt(
                get_logger_name(type),
                get_log_file_name(type),
                MAX_FILE_SIZE,
                MAX_FILES
            );
            it = loggers_.emplace(type, std::move(logger)).first;
        }
        
        it->second->log(log_level, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void trace(BusinessType type, const char* fmt, Args&&... args) {
        log(type, level::trace, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void debug(BusinessType type, const char* fmt, Args&&... args) {
        log(type, level::debug, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void info(BusinessType type, const char* fmt, Args&&... args) {
        log(type, level::info, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void warn(BusinessType type, const char* fmt, Args&&... args) {
        log(type, level::warn, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void error(BusinessType type, const char* fmt, Args&&... args) {
        log(type, level::err, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void critical(BusinessType type, const char* fmt, Args&&... args) {
        log(type, level::critical, fmt, std::forward<Args>(args)...);
    }
};

} // namespace business
} // namespace spdlog
