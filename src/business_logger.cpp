// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/business_logger.h>

namespace spdlog {

// 初始化静态成员
BusinessLogger* BusinessLogger::instance_ = nullptr;

// 获取单例实例
BusinessLogger* BusinessLogger::get_instance() {
    if (instance_ == nullptr) {
        instance_ = new BusinessLogger();
    }
    return instance_;
}

// 初始化所有业务日志记录器
void BusinessLogger::init_loggers() {
    // 为每种业务类型创建日志记录器
    loggers_[BusinessType::SCREEN_RECORD] = rotating_logger_mt(
        get_logger_name(BusinessType::SCREEN_RECORD),
        get_log_filename(BusinessType::SCREEN_RECORD),
        MAX_FILE_SIZE,
        MAX_FILES
    );
    
    loggers_[BusinessType::DESKTOP_OPEN] = rotating_logger_mt(
        get_logger_name(BusinessType::DESKTOP_OPEN),
        get_log_filename(BusinessType::DESKTOP_OPEN),
        MAX_FILE_SIZE,
        MAX_FILES
    );
    
    loggers_[BusinessType::KEYBOARD_RECORD] = rotating_logger_mt(
        get_logger_name(BusinessType::KEYBOARD_RECORD),
        get_log_filename(BusinessType::KEYBOARD_RECORD),
        MAX_FILE_SIZE,
        MAX_FILES
    );
    
    loggers_[BusinessType::AUDIO_RECORD] = rotating_logger_mt(
        get_logger_name(BusinessType::AUDIO_RECORD),
        get_log_filename(BusinessType::AUDIO_RECORD),
        MAX_FILE_SIZE,
        MAX_FILES
    );
}

// 根据业务类型获取日志文件名
std::string BusinessLogger::get_log_filename(BusinessType type) {
    std::string filename = LOG_DIR;
    
    switch (type) {
        case BusinessType::SCREEN_RECORD:
            filename += "screen_record.log";
            break;
        case BusinessType::DESKTOP_OPEN:
            filename += "desktop_open.log";
            break;
        case BusinessType::KEYBOARD_RECORD:
            filename += "keyboard_record.log";
            break;
        case BusinessType::AUDIO_RECORD:
            filename += "audio_record.log";
            break;
    }
    
    return filename;
}

// 根据业务类型获取日志记录器名称
std::string BusinessLogger::get_logger_name(BusinessType type) {
    switch (type) {
        case BusinessType::SCREEN_RECORD:
            return "screen_record_logger";
        case BusinessType::DESKTOP_OPEN:
            return "desktop_open_logger";
        case BusinessType::KEYBOARD_RECORD:
            return "keyboard_record_logger";
        case BusinessType::AUDIO_RECORD:
            return "audio_record_logger";
        default:
            return "unknown_logger";
    }
}

} // namespace spdlog
