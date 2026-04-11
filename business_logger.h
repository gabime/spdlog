#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

enum class BusinessType {
    RECORD_SCREEN,
    OPEN_DESKTOP,
    RECORD_KEYBOARD,
    RECORD_AUDIO
};

class BusinessLogger {
private:
    std::unordered_map<BusinessType, std::shared_ptr<spdlog::logger>> loggers_;
    std::string log_dir_;
    const size_t max_file_size_;
    const size_t max_files_;

    std::string get_business_name(BusinessType type) {
        switch (type) {
            case BusinessType::RECORD_SCREEN:
                return "record_screen";
            case BusinessType::OPEN_DESKTOP:
                return "open_desktop";
            case BusinessType::RECORD_KEYBOARD:
                return "record_keyboard";
            case BusinessType::RECORD_AUDIO:
                return "record_audio";
            default:
                return "unknown";
        }
    }

    std::shared_ptr<spdlog::logger> create_logger(BusinessType type) {
        std::string business_name = get_business_name(type);
        std::string log_path = log_dir_ + "/" + business_name + ".log";
        
        auto logger = spdlog::rotating_logger_mt(
            business_name,
            log_path,
            max_file_size_,
            max_files_
        );
        
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::info);
        
        return logger;
    }

public:
    BusinessLogger(const std::string& log_dir = "logs", 
                   size_t max_file_size = 30 * 1024 * 1024, 
                   size_t max_files = 3)
        : log_dir_(log_dir)
        , max_file_size_(max_file_size)
        , max_files_(max_files) {
    }

    void create_directory(const std::string& path) {
#ifdef _WIN32
        _mkdir(path.c_str());
#else
        mkdir(path.c_str(), 0755);
#endif
    }

    void init() {
        create_directory(log_dir_);
        loggers_[BusinessType::RECORD_SCREEN] = create_logger(BusinessType::RECORD_SCREEN);
        loggers_[BusinessType::OPEN_DESKTOP] = create_logger(BusinessType::OPEN_DESKTOP);
        loggers_[BusinessType::RECORD_KEYBOARD] = create_logger(BusinessType::RECORD_KEYBOARD);
        loggers_[BusinessType::RECORD_AUDIO] = create_logger(BusinessType::RECORD_AUDIO);
    }

    std::shared_ptr<spdlog::logger> get_logger(BusinessType type) {
        auto it = loggers_.find(type);
        if (it != loggers_.end()) {
            return it->second;
        }
        return nullptr;
    }

    template<typename... Args>
    void log(BusinessType type, spdlog::level::level_enum level, fmt::format_string<Args...> fmt, Args&&... args) {
        auto logger = get_logger(type);
        if (logger) {
            logger->log(level, fmt, std::forward<Args>(args)...);
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

    void flush_all() {
        for (auto& pair : loggers_) {
            if (pair.second) {
                pair.second->flush();
            }
        }
    }

    ~BusinessLogger() {
        flush_all();
        spdlog::shutdown();
    }
};
