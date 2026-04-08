#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace spdlog {

enum class BusinessType {
    RECORD_SCREEN,
    OPEN_DESKTOP,
    RECORD_KEYBOARD,
    RECORD_AUDIO
};

class SPDLOG_API BusinessLogger {
public:
    static BusinessLogger& instance();

    void init(const filename_t& log_dir = SPDLOG_FILENAME_T("logs"));

    template <typename... Args>
    void log(BusinessType type, level::level_enum level, format_string_t<Args...> fmt, Args&&... args);

    template <typename... Args>
    void log_info(BusinessType type, format_string_t<Args...> fmt, Args&&... args);

    template <typename... Args>
    void log_warn(BusinessType type, format_string_t<Args...> fmt, Args&&... args);

    template <typename... Args>
    void log_error(BusinessType type, format_string_t<Args...> fmt, Args&&... args);

    template <typename... Args>
    void log_debug(BusinessType type, format_string_t<Args...> fmt, Args&&... args);

    std::shared_ptr<logger> get_logger(BusinessType type);

    BusinessLogger(const BusinessLogger&) = delete;
    BusinessLogger& operator=(const BusinessLogger&) = delete;

private:
    BusinessLogger();
    ~BusinessLogger() = default;

    filename_t get_business_name(BusinessType type);
    filename_t get_log_filename(BusinessType type);

    filename_t log_dir_;
    std::unordered_map<BusinessType, std::shared_ptr<logger>> loggers_;
    std::mutex mutex_;
    bool initialized_;
};

template <typename... Args>
void BusinessLogger::log(BusinessType type, level::level_enum level, format_string_t<Args...> fmt, Args&&... args) {
    if (!initialized_) {
        init();
    }
    auto logger = get_logger(type);
    if (logger) {
        logger->log(level, fmt, std::forward<Args>(args)...);
    }
}

template <typename... Args>
void BusinessLogger::log_info(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
    log(type, level::info, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void BusinessLogger::log_warn(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
    log(type, level::warn, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void BusinessLogger::log_error(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
    log(type, level::err, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void BusinessLogger::log_debug(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
    log(type, level::debug, fmt, std::forward<Args>(args)...);
}

#define BUSINESS_LOG(type, level, fmt, ...) \
    spdlog::BusinessLogger::instance().log(type, level, fmt, ##__VA_ARGS__)

#define BUSINESS_LOG_INFO(type, fmt, ...) \
    spdlog::BusinessLogger::instance().log_info(type, fmt, ##__VA_ARGS__)

#define BUSINESS_LOG_WARN(type, fmt, ...) \
    spdlog::BusinessLogger::instance().log_warn(type, fmt, ##__VA_ARGS__)

#define BUSINESS_LOG_ERROR(type, fmt, ...) \
    spdlog::BusinessLogger::instance().log_error(type, fmt, ##__VA_ARGS__)

#define BUSINESS_LOG_DEBUG(type, fmt, ...) \
    spdlog::BusinessLogger::instance().log_debug(type, fmt, ##__VA_ARGS__)

}  // namespace spdlog
