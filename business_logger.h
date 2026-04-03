#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>

enum class BusinessType {
    ScreenRecording,
    DesktopOpen,
    KeyboardRecording,
    AudioRecording
};

class BusinessLogger {
public:
    static BusinessLogger& instance();

    void init(const std::string& log_dir = "logs");
    
    void log(BusinessType type, spdlog::level::level_enum level, const std::string& message);
    
    void info(BusinessType type, const std::string& message);
    void warn(BusinessType type, const std::string& message);
    void error(BusinessType type, const std::string& message);
    void debug(BusinessType type, const std::string& message);
    void critical(BusinessType type, const std::string& message);

    std::string get_business_name(BusinessType type) const;

private:
    BusinessLogger() = default;
    ~BusinessLogger() = default;
    BusinessLogger(const BusinessLogger&) = delete;
    BusinessLogger& operator=(const BusinessLogger&) = delete;

    std::shared_ptr<spdlog::logger> get_or_create_logger(BusinessType type);

    std::string log_dir_;
    std::unordered_map<BusinessType, std::shared_ptr<spdlog::logger>> loggers_;
    std::mutex mutex_;
    bool initialized_ = false;
};
