// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "business_logger.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include <mutex>

namespace spdlog {

business_logger_manager &business_logger_manager::instance() {
    static business_logger_manager manager;
    return manager;
}

business_logger_manager::business_logger_manager()
    : log_dir_("logs"),
      initialized_(false) {}

business_logger_manager::~business_logger_manager() {
    shutdown();
}

void business_logger_manager::init(const std::string &log_dir) {
    std::lock_guard<std::mutex> lock(mutex_);
    log_dir_ = log_dir;
    initialized_ = true;
}

void business_logger_manager::set_log_dir(const std::string &log_dir) {
    std::lock_guard<std::mutex> lock(mutex_);
    log_dir_ = log_dir;
}

std::string business_logger_manager::get_log_dir() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return log_dir_;
}

std::string business_logger_manager::business_type_to_string(BusinessType type) const {
    switch (type) {
        case BusinessType::ScreenRecording:
            return "screen_recording";
        case BusinessType::OpenDesktop:
            return "open_desktop";
        case BusinessType::KeyboardRecording:
            return "keyboard_recording";
        case BusinessType::AudioRecording:
            return "audio_recording";
        default:
            return "unknown";
    }
}

std::shared_ptr<logger> business_logger_manager::create_business_logger(BusinessType type) {
    std::string business_name = business_type_to_string(type);
    std::string logger_name = "business_" + business_name;
    std::string log_file = log_dir_ + "/" + business_name + ".log";

    auto sink = std::make_shared<sinks::rotating_file_sink_mt>(log_file, MAX_FILE_SIZE, MAX_FILES);
    auto logger = std::make_shared<spdlog::logger>(logger_name, sink);
    
    logger->set_level(level::trace);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
    logger->flush_on(level::info);
    
    return logger;
}

std::shared_ptr<logger> business_logger_manager::get_logger(BusinessType business_type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        init(log_dir_);
    }
    
    auto it = loggers_.find(business_type);
    if (it != loggers_.end()) {
        return it->second;
    }
    
    auto logger = create_business_logger(business_type);
    loggers_[business_type] = logger;
    return logger;
}

template <typename... Args>
void business_logger_manager::log(BusinessType business_type, level::level_enum lvl, format_string_t<Args...> fmt, Args &&...args) {
    auto logger = get_logger(business_type);
    logger->log(lvl, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void business_logger_manager::trace(BusinessType business_type, format_string_t<Args...> fmt, Args &&...args) {
    auto logger = get_logger(business_type);
    logger->trace(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void business_logger_manager::debug(BusinessType business_type, format_string_t<Args...> fmt, Args &&...args) {
    auto logger = get_logger(business_type);
    logger->debug(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void business_logger_manager::info(BusinessType business_type, format_string_t<Args...> fmt, Args &&...args) {
    auto logger = get_logger(business_type);
    logger->info(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void business_logger_manager::warn(BusinessType business_type, format_string_t<Args...> fmt, Args &&...args) {
    auto logger = get_logger(business_type);
    logger->warn(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void business_logger_manager::error(BusinessType business_type, format_string_t<Args...> fmt, Args &&...args) {
    auto logger = get_logger(business_type);
    logger->error(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void business_logger_manager::critical(BusinessType business_type, format_string_t<Args...> fmt, Args &&...args) {
    auto logger = get_logger(business_type);
    logger->critical(fmt, std::forward<Args>(args)...);
}

void business_logger_manager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    loggers_.clear();
    initialized_ = false;
}

}  // namespace spdlog
