// Copyright(c) 2024.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include "spdlog/business_logger.h"
#endif

#include "spdlog/sinks/rotating_file_sink.h"
#include <filesystem>

namespace spdlog {

BusinessLogger& BusinessLogger::instance() {
    static BusinessLogger inst;
    return inst;
}

void BusinessLogger::init(const std::string& log_dir) {
    if (initialized_) {
        return;
    }

    log_dir_ = log_dir;

    std::filesystem::create_directories(log_dir_);

    const std::size_t max_size = 30 * 1024 * 1024; // 30MB
    const std::size_t max_files = 3;

    auto create_logger = [&](BusinessType business) {
        std::string logger_name = get_business_name(business);
        std::string log_path = get_log_filename(business);

        auto logger = rotating_logger_mt(logger_name, log_path, max_size, max_files);
        logger->set_level(level::trace);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
        logger->flush_on(level::info);
        loggers_[business] = logger;
    };

    create_logger(BusinessType::ScreenRecording);
    create_logger(BusinessType::OpenDesktop);
    create_logger(BusinessType::KeyboardRecording);
    create_logger(BusinessType::AudioRecording);

    initialized_ = true;
}

void BusinessLogger::log(BusinessType business, level::level_enum lvl, std::string_view msg) {
    auto logger = get_logger(business);
    if (logger) {
        logger->log(lvl, msg);
    }
}

std::shared_ptr<logger> BusinessLogger::get_logger(BusinessType business) {
    if (!initialized_) {
        init();
    }

    auto it = loggers_.find(business);
    if (it != loggers_.end()) {
        return it->second;
    }
    return nullptr;
}

std::string BusinessLogger::get_business_name(BusinessType business) {
    switch (business) {
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

std::string BusinessLogger::get_log_filename(BusinessType business) {
    return log_dir_ + "/" + get_business_name(business) + ".log";
}

} // namespace spdlog
