#include "module_core.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/details/os.h"
#include <mutex>

namespace module_core {

namespace {
std::once_flag init_flag;
const char* const LOGGER_NAME = "core_logger";
const int MODULE_ID = 1;

spdlog::filename_t path_join(const spdlog::filename_t& dir, const spdlog::filename_t& filename) {
    if (dir.empty()) {
        return filename;
    }
    
    spdlog::filename_t result = dir;
    
    if (!result.empty()) {
        auto last_char = result.back();
        if (last_char != SPDLOG_FILENAME_T('/') && last_char != SPDLOG_FILENAME_T('\\')) {
#ifdef _WIN32
            result += SPDLOG_FILENAME_T('\\');
#else
            result += SPDLOG_FILENAME_T('/');
#endif
        }
    }
    
    result += filename;
    return result;
}
}

CoreLogger& CoreLogger::instance() {
    static CoreLogger instance;
    return instance;
}

void CoreLogger::init(const spdlog::filename_t& log_dir) {
    if (initialized_) {
        return;
    }
    
    log_file_ = path_join(log_dir, SPDLOG_FILENAME_T("core.log"));
    
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_, true);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
    std::vector<spdlog::sink_ptr> sinks = {file_sink, console_sink};
    logger_ = std::make_shared<spdlog::logger>(LOGGER_NAME, sinks.begin(), sinks.end());
    
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %v");
    logger_->set_level(spdlog::level::debug);
    logger_->flush_on(spdlog::level::warn);
    
    spdlog::register_logger(logger_);
    initialized_ = true;
}

void CoreLogger::shutdown() {
    if (logger_) {
        logger_->flush();
        spdlog::drop(LOGGER_NAME);
        logger_.reset();
    }
    initialized_ = false;
}

std::shared_ptr<spdlog::logger> CoreLogger::get_logger() const {
    return logger_;
}

spdlog::filename_t CoreLogger::get_log_file() const {
    return log_file_;
}

void CoreLogger::log_info(const std::string& message) {
    if (logger_) {
        logger_->info("[Core] {}", message);
    }
}

void CoreLogger::log_warn(const std::string& message) {
    if (logger_) {
        logger_->warn("[Core] {}", message);
    }
}

void CoreLogger::log_error(const std::string& message) {
    if (logger_) {
        logger_->error("[Core] {}", message);
    }
}

void CoreLogger::log_debug(const std::string& message) {
    if (logger_) {
        logger_->debug("[Core] {}", message);
    }
}

void CoreLogger::set_level(spdlog::level::level_enum level) {
    if (logger_) {
        logger_->set_level(level);
    }
}

spdlog::level::level_enum CoreLogger::get_level() const {
    if (logger_) {
        return logger_->level();
    }
    return spdlog::level::off;
}

void CoreLogger::flush() {
    if (logger_) {
        logger_->flush();
    }
}

void test_function() {
    auto& core_logger = CoreLogger::instance();
    core_logger.log_info("Test message from core module");
}

int get_module_id() {
    return MODULE_ID;
}

}
