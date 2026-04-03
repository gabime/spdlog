// Copyright(c) 2024.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <unordered_map>
#include <memory>
#include <string>

namespace spdlog {

enum class BusinessType {
    ScreenRecording = 0,
    OpenDesktop = 1,
    KeyboardRecording = 2,
    AudioRecording = 3
};

class BusinessLogger {
public:
    static BusinessLogger& instance();

    BusinessLogger(const BusinessLogger&) = delete;
    BusinessLogger& operator=(const BusinessLogger&) = delete;

    void init(const std::string& log_dir = "logs");

    void log(BusinessType business, level::level_enum lvl, std::string_view msg);

    template <typename... Args>
    void log(BusinessType business, level::level_enum lvl, std::string_view fmt, Args&&... args);

    template <typename... Args>
    void info(BusinessType business, std::string_view fmt, Args&&... args);

    template <typename... Args>
    void warn(BusinessType business, std::string_view fmt, Args&&... args);

    template <typename... Args>
    void error(BusinessType business, std::string_view fmt, Args&&... args);

    template <typename... Args>
    void debug(BusinessType business, std::string_view fmt, Args&&... args);

    template <typename... Args>
    void critical(BusinessType business, std::string_view fmt, Args&&... args);

    std::shared_ptr<logger> get_logger(BusinessType business);

private:
    BusinessLogger() = default;
    ~BusinessLogger() = default;

    std::string get_business_name(BusinessType business);
    std::string get_log_filename(BusinessType business);

    std::unordered_map<BusinessType, std::shared_ptr<logger>> loggers_;
    std::string log_dir_;
    bool initialized_ = false;
};

template <typename... Args>
void BusinessLogger::log(BusinessType business, level::level_enum lvl, std::string_view fmt, Args&&... args) {
    auto logger = get_logger(business);
    if (logger) {
        logger->log(lvl, fmt, std::forward<Args>(args)...);
    }
}

template <typename... Args>
void BusinessLogger::info(BusinessType business, std::string_view fmt, Args&&... args) {
    log(business, level::info, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void BusinessLogger::warn(BusinessType business, std::string_view fmt, Args&&... args) {
    log(business, level::warn, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void BusinessLogger::error(BusinessType business, std::string_view fmt, Args&&... args) {
    log(business, level::err, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void BusinessLogger::debug(BusinessType business, std::string_view fmt, Args&&... args) {
    log(business, level::debug, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void BusinessLogger::critical(BusinessType business, std::string_view fmt, Args&&... args) {
    log(business, level::critical, fmt, std::forward<Args>(args)...);
}

} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "business_logger-inl.h"
#endif

