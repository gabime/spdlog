#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

enum class BusinessType {
    RECORD_SCREEN,
    OPEN_DESKTOP,
    RECORD_KEYBOARD,
    RECORD_AUDIO
};

class BusinessLogger {
public:
    static BusinessLogger& getInstance();

    BusinessLogger(const BusinessLogger&) = delete;
    BusinessLogger& operator=(const BusinessLogger&) = delete;

    void init(const std::string& log_dir = "logs");

    void log(BusinessType type, spdlog::level::level_enum level, const std::string& message);

    template<typename... Args>
    void info(BusinessType type, const std::string& fmt, Args&&... args) {
        log(type, spdlog::level::info, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void warn(BusinessType type, const std::string& fmt, Args&&... args) {
        log(type, spdlog::level::warn, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void error(BusinessType type, const std::string& fmt, Args&&... args) {
        log(type, spdlog::level::err, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void debug(BusinessType type, const std::string& fmt, Args&&... args) {
        log(type, spdlog::level::debug, fmt::format(fmt, std::forward<Args>(args)...));
    }

private:
    BusinessLogger() = default;
    ~BusinessLogger() = default;

    std::string getBusinessName(BusinessType type);
    std::shared_ptr<spdlog::logger> getLogger(BusinessType type);

    std::string log_dir_;
    std::unordered_map<BusinessType, std::shared_ptr<spdlog::logger>> loggers_;
    const size_t max_file_size_ = 30 * 1024 * 1024;
    const size_t max_files_ = 3;
};
