// Copyright(c) 2025 spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/business_logger.h>
#include <spdlog/details/os.h>

#include <cassert>

namespace spdlog {

business_logger_manager& business_logger_manager::instance() {
    static business_logger_manager instance;
    return instance;
}

void business_logger_manager::initialize(const filename_t& base_dir) {
    std::lock_guard<std::mutex> lock(mutex_);
    base_dir_ = base_dir;
    initialized_ = true;

    // 确保日志目录存在 (跨平台支持 Windows/Linux/macOS)
    details::os::create_dir(base_dir_);
}

std::shared_ptr<logger> business_logger_manager::get_logger(business_type type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        throw_spdlog_ex("business_logger_manager not initialized. Call initialize() first.");
    }

    auto it = loggers_.find(type);
    if (it != loggers_.end()) {
        return it->second;
    }

    // 创建新的logger
    auto logger = create_logger(type);
    loggers_[type] = logger;
    return logger;
}

void business_logger_manager::log(business_type type, level::level_enum lvl, const std::string& msg) {
    auto logger = get_logger(type);
    logger->log(lvl, msg);
}

void business_logger_manager::set_level(level::level_enum lvl) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& kv : loggers_) {
        kv.second->set_level(lvl);
    }
}

void business_logger_manager::flush_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& kv : loggers_) {
        kv.second->flush();
    }
}

void business_logger_manager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    loggers_.clear();
    initialized_ = false;
}

std::shared_ptr<logger> business_logger_manager::create_logger(business_type type) {
    filename_t log_file = get_log_filename(type);

    // 创建轮转文件sink，每个文件30MB，最多保留3个
    auto sink = std::make_shared<sinks::rotating_file_sink_mt>(
        log_file, max_file_size, max_files, false);

    // 设置日志格式: [时间] [级别] 消息
    sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    // 创建logger
    auto logger = std::make_shared<spdlog::logger>(get_business_name(type), sink);
    logger->set_level(level::info);
    
    return logger;
}

std::string business_logger_manager::get_business_name(business_type type) {
    switch (type) {
        case business_type::screen_record:
            return "screen_record";
        case business_type::desktop_open:
            return "desktop_open";
        case business_type::keyboard_record:
            return "keyboard_record";
        case business_type::sound_record:
            return "sound_record";
        default:
            return "unknown";
    }
}

filename_t business_logger_manager::get_log_filename(business_type type) {
    // 使用spdlog的跨平台路径分隔符
    // folder_seps_filename 在 Windows 上是 "\\/"，在其他平台是 "/"
    const filename_t::value_type sep = details::os::folder_seps_filename[0];
    
    std::string name = get_business_name(type);
    
#ifdef SPDLOG_WCHAR_FILENAMES
    // Windows 宽字符模式: 将 std::string 转换为 std::wstring
    filename_t filename = base_dir_;
    filename += sep;
    // 将 ASCII 字符转换为 wchar_t
    for (char c : name) {
        filename += static_cast<wchar_t>(c);
    }
    filename += SPDLOG_FILENAME_T(".log");
    return filename;
#else
    // 普通模式: 使用 std::string
    filename_t filename = base_dir_;
    filename += sep;
    filename += name;
    filename += ".log";
    return filename;
#endif
}

}  // namespace spdlog
