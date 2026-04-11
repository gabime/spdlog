// business_logger.h
// 多业务日志管理器 - 基于 spdlog 实现
// 支持按业务类型分文件记录日志，自动轮转，最多保留3个文件，每文件最大30M

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/logger.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace business {

// 业务类型枚举
enum class BusinessType {
    ScreenRecord,   // 录制屏幕
    DesktopOpen,    // 打开桌面
    KeyboardRecord, // 录制键盘
    SoundRecord,    // 录制声音
    Unknown         // 未知业务
};

// 业务类型工具类
class BusinessTypeHelper {
public:
    // 将业务类型转换为字符串
    static std::string to_string(BusinessType type) {
        switch (type) {
            case BusinessType::ScreenRecord:   return "screen_record";
            case BusinessType::DesktopOpen:    return "desktop_open";
            case BusinessType::KeyboardRecord: return "keyboard_record";
            case BusinessType::SoundRecord:    return "sound_record";
            default:                           return "unknown";
        }
    }

    // 将字符串转换为业务类型
    static BusinessType from_string(const std::string& str) {
        if (str == "screen_record")   return BusinessType::ScreenRecord;
        if (str == "desktop_open")    return BusinessType::DesktopOpen;
        if (str == "keyboard_record") return BusinessType::KeyboardRecord;
        if (str == "sound_record")    return BusinessType::SoundRecord;
        return BusinessType::Unknown;
    }
};

// 日志配置结构体
struct LogConfig {
    std::string log_dir = "logs";           // 日志目录
    size_t max_file_size = 30 * 1024 * 1024; // 单个文件最大30M
    size_t max_files = 3;                    // 最多保留3个文件
    bool enable_console = true;              // 是否同时输出到控制台
    spdlog::level::level_enum level = spdlog::level::info; // 日志级别

    LogConfig() = default;
    LogConfig(std::string dir, size_t file_size, size_t files, bool console, spdlog::level::level_enum lvl)
        : log_dir(std::move(dir)), max_file_size(file_size), max_files(files), enable_console(console), level(lvl) {}
};

// 多业务日志管理器
class BusinessLogger {
public:
    // 获取单例实例
    static BusinessLogger& instance() {
        static BusinessLogger instance;
        return instance;
    }

    // 初始化日志管理器
    void initialize(const LogConfig& config = LogConfig()) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = config;
        initialized_ = true;
    }

    // 获取指定业务类型的日志记录器
    std::shared_ptr<spdlog::logger> get_logger(BusinessType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            throw std::runtime_error("BusinessLogger not initialized. Call initialize() first.");
        }

        // 检查是否已存在
        auto it = loggers_.find(type);
        if (it != loggers_.end()) {
            return it->second;
        }

        // 创建新的日志记录器
        auto logger = create_logger(type);
        loggers_[type] = logger;
        return logger;
    }

    // 记录日志 - 便捷方法
    void log(BusinessType type, spdlog::level::level_enum level, const std::string& message) {
        auto logger = get_logger(type);
        logger->log(level, message);
    }

    // 各业务类型的便捷方法
    void screen_record(spdlog::level::level_enum level, const std::string& message) {
        log(BusinessType::ScreenRecord, level, message);
    }

    void desktop_open(spdlog::level::level_enum level, const std::string& message) {
        log(BusinessType::DesktopOpen, level, message);
    }

    void keyboard_record(spdlog::level::level_enum level, const std::string& message) {
        log(BusinessType::KeyboardRecord, level, message);
    }

    void sound_record(spdlog::level::level_enum level, const std::string& message) {
        log(BusinessType::SoundRecord, level, message);
    }

    // 设置所有日志记录器的级别
    void set_level(spdlog::level::level_enum level) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_.level = level;
        for (auto& [type, logger] : loggers_) {
            logger->set_level(level);
        }
    }

    // 刷新所有日志
    void flush_all() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [type, logger] : loggers_) {
            logger->flush();
        }
    }

    // 关闭所有日志记录器
    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        loggers_.clear();
        spdlog::shutdown();
    }

private:
    BusinessLogger() = default;
    ~BusinessLogger() = default;

    // 禁止拷贝和赋值
    BusinessLogger(const BusinessLogger&) = delete;
    BusinessLogger& operator=(const BusinessLogger&) = delete;

    // 创建指定业务类型的日志记录器
    std::shared_ptr<spdlog::logger> create_logger(BusinessType type) {
        std::string business_name = BusinessTypeHelper::to_string(type);
        std::string filename = config_.log_dir + "/" + business_name + ".log";

        std::vector<spdlog::sink_ptr> sinks;

        // 创建轮转文件 sink
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            filename, 
            config_.max_file_size, 
            config_.max_files
        );
        file_sink->set_level(config_.level);
        sinks.push_back(file_sink);

        // 如果需要同时输出到控制台
        if (config_.enable_console) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(config_.level);
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [" + business_name + "] [%^%l%$] %v");
            sinks.push_back(console_sink);
        }

        // 创建 logger
        auto logger = std::make_shared<spdlog::logger>(business_name, sinks.begin(), sinks.end());
        logger->set_level(config_.level);
        
        // 设置日志格式
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        // 注册到 spdlog 全局注册表
        spdlog::register_logger(logger);

        return logger;
    }

    LogConfig config_;
    std::unordered_map<BusinessType, std::shared_ptr<spdlog::logger>> loggers_;
    std::mutex mutex_;
    bool initialized_ = false;
};

// 全局便捷函数

// 初始化日志系统
inline void init_logger(const LogConfig& config = LogConfig()) {
    BusinessLogger::instance().initialize(config);
}

// 获取业务日志记录器
inline std::shared_ptr<spdlog::logger> get_business_logger(BusinessType type) {
    return BusinessLogger::instance().get_logger(type);
}

// 记录业务日志
inline void log_business(BusinessType type, spdlog::level::level_enum level, const std::string& message) {
    BusinessLogger::instance().log(type, level, message);
}

} // namespace business

// 宏定义，方便使用
#define LOG_SCREEN_RECORD(level, ...) \
    business::BusinessLogger::instance().screen_record(level, fmt::format(__VA_ARGS__))

#define LOG_DESKTOP_OPEN(level, ...) \
    business::BusinessLogger::instance().desktop_open(level, fmt::format(__VA_ARGS__))

#define LOG_KEYBOARD_RECORD(level, ...) \
    business::BusinessLogger::instance().keyboard_record(level, fmt::format(__VA_ARGS__))

#define LOG_SOUND_RECORD(level, ...) \
    business::BusinessLogger::instance().sound_record(level, fmt::format(__VA_ARGS__))
