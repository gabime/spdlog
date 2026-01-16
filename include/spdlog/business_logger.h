// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <type_traits>

namespace spdlog {
namespace business {

enum class BusinessType : int {
    SCREEN_RECORD,    
    DESKTOP_OPEN,     
    KEYBOARD_RECORD,  
    AUDIO_RECORD      
};

}
}

namespace std {
template<>
struct hash<spdlog::business::BusinessType> {
    size_t operator()(spdlog::business::BusinessType type) const {
        return hash<std::underlying_type<spdlog::business::BusinessType>::type>()(
            static_cast<std::underlying_type<spdlog::business::BusinessType>::type>(type)
        );
    }
};
}

namespace spdlog {
namespace business {

class BusinessLogger {
private:
    static constexpr size_t MAX_FILE_SIZE = 30 * 1024 * 1024;
    static constexpr size_t MAX_FILES = 3;
    
    std::unordered_map<BusinessType, std::shared_ptr<spdlog::logger>> loggers_;
    std::string log_directory_;
    std::mutex mutex_;
    
    static std::string business_to_name(BusinessType type) {
        switch (type) {
            case BusinessType::SCREEN_RECORD: return "screen_record";
            case BusinessType::DESKTOP_OPEN: return "desktop_open";
            case BusinessType::KEYBOARD_RECORD: return "keyboard_record";
            case BusinessType::AUDIO_RECORD: return "audio_record";
            default: return "unknown";
        }
    }
    
    static std::string business_to_filename(BusinessType type) {
        switch (type) {
            case BusinessType::SCREEN_RECORD: return "screen_record.log";
            case BusinessType::DESKTOP_OPEN: return "desktop_open.log";
            case BusinessType::KEYBOARD_RECORD: return "keyboard_record.log";
            case BusinessType::AUDIO_RECORD: return "audio_record.log";
            default: return "unknown.log";
        }
    }
    
    std::shared_ptr<spdlog::logger> get_or_create_logger(BusinessType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = loggers_.find(type);
        if (it != loggers_.end()) {
            return it->second;
        }
        
        std::string filename = log_directory_ + "/" + business_to_filename(type);
        std::string logger_name = business_to_name(type);
        
        auto logger = spdlog::rotating_logger_mt(
            logger_name,
            filename,
            MAX_FILE_SIZE,
            MAX_FILES
        );
        
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        logger->set_level(spdlog::level::info);
        
        loggers_[type] = logger;
        return logger;
    }
    
    BusinessLogger() : log_directory_("./logs") {}
    
public:
    static BusinessLogger& instance() {
        static BusinessLogger instance;
        return instance;
    }
    
    BusinessLogger(const BusinessLogger&) = delete;
    BusinessLogger& operator=(const BusinessLogger&) = delete;
    
    void set_log_directory(const std::string& directory) {
        std::lock_guard<std::mutex> lock(mutex_);
        log_directory_ = directory;
    }
    
    void log(BusinessType type, spdlog::level::level_enum level, const std::string& msg) {
        auto logger = get_or_create_logger(type);
        logger->log(level, msg);
    }
    
    void trace(BusinessType type, const std::string& msg) {
        log(type, spdlog::level::trace, msg);
    }
    
    void debug(BusinessType type, const std::string& msg) {
        log(type, spdlog::level::debug, msg);
    }
    
    void info(BusinessType type, const std::string& msg) {
        log(type, spdlog::level::info, msg);
    }
    
    void warn(BusinessType type, const std::string& msg) {
        log(type, spdlog::level::warn, msg);
    }
    
    void error(BusinessType type, const std::string& msg) {
        log(type, spdlog::level::err, msg);
    }
    
    void critical(BusinessType type, const std::string& msg) {
        log(type, spdlog::level::critical, msg);
    }
    
    template<typename... Args>
    void logf(BusinessType type, spdlog::level::level_enum level, const char* fmt, const Args&... args) {
        auto logger = get_or_create_logger(type);
        logger->log(level, fmt, args...);
    }
    
    template<typename... Args>
    void infof(BusinessType type, const char* fmt, const Args&... args) {
        logf(type, spdlog::level::info, fmt, args...);
    }
    
    template<typename... Args>
    void errorf(BusinessType type, const char* fmt, const Args&... args) {
        logf(type, spdlog::level::err, fmt, args...);
    }
    
    template<typename... Args>
    void warnf(BusinessType type, const char* fmt, const Args&... args) {
        logf(type, spdlog::level::warn, fmt, args...);
    }
    
    void flush_all() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : loggers_) {
            pair.second->flush();
        }
    }
};

}
}
