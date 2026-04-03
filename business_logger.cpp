#include "business_logger.h"
#include <stdexcept>

BusinessLogger& BusinessLogger::instance() {
    static BusinessLogger logger;
    return logger;
}

void BusinessLogger::init(const std::string& log_dir) {
    std::lock_guard<std::mutex> lock(mutex_);
    log_dir_ = log_dir;
    initialized_ = true;
}

std::string BusinessLogger::get_business_name(BusinessType type) const {
    switch (type) {
        case BusinessType::ScreenRecording:
            return "screen_recording";
        case BusinessType::DesktopOpen:
            return "desktop_open";
        case BusinessType::KeyboardRecording:
            return "keyboard_recording";
        case BusinessType::AudioRecording:
            return "audio_recording";
        default:
            return "unknown";
    }
}

std::shared_ptr<spdlog::logger> BusinessLogger::get_or_create_logger(BusinessType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        throw std::runtime_error("BusinessLogger not initialized. Call init() first.");
    }
    
    auto it = loggers_.find(type);
    if (it != loggers_.end()) {
        return it->second;
    }
    
    std::string business_name = get_business_name(type);
    std::string log_file = log_dir_ + "/" + business_name + ".log";
    
    const size_t max_file_size = 30 * 1024 * 1024;
    const size_t max_files = 3;
    
    auto logger = spdlog::rotating_logger_mt(business_name, log_file, max_file_size, max_files);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::info);
    
    loggers_[type] = logger;
    return logger;
}

void BusinessLogger::log(BusinessType type, spdlog::level::level_enum level, const std::string& message) {
    auto logger = get_or_create_logger(type);
    logger->log(level, message);
}

void BusinessLogger::info(BusinessType type, const std::string& message) {
    log(type, spdlog::level::info, message);
}

void BusinessLogger::warn(BusinessType type, const std::string& message) {
    log(type, spdlog::level::warn, message);
}

void BusinessLogger::error(BusinessType type, const std::string& message) {
    log(type, spdlog::level::err, message);
}

void BusinessLogger::debug(BusinessType type, const std::string& message) {
    log(type, spdlog::level::debug, message);
}

void BusinessLogger::critical(BusinessType type, const std::string& message) {
    log(type, spdlog::level::critical, message);
}
