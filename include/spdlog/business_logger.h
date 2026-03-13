#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <map>
#include <memory>
#include <string>
#include <mutex>

namespace spdlog {

enum class BusinessType {
    ScreenRecording = 0,
    OpenDesktop = 1,
    KeyboardRecording = 2,
    SoundRecording = 3
};

class BusinessLogger {
public:
    static BusinessLogger& instance() {
        static BusinessLogger logger;
        return logger;
    }

    void init(const std::string& log_dir = "logs") {
        std::lock_guard<std::mutex> lock(mutex_);
        log_dir_ = log_dir;
        
        for (const auto& pair : business_names_) {
            auto logger = create_business_logger(pair.first);
            loggers_[pair.first] = logger;
        }
    }

    std::shared_ptr<logger> get_logger(BusinessType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = loggers_.find(type);
        if (it != loggers_.end()) {
            return it->second;
        }
        return nullptr;
    }

    template<typename... Args>
    void info(BusinessType type, fmt::format_string<Args...> fmt, Args&&... args) {
        auto logger = get_logger(type);
        if (logger) {
            logger->info(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    void debug(BusinessType type, fmt::format_string<Args...> fmt, Args&&... args) {
        auto logger = get_logger(type);
        if (logger) {
            logger->debug(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    void warn(BusinessType type, fmt::format_string<Args...> fmt, Args&&... args) {
        auto logger = get_logger(type);
        if (logger) {
            logger->warn(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    void error(BusinessType type, fmt::format_string<Args...> fmt, Args&&... args) {
        auto logger = get_logger(type);
        if (logger) {
            logger->error(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    void critical(BusinessType type, fmt::format_string<Args...> fmt, Args&&... args) {
        auto logger = get_logger(type);
        if (logger) {
            logger->critical(fmt, std::forward<Args>(args)...);
        }
    }

private:
    BusinessLogger() {
        business_names_[BusinessType::ScreenRecording] = "screen_recording";
        business_names_[BusinessType::OpenDesktop] = "open_desktop";
        business_names_[BusinessType::KeyboardRecording] = "keyboard_recording";
        business_names_[BusinessType::SoundRecording] = "sound_recording";
    }

    ~BusinessLogger() {
        spdlog::shutdown();
    }

    BusinessLogger(const BusinessLogger&) = delete;
    BusinessLogger& operator=(const BusinessLogger&) = delete;

    std::shared_ptr<logger> create_business_logger(BusinessType type) {
        const std::string& name = business_names_[type];
        std::string filename = log_dir_ + "/" + name + ".log";
        
        const size_t max_size = 30 * 1024 * 1024; 
        const size_t max_files = 3;
        
        auto logger = rotating_logger_mt(name, filename, max_size, max_files);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
        logger->flush_on(level::info);
        return logger;
    }

    std::map<BusinessType, std::string> business_names_;
    std::map<BusinessType, std::shared_ptr<logger>> loggers_;
    std::string log_dir_;
    std::mutex mutex_;
};

} 
