#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <mutex>

namespace spdlog {

enum class BusinessType {
    ScreenRecording,
    DesktopOpening,
    KeyboardRecording,
    AudioRecording
};

class BusinessLogger {
public:
    static BusinessLogger& instance() {
        static BusinessLogger instance;
        return instance;
    }

    void init(const std::string& log_dir = "logs") {
        std::lock_guard<std::mutex> lock(mutex_);
        log_dir_ = log_dir;
        
        init_logger(BusinessType::ScreenRecording, "screen_recording");
        init_logger(BusinessType::DesktopOpening, "desktop_opening");
        init_logger(BusinessType::KeyboardRecording, "keyboard_recording");
        init_logger(BusinessType::AudioRecording, "audio_recording");
    }

    template<typename... Args>
    void log(BusinessType type, level::level_enum level, spdlog::format_string_t<Args...> fmt, Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = loggers_.find(type);
        if (it != loggers_.end()) {
            it->second->log(level, fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    void info(BusinessType type, spdlog::format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::info, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(BusinessType type, spdlog::format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::warn, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(BusinessType type, spdlog::format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::err, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(BusinessType type, spdlog::format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::debug, fmt, std::forward<Args>(args)...);
    }

    std::shared_ptr<logger> get_logger(BusinessType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = loggers_.find(type);
        if (it != loggers_.end()) {
            return it->second;
        }
        return nullptr;
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : loggers_) {
            pair.second->flush();
        }
        loggers_.clear();
    }

private:
    BusinessLogger() = default;
    ~BusinessLogger() {
        shutdown();
    }

    BusinessLogger(const BusinessLogger&) = delete;
    BusinessLogger& operator=(const BusinessLogger&) = delete;

    void init_logger(BusinessType type, const std::string& name) {
        std::string file_path = log_dir_ + "/" + name + ".log";
        auto logger = rotating_logger_mt(name, file_path, 30 * 1024 * 1024, 3);
        logger->set_level(level::debug);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
        logger->flush_on(level::info);
        loggers_[type] = logger;
    }

    std::unordered_map<BusinessType, std::shared_ptr<logger>> loggers_;
    std::string log_dir_;
    std::mutex mutex_;
};

}
