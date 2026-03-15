// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <map>
#include <memory>
#include <string>

namespace spdlog {

enum class BusinessType {
    ScreenRecording = 0,
    OpenDesktop = 1,
    KeyboardRecording = 2,
    AudioRecording = 3
};

class SPDLOG_API business_logger_manager {
public:
    static business_logger_manager &instance();

    void init(const std::string &log_dir = "logs");
    void set_log_dir(const std::string &log_dir);
    std::string get_log_dir() const;

    std::shared_ptr<logger> get_logger(BusinessType business_type);

    template <typename... Args>
    void log(BusinessType business_type, level::level_enum lvl, format_string_t<Args...> fmt, Args &&...args);

    template <typename... Args>
    void trace(BusinessType business_type, format_string_t<Args...> fmt, Args &&...args);

    template <typename... Args>
    void debug(BusinessType business_type, format_string_t<Args...> fmt, Args &&...args);

    template <typename... Args>
    void info(BusinessType business_type, format_string_t<Args...> fmt, Args &&...args);

    template <typename... Args>
    void warn(BusinessType business_type, format_string_t<Args...> fmt, Args &&...args);

    template <typename... Args>
    void error(BusinessType business_type, format_string_t<Args...> fmt, Args &&...args);

    template <typename... Args>
    void critical(BusinessType business_type, format_string_t<Args...> fmt, Args &&...args);

    void shutdown();

private:
    business_logger_manager();
    ~business_logger_manager();
    business_logger_manager(const business_logger_manager &) = delete;
    business_logger_manager &operator=(const business_logger_manager &) = delete;

    std::string business_type_to_string(BusinessType type) const;
    std::shared_ptr<logger> create_business_logger(BusinessType type);

    std::string log_dir_;
    std::map<BusinessType, std::shared_ptr<logger>> loggers_;
    bool initialized_;
    mutable std::mutex mutex_;

    static constexpr size_t MAX_FILE_SIZE = 30 * 1024 * 1024;
    static constexpr size_t MAX_FILES = 3;
};

}  // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "business_logger-inl.h"
#endif
