#include "business_logger.h"
#include "spdlog/fmt/ostr.h"

BusinessLogger& BusinessLogger::getInstance() {
    static BusinessLogger instance;
    return instance;
}

void BusinessLogger::init(const std::string& log_dir) {
    log_dir_ = log_dir;
    loggers_.clear();

    getLogger(BusinessType::RECORD_SCREEN);
    getLogger(BusinessType::OPEN_DESKTOP);
    getLogger(BusinessType::RECORD_KEYBOARD);
    getLogger(BusinessType::RECORD_AUDIO);
}

std::string BusinessLogger::getBusinessName(BusinessType type) {
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

std::shared_ptr<spdlog::logger> BusinessLogger::getLogger(BusinessType type) {
    auto it = loggers_.find(type);
    if (it != loggers_.end()) {
        return it->second;
    }

    std::string business_name = getBusinessName(type);
    std::string log_file = log_dir_ + "/" + business_name + ".log";

    auto logger = spdlog::rotating_logger_mt(business_name, log_file, max_file_size_, max_files_);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
    logger->set_level(spdlog::level::debug);
    logger->flush_on(spdlog::level::info);

    loggers_[type] = logger;
    return logger;
}

void BusinessLogger::log(BusinessType type, spdlog::level::level_enum level, const std::string& message) {
    auto logger = getLogger(type);
    logger->log(level, message);
}
