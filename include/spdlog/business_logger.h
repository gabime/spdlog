#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace business_logger {

enum class BusinessType {
    SCREEN_RECORD,
    DESKTOP_OPEN,
    KEYBOARD_RECORD,
    VOLUME_RECORD
};

}

namespace std {
    template<>
    struct hash<business_logger::BusinessType> {
        size_t operator()(const business_logger::BusinessType& type) const {
            return static_cast<size_t>(type);
        }
    };
}

namespace business_logger {

class BusinessLogger {
public:
    static BusinessLogger& getInstance() {
        static BusinessLogger instance;
        return instance;
    }

    void initialize(const std::string& log_dir = "logs", 
                    size_t max_file_size = 30 * 1024 * 1024,
                    size_t max_files = 3) {
        std::lock_guard<std::mutex> lock(mutex_);
        log_dir_ = log_dir;
        max_file_size_ = max_file_size;
        max_files_ = max_files;
        initialized_ = true;
    }

    void log(BusinessType type, spdlog::level::level_enum level, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) {
            spdlog::warn("BusinessLogger not initialized. Using default settings.");
            initialize();
        }

        auto logger = getLogger(type);
        if (logger) {
            logger->log(level, message);
        }
    }

    template<typename... Args>
    void log(BusinessType type, spdlog::level::level_enum level, const std::string& fmt, Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) {
            spdlog::warn("BusinessLogger not initialized. Using default settings.");
            initialize();
        }

        auto logger = getLogger(type);
        if (logger) {
            logger->log(level, fmt, std::forward<Args>(args)...);
        }
    }

    std::shared_ptr<spdlog::logger> getLogger(BusinessType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = loggers_.find(type);
        if (it != loggers_.end()) {
            return it->second;
        }

        auto logger = createLogger(type);
        if (logger) {
            loggers_[type] = logger;
        }
        return logger;
    }

    void flush(BusinessType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto logger = getLogger(type);
        if (logger) {
            logger->flush();
        }
    }

    void flushAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : loggers_) {
            if (pair.second) {
                pair.second->flush();
            }
        }
    }

    void setLogLevel(BusinessType type, spdlog::level::level_enum level) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto logger = getLogger(type);
        if (logger) {
            logger->set_level(level);
        }
    }

    void setAllLogLevels(spdlog::level::level_enum level) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : loggers_) {
            if (pair.second) {
                pair.second->set_level(level);
            }
        }
    }

private:
    BusinessLogger() = default;
    ~BusinessLogger() {
        flushAll();
    }

    BusinessLogger(const BusinessLogger&) = delete;
    BusinessLogger& operator=(const BusinessLogger&) = delete;

    std::shared_ptr<spdlog::logger> createLogger(BusinessType type) {
        std::string filename = getLogFileName(type);
        std::string logger_name = getLoggerName(type);

        try {
            auto logger = spdlog::rotating_logger_mt(
                logger_name,
                filename,
                max_file_size_,
                max_files_
            );
            
            logger->set_level(spdlog::level::debug);
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %v");
            
            return logger;
        } catch (const spdlog::spdlog_ex& ex) {
            spdlog::error("Failed to create logger for {}: {}", logger_name, ex.what());
            return nullptr;
        }
    }

    std::string getLogFileName(BusinessType type) {
        std::string base_name;
        switch (type) {
            case BusinessType::SCREEN_RECORD:
                base_name = "screen_record";
                break;
            case BusinessType::DESKTOP_OPEN:
                base_name = "desktop_open";
                break;
            case BusinessType::KEYBOARD_RECORD:
                base_name = "keyboard_record";
                break;
            case BusinessType::VOLUME_RECORD:
                base_name = "volume_record";
                break;
            default:
                base_name = "unknown";
                break;
        }
        return log_dir_ + "/" + base_name + ".log";
    }

    std::string getLoggerName(BusinessType type) {
        std::string name;
        switch (type) {
            case BusinessType::SCREEN_RECORD:
                name = "screen_record_logger";
                break;
            case BusinessType::DESKTOP_OPEN:
                name = "desktop_open_logger";
                break;
            case BusinessType::KEYBOARD_RECORD:
                name = "keyboard_record_logger";
                break;
            case BusinessType::VOLUME_RECORD:
                name = "volume_record_logger";
                break;
            default:
                name = "unknown_logger";
                break;
        }
        return name;
    }

    std::mutex mutex_;
    std::unordered_map<BusinessType, std::shared_ptr<spdlog::logger>> loggers_;
    std::string log_dir_ = "logs";
    size_t max_file_size_ = 30 * 1024 * 1024;
    size_t max_files_ = 3;
    bool initialized_ = false;
};

#define BUSINESS_LOG_TRACE(type, ...) business_logger::BusinessLogger::getInstance().log(type, spdlog::level::trace, __VA_ARGS__)
#define BUSINESS_LOG_DEBUG(type, ...) business_logger::BusinessLogger::getInstance().log(type, spdlog::level::debug, __VA_ARGS__)
#define BUSINESS_LOG_INFO(type, ...) business_logger::BusinessLogger::getInstance().log(type, spdlog::level::info, __VA_ARGS__)
#define BUSINESS_LOG_WARN(type, ...) business_logger::BusinessLogger::getInstance().log(type, spdlog::level::warn, __VA_ARGS__)
#define BUSINESS_LOG_ERROR(type, ...) business_logger::BusinessLogger::getInstance().log(type, spdlog::level::err, __VA_ARGS__)
#define BUSINESS_LOG_CRITICAL(type, ...) business_logger::BusinessLogger::getInstance().log(type, spdlog::level::critical, __VA_ARGS__)

}
