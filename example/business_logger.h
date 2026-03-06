
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

enum class BusinessType {
    ScreenRecording,
    DesktopOpen,
    KeyboardRecording,
    AudioRecording
};

class BusinessLogger {
public:
    static BusinessLogger& instance() {
        static BusinessLogger logger;
        return logger;
    }

    void init(const std::string& log_dir = "logs") {
        log_dir_ = log_dir;
        
        create_logger(BusinessType::ScreenRecording, "screen_recording");
        create_logger(BusinessType::DesktopOpen, "desktop_open");
        create_logger(BusinessType::KeyboardRecording, "keyboard_recording");
        create_logger(BusinessType::AudioRecording, "audio_recording");
        
        spdlog::set_level(spdlog::level::debug);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    }

    template<typename... Args>
    void log(BusinessType type, spdlog::level::level_enum level, fmt::format_string<Args...> fmt, Args&&... args) {
        auto it = loggers_.find(type);
        if (it != loggers_.end()) {
            it->second->log(level, fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    void info(BusinessType type, fmt::format_string<Args...> fmt, Args&&... args) {
        log(type, spdlog::level::info, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(BusinessType type, fmt::format_string<Args...> fmt, Args&&... args) {
        log(type, spdlog::level::debug, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(BusinessType type, fmt::format_string<Args...> fmt, Args&&... args) {
        log(type, spdlog::level::warn, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(BusinessType type, fmt::format_string<Args...> fmt, Args&&... args) {
        log(type, spdlog::level::err, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void critical(BusinessType type, fmt::format_string<Args...> fmt, Args&&... args) {
        log(type, spdlog::level::critical, fmt, std::forward<Args>(args)...);
    }

    void shutdown() {
        spdlog::shutdown();
        loggers_.clear();
    }

private:
    BusinessLogger() = default;
    ~BusinessLogger() = default;
    BusinessLogger(const BusinessLogger&) = delete;
    BusinessLogger& operator=(const BusinessLogger&) = delete;

    void create_logger(BusinessType type, const std::string& name) {
        std::string file_path = log_dir_ + "/" + name + ".log";
        const std::size_t max_file_size = 30 * 1024 * 1024;
        const std::size_t max_files = 3;

        auto logger = spdlog::rotating_logger_mt(name, file_path, max_file_size, max_files);
        logger->flush_on(spdlog::level::info);
        loggers_[type] = logger;
    }

    std::string log_dir_;
    std::unordered_map<BusinessType, std::shared_ptr<spdlog::logger>> loggers_;
};
