#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <string>
#include <memory>

namespace spdlog {
namespace business {

enum class BusinessType {
    ScreenRecording,
    DesktopOpening,
    KeyboardRecording,
    AudioRecording
};

class BusinessLogger {
public:
    static void init() {
        create_logger(BusinessType::ScreenRecording, "screen_recording");
        create_logger(BusinessType::DesktopOpening, "desktop_opening");
        create_logger(BusinessType::KeyboardRecording, "keyboard_recording");
        create_logger(BusinessType::AudioRecording, "audio_recording");
    }

    static std::shared_ptr<logger> get_logger(BusinessType type) {
        std::string logger_name = get_logger_name(type);
        return spdlog::get(logger_name);
    }

    template<typename... Args>
    static void log(BusinessType type, level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        auto logger = get_logger(type);
        if (logger) {
            logger->log(lvl, fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void trace(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::trace, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void debug(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::debug, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void info(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::info, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void warn(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::warn, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::err, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void critical(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::critical, fmt, std::forward<Args>(args)...);
    }

private:
    static constexpr std::size_t MAX_FILE_SIZE = 30 * 1024 * 1024;
    static constexpr std::size_t MAX_FILES = 3;

    static std::string get_logger_name(BusinessType type) {
        switch (type) {
            case BusinessType::ScreenRecording:
                return "screen_recording_logger";
            case BusinessType::DesktopOpening:
                return "desktop_opening_logger";
            case BusinessType::KeyboardRecording:
                return "keyboard_recording_logger";
            case BusinessType::AudioRecording:
                return "audio_recording_logger";
            default:
                return "unknown_logger";
        }
    }

    static std::string get_log_filename(const std::string& name) {
        return "logs/" + name + ".log";
    }

    static void create_logger(BusinessType type, const std::string& name) {
        std::string logger_name = get_logger_name(type);
        if (spdlog::get(logger_name)) {
            return;
        }

        std::string filename = get_log_filename(name);
        auto logger = rotating_logger_mt(logger_name, filename, MAX_FILE_SIZE, MAX_FILES);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] %v");
        logger->set_level(level::trace);
        logger->flush_on(level::info);
    }
};

}
}

#define LOG_SCREEN_RECORDING(lvl, ...) \
    spdlog::business::BusinessLogger::log(spdlog::business::BusinessType::ScreenRecording, spdlog::level::lvl, __VA_ARGS__)

#define LOG_DESKTOP_OPENING(lvl, ...) \
    spdlog::business::BusinessLogger::log(spdlog::business::BusinessType::DesktopOpening, spdlog::level::lvl, __VA_ARGS__)

#define LOG_KEYBOARD_RECORDING(lvl, ...) \
    spdlog::business::BusinessLogger::log(spdlog::business::BusinessType::KeyboardRecording, spdlog::level::lvl, __VA_ARGS__)

#define LOG_AUDIO_RECORDING(lvl, ...) \
    spdlog::business::BusinessLogger::log(spdlog::business::BusinessType::AudioRecording, spdlog::level::lvl, __VA_ARGS__)
