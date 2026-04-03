// Copyright(c) 2024 spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// 多业务日志管理器
// 支持根据业务类型写入不同的日志文件，每个业务独立管理日志轮转

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/details/registry.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace business_log {

// 业务类型枚举
enum class BusinessType {
    Screen,     // 录制屏幕
    Desktop,    // 打开桌面
    Keyboard,   // 录制键盘
    Audio       // 录制声音
};

// 业务类型转换为字符串
inline std::string business_type_to_string(BusinessType type) {
    switch (type) {
        case BusinessType::Screen:   return "screen";
        case BusinessType::Desktop:  return "desktop";
        case BusinessType::Keyboard: return "keyboard";
        case BusinessType::Audio:    return "audio";
        default:                     return "unknown";
    }
}

// 日志配置结构体
struct LogConfig {
    std::string log_dir = "logs";           // 日志目录
    size_t max_file_size = 30 * 1024 * 1024; // 单个文件最大30MB
    size_t max_files = 3;                    // 最多保留3个文件
    bool rotate_on_open = false;             // 打开时是否轮转

    LogConfig() = default;
    LogConfig(std::string dir, size_t file_size, size_t files)
        : log_dir(std::move(dir)), max_file_size(file_size), max_files(files) {}
};

// 业务日志管理器类
class BusinessLogManager {
public:
    // 获取单例实例
    static BusinessLogManager& instance() {
        static BusinessLogManager instance;
        return instance;
    }

    // 初始化日志管理器
    void initialize(const LogConfig& config = LogConfig()) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = config;
        initialized_ = true;
    }

    // 写入日志（通过业务类型）
    template <typename... Args>
    void log(BusinessType business_type, spdlog::level::level_enum level, const std::string& format, Args&&... args) {
        auto logger = get_logger(business_type);
        if (logger) {
            logger->log(level, format, std::forward<Args>(args)...);
        }
    }

    // 便捷方法：trace级别
    template <typename... Args>
    void trace(BusinessType business_type, const std::string& format, Args&&... args) {
        log(business_type, spdlog::level::trace, format, std::forward<Args>(args)...);
    }

    // 便捷方法：debug级别
    template <typename... Args>
    void debug(BusinessType business_type, const std::string& format, Args&&... args) {
        log(business_type, spdlog::level::debug, format, std::forward<Args>(args)...);
    }

    // 便捷方法：info级别
    template <typename... Args>
    void info(BusinessType business_type, const std::string& format, Args&&... args) {
        log(business_type, spdlog::level::info, format, std::forward<Args>(args)...);
    }

    // 便捷方法：warn级别
    template <typename... Args>
    void warn(BusinessType business_type, const std::string& format, Args&&... args) {
        log(business_type, spdlog::level::warn, format, std::forward<Args>(args)...);
    }

    // 便捷方法：error级别
    template <typename... Args>
    void error(BusinessType business_type, const std::string& format, Args&&... args) {
        log(business_type, spdlog::level::err, format, std::forward<Args>(args)...);
    }

    // 便捷方法：critical级别
    template <typename... Args>
    void critical(BusinessType business_type, const std::string& format, Args&&... args) {
        log(business_type, spdlog::level::critical, format, std::forward<Args>(args)...);
    }

    // 刷新指定业务的日志
    void flush(BusinessType business_type) {
        auto logger = get_logger(business_type);
        if (logger) {
            logger->flush();
        }
    }

    // 刷新所有业务日志
    void flush_all() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : loggers_) {
            if (pair.second) {
                pair.second->flush();
            }
        }
    }

    // 设置指定业务的日志级别
    void set_level(BusinessType business_type, spdlog::level::level_enum level) {
        auto logger = get_logger(business_type);
        if (logger) {
            logger->set_level(level);
        }
    }

    // 设置所有业务的日志级别
    void set_all_levels(spdlog::level::level_enum level) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : loggers_) {
            if (pair.second) {
                pair.second->set_level(level);
            }
        }
    }

    // 立即轮转指定业务的日志文件
    void rotate_now(BusinessType business_type) {
        std::string logger_name = get_logger_name(business_type);
        auto logger = spdlog::get(logger_name);
        if (logger) {
            // 获取sink并强制轮转
            for (auto& sink : logger->sinks()) {
                auto rotating_sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(sink);
                if (rotating_sink) {
                    rotating_sink->rotate_now();
                }
            }
        }
    }

    // 关闭所有日志器并清理资源
    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        loggers_.clear();
        spdlog::shutdown();
    }

private:
    BusinessLogManager() = default;
    ~BusinessLogManager() = default;

    // 禁止拷贝和赋值
    BusinessLogManager(const BusinessLogManager&) = delete;
    BusinessLogManager& operator=(const BusinessLogManager&) = delete;

    // 获取日志器名称
    std::string get_logger_name(BusinessType business_type) {
        return "business_" + business_type_to_string(business_type);
    }

    // 获取或创建日志器
    std::shared_ptr<spdlog::logger> get_logger(BusinessType business_type) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!initialized_) {
            // 自动初始化使用默认配置
            initialized_ = true;
        }

        // 检查是否已存在
        auto it = loggers_.find(business_type);
        if (it != loggers_.end() && it->second) {
            return it->second;
        }

        // 创建新的日志器
        std::string logger_name = get_logger_name(business_type);
        std::string filename = config_.log_dir + "/" + business_type_to_string(business_type) + ".log";

        // 创建轮转文件sink
        auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            filename,
            config_.max_file_size,
            config_.max_files,
            config_.rotate_on_open
        );

        // 创建logger
        auto logger = std::make_shared<spdlog::logger>(logger_name, sink);
        logger->set_level(spdlog::level::info);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

        // 注册到spdlog全局registry
        spdlog::register_logger(logger);

        // 缓存到本地map
        loggers_[business_type] = logger;

        return logger;
    }

private:
    std::mutex mutex_;
    std::unordered_map<BusinessType, std::shared_ptr<spdlog::logger>> loggers_;
    LogConfig config_;
    bool initialized_ = false;
};

// 全局便捷函数

// 初始化日志系统
inline void init(const LogConfig& config = LogConfig()) {
    BusinessLogManager::instance().initialize(config);
}

// 写入日志
template <typename... Args>
inline void log(BusinessType business_type, spdlog::level::level_enum level, const std::string& format, Args&&... args) {
    BusinessLogManager::instance().log(business_type, level, format, std::forward<Args>(args)...);
}

// 各级别的便捷函数
template <typename... Args>
inline void trace(BusinessType business_type, const std::string& format, Args&&... args) {
    BusinessLogManager::instance().trace(business_type, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void debug(BusinessType business_type, const std::string& format, Args&&... args) {
    BusinessLogManager::instance().debug(business_type, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void info(BusinessType business_type, const std::string& format, Args&&... args) {
    BusinessLogManager::instance().info(business_type, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void warn(BusinessType business_type, const std::string& format, Args&&... args) {
    BusinessLogManager::instance().warn(business_type, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void error(BusinessType business_type, const std::string& format, Args&&... args) {
    BusinessLogManager::instance().error(business_type, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void critical(BusinessType business_type, const std::string& format, Args&&... args) {
    BusinessLogManager::instance().critical(business_type, format, std::forward<Args>(args)...);
}

// 刷新日志
inline void flush(BusinessType business_type) {
    BusinessLogManager::instance().flush(business_type);
}

inline void flush_all() {
    BusinessLogManager::instance().flush_all();
}

// 设置日志级别
inline void set_level(BusinessType business_type, spdlog::level::level_enum level) {
    BusinessLogManager::instance().set_level(business_type, level);
}

inline void set_all_levels(spdlog::level::level_enum level) {
    BusinessLogManager::instance().set_all_levels(level);
}

// 立即轮转
inline void rotate_now(BusinessType business_type) {
    BusinessLogManager::instance().rotate_now(business_type);
}

// 关闭日志系统
inline void shutdown() {
    BusinessLogManager::instance().shutdown();
}

} // namespace business_log
