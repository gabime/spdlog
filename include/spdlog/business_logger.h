// Copyright(c) 2025-present, Business Logger Extension for spdlog.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/business_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/details/registry.h>

#include <memory>
#include <string>
#include <vector>

namespace spdlog {

// Business logger manager - provides convenient API for business-specific logging
class business_logger_manager {
public:
    // Business type aliases for convenience
    using business_type = sinks::business_type;
    static constexpr business_type screen_recorder = business_type::screen_recorder;
    static constexpr business_type desktop_open = business_type::desktop_open;
    static constexpr business_type keyboard_recorder = business_type::keyboard_recorder;
    static constexpr business_type sound_recorder = business_type::sound_recorder;

    // Constructor with optional prefix for logger names
    explicit business_logger_manager(const filename_t &log_dir = SPDLOG_FILENAME_T("logs"),
                                     std::size_t max_file_size = 30 * 1024 * 1024,  // 30MB
                                     std::size_t max_files = 3,
                                     bool also_log_to_console = true,
                                     const std::string &logger_name_prefix = "business")
        : log_dir_(log_dir),
          max_file_size_(max_file_size),
          max_files_(max_files),
          also_log_to_console_(also_log_to_console),
          logger_name_prefix_(logger_name_prefix) {
        init_loggers();
    }

    // Log with specific business type
    template <typename... Args>
    void log(business_type biz_type,
             level::level_enum lvl,
             format_string_t<Args...> fmt,
             Args &&...args) {
        auto logger = get_logger(biz_type);
        if (logger) {
            logger->log(lvl, fmt, std::forward<Args>(args)...);
        }
    }

    // Convenience methods for each log level
    template <typename... Args>
    void trace(business_type biz_type, format_string_t<Args...> fmt, Args &&...args) {
        log(biz_type, level::trace, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(business_type biz_type, format_string_t<Args...> fmt, Args &&...args) {
        log(biz_type, level::debug, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(business_type biz_type, format_string_t<Args...> fmt, Args &&...args) {
        log(biz_type, level::info, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(business_type biz_type, format_string_t<Args...> fmt, Args &&...args) {
        log(biz_type, level::warn, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(business_type biz_type, format_string_t<Args...> fmt, Args &&...args) {
        log(biz_type, level::err, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(business_type biz_type, format_string_t<Args...> fmt, Args &&...args) {
        log(biz_type, level::critical, fmt, std::forward<Args>(args)...);
    }

    // Get logger name for a business type
    std::string get_logger_name(business_type biz_type) const {
        return logger_name_prefix_ + "_" + sinks::business_type_to_string(biz_type);
    }

    // Get the underlying logger for a business type
    std::shared_ptr<logger> get_logger(business_type biz_type) const {
        std::string name = get_logger_name(biz_type);
        return spdlog::details::registry::instance().get(name);
    }

    // Set log level for a specific business type
    void set_level(business_type biz_type, level::level_enum lvl) {
        auto logger = get_logger(biz_type);
        if (logger) {
            logger->set_level(lvl);
        }
    }

    // Set log level for all business types
    void set_all_levels(level::level_enum lvl) {
        set_level(screen_recorder, lvl);
        set_level(desktop_open, lvl);
        set_level(keyboard_recorder, lvl);
        set_level(sound_recorder, lvl);
    }

    // Flush all loggers
    void flush() const {
        spdlog::details::registry::instance().apply_all([](std::shared_ptr<logger> l) { l->flush(); });
    }

    // Get log directory
    filename_t log_dir() const { return log_dir_; }

    // Get max file size
    std::size_t max_file_size() const { return max_file_size_; }

    // Get max files
    std::size_t max_files() const { return max_files_; }

private:
    void init_loggers() {
        // Create a rotating file logger for each business type
        std::vector<business_type> all_types = {
            screen_recorder, desktop_open, keyboard_recorder, sound_recorder
        };

        for (auto biz_type : all_types) {
            std::string logger_name = get_logger_name(biz_type);
            
            // Create sinks vector
            std::vector<sink_ptr> sinks;
            
            // Add rotating file sink
            filename_t filename = build_filename(biz_type);
            auto file_sink = std::make_shared<sinks::rotating_file_sink_mt>(
                filename, max_file_size_, max_files_);
            sinks.push_back(file_sink);
            
            // Add console sink if requested
            if (also_log_to_console_) {
                auto console_sink = std::make_shared<sinks::stdout_color_sink_mt>();
                sinks.push_back(console_sink);
            }
            
            // Create logger with multiple sinks
            auto biz_logger = std::make_shared<logger>(logger_name, sinks.begin(), sinks.end());
            biz_logger->set_level(level::info);
            
            // Register logger
            spdlog::details::registry::instance().register_logger(biz_logger);
        }
    }

    filename_t build_filename(business_type biz_type) {
        std::string biz_name = sinks::business_type_to_string(biz_type);
#ifdef _WIN32
        return log_dir_ + SPDLOG_FILENAME_T("\\") + SPDLOG_FILENAME_T(biz_name.c_str()) +
               SPDLOG_FILENAME_T(".log");
#else
        return log_dir_ + SPDLOG_FILENAME_T("/") + SPDLOG_FILENAME_T(biz_name.c_str()) +
               SPDLOG_FILENAME_T(".log");
#endif
    }

    filename_t log_dir_;
    std::size_t max_file_size_;
    std::size_t max_files_;
    bool also_log_to_console_;
    std::string logger_name_prefix_;
};

// Global business logger manager instance accessor
inline std::shared_ptr<business_logger_manager> &get_business_logger_manager() {
    static std::shared_ptr<business_logger_manager> instance;
    return instance;
}

// Initialize global business logger manager
inline void init_business_logger(const filename_t &log_dir = SPDLOG_FILENAME_T("logs"),
                                  std::size_t max_file_size = 30 * 1024 * 1024,
                                  std::size_t max_files = 3,
                                  bool also_log_to_console = true,
                                  const std::string &logger_name_prefix = "global_business") {
    get_business_logger_manager() = std::make_shared<business_logger_manager>(
        log_dir, max_file_size, max_files, also_log_to_console, logger_name_prefix);
}

// Log to specific business type using global manager
template <typename... Args>
inline void business_log(business_logger_manager::business_type biz_type,
                         level::level_enum lvl,
                         format_string_t<Args...> fmt,
                         Args &&...args) {
    auto &manager = get_business_logger_manager();
    if (manager) {
        manager->log(biz_type, lvl, fmt, std::forward<Args>(args)...);
    }
}

// Convenience macros for business logging
#define SPDLOG_BUSINESS_TRACE(biz_type, ...) \
    spdlog::business_log(biz_type, spdlog::level::trace, __VA_ARGS__)
#define SPDLOG_BUSINESS_DEBUG(biz_type, ...) \
    spdlog::business_log(biz_type, spdlog::level::debug, __VA_ARGS__)
#define SPDLOG_BUSINESS_INFO(biz_type, ...) \
    spdlog::business_log(biz_type, spdlog::level::info, __VA_ARGS__)
#define SPDLOG_BUSINESS_WARN(biz_type, ...) \
    spdlog::business_log(biz_type, spdlog::level::warn, __VA_ARGS__)
#define SPDLOG_BUSINESS_ERROR(biz_type, ...) \
    spdlog::business_log(biz_type, spdlog::level::err, __VA_ARGS__)
#define SPDLOG_BUSINESS_CRITICAL(biz_type, ...) \
    spdlog::business_log(biz_type, spdlog::level::critical, __VA_ARGS__)

}  // namespace spdlog
