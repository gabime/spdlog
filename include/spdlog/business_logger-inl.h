// Copyright(c) 2025-present.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/business_logger.h>
#endif

namespace spdlog {

// BusinessLogger 实现
inline BusinessLogger::BusinessLogger(const std::string& log_dir,
                                       size_t max_size,
                                       size_t max_files,
                                       bool enable_console)
    : enable_console_(enable_console) {
    
    // 创建业务分类 sink
    business_sink_ = std::make_shared<sinks::business_sink_mt>(log_dir, max_size, max_files);
    
    // 初始化 logger
    init_loggers_();
    
    // 如果启用控制台输出，创建控制台 sink
    if (enable_console_) {
        auto console_sink = std::make_shared<sinks::stdout_color_sink_mt>();
        console_sink->set_pattern("[%H:%M:%S] [%^%l%$] %v");
        console_logger_ = std::make_shared<logger>("console", console_sink);
    }
}

inline void BusinessLogger::init_loggers_() {
    // 为每种业务类型创建 logger
    business_loggers_.resize(static_cast<size_t>(BusinessType::Count));
    
    business_loggers_[static_cast<size_t>(BusinessType::ScreenRecord)] = 
        create_logger_(BusinessType::ScreenRecord);
    business_loggers_[static_cast<size_t>(BusinessType::DesktopOpen)] = 
        create_logger_(BusinessType::DesktopOpen);
    business_loggers_[static_cast<size_t>(BusinessType::KeyboardRecord)] = 
        create_logger_(BusinessType::KeyboardRecord);
    business_loggers_[static_cast<size_t>(BusinessType::AudioRecord)] = 
        create_logger_(BusinessType::AudioRecord);
}

inline std::shared_ptr<logger> BusinessLogger::create_logger_(BusinessType type) {
    std::string name = business_type_to_string(type);
    return std::make_shared<logger>(name, business_sink_);
}

inline std::shared_ptr<logger> BusinessLogger::get_logger(BusinessType type) {
    size_t index = static_cast<size_t>(type);
    if (index < business_loggers_.size() && business_loggers_[index]) {
        return business_loggers_[index];
    }
    
    // 如果 logger 不存在，创建一个
    auto logger = create_logger_(type);
    if (index >= business_loggers_.size()) {
        business_loggers_.resize(index + 1);
    }
    business_loggers_[index] = logger;
    return logger;
}

inline void BusinessLogger::flush() {
    business_sink_->flush();
    if (console_logger_) {
        console_logger_->flush();
    }
}

inline void BusinessLogger::set_level(level::level_enum lvl) {
    for (auto& logger : business_loggers_) {
        if (logger) {
            logger->set_level(lvl);
        }
    }
}

inline void BusinessLogger::set_business_level(BusinessType type, level::level_enum lvl) {
    auto logger = get_logger(type);
    logger->set_level(lvl);
}

inline void BusinessLogger::set_business_config(BusinessType type,
                                                 const std::string& name,
                                                 const std::string& custom_filename) {
    business_sink_->set_business_config(type, name, custom_filename);
}

// BusinessLoggerManager 实现
inline BusinessLoggerManager& BusinessLoggerManager::instance() {
    static BusinessLoggerManager instance;
    return instance;
}

inline void BusinessLoggerManager::initialize(const std::string& log_dir,
                                               size_t max_size,
                                               size_t max_files,
                                               bool enable_console) {
    logger_ = std::make_shared<BusinessLogger>(log_dir, max_size, max_files, enable_console);
}

inline std::shared_ptr<BusinessLogger> BusinessLoggerManager::get_logger() {
    if (!logger_) {
        // 如果未初始化，使用默认配置
        initialize();
    }
    return logger_;
}

inline void BusinessLoggerManager::flush() {
    if (logger_) {
        logger_->flush();
    }
}

}  // namespace spdlog
