#ifndef SPDLOG_COMPILED_LIB
#error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

#include <spdlog/business_logger.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace spdlog {

BusinessLogger& BusinessLogger::instance() {
    static BusinessLogger instance;
    return instance;
}

BusinessLogger::BusinessLogger() : initialized_(false) {}

void BusinessLogger::init(const filename_t& log_dir) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return;
    }

    log_dir_ = log_dir;

    const size_t max_file_size = 30 * 1024 * 1024;
    const size_t max_files = 3;

    auto create_logger = [&](BusinessType type) {
        std::string logger_name;
        filename_t log_file = get_log_filename(type);
        
        switch (type) {
            case BusinessType::RECORD_SCREEN:
                logger_name = "record_screen";
                break;
            case BusinessType::OPEN_DESKTOP:
                logger_name = "open_desktop";
                break;
            case BusinessType::RECORD_KEYBOARD:
                logger_name = "record_keyboard";
                break;
            case BusinessType::RECORD_AUDIO:
                logger_name = "record_audio";
                break;
            default:
                logger_name = "unknown";
                break;
        }
        
        auto sink = std::make_shared<sinks::rotating_file_sink_mt>(
            log_file, max_file_size, max_files);
        
        auto logger = std::make_shared<spdlog::logger>(logger_name, sink);
        logger->set_level(level::trace);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
        logger->flush_on(level::info);
        
        spdlog::register_logger(logger);
        loggers_[type] = logger;
    };

    create_logger(BusinessType::RECORD_SCREEN);
    create_logger(BusinessType::OPEN_DESKTOP);
    create_logger(BusinessType::RECORD_KEYBOARD);
    create_logger(BusinessType::RECORD_AUDIO);

    initialized_ = true;
}

std::shared_ptr<logger> BusinessLogger::get_logger(BusinessType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = loggers_.find(type);
    if (it != loggers_.end()) {
        return it->second;
    }
    return nullptr;
}

filename_t BusinessLogger::get_business_name(BusinessType type) {
    switch (type) {
        case BusinessType::RECORD_SCREEN:
            return SPDLOG_FILENAME_T("record_screen");
        case BusinessType::OPEN_DESKTOP:
            return SPDLOG_FILENAME_T("open_desktop");
        case BusinessType::RECORD_KEYBOARD:
            return SPDLOG_FILENAME_T("record_keyboard");
        case BusinessType::RECORD_AUDIO:
            return SPDLOG_FILENAME_T("record_audio");
        default:
            return SPDLOG_FILENAME_T("unknown");
    }
}

filename_t BusinessLogger::get_log_filename(BusinessType type) {
    return log_dir_ + SPDLOG_FILENAME_T("/") + get_business_name(type) + SPDLOG_FILENAME_T(".log");
}

}  // namespace spdlog
