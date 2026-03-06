// Copyright(c) 2025-present.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/business_file_sink.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace spdlog {

//
// 业务日志管理器
// 为不同业务类型提供独立的日志记录功能
//
// 使用示例：
//   auto manager = std::make_shared<BusinessLoggerManager>("./logs");
//   manager->log(BusinessType::Screen, level::info, "开始录制屏幕");
//   manager->log(BusinessType::Keyboard, level::debug, "按键: {}", key);
//
class BusinessLoggerManager {
public:
    // 构造函数
    // @param base_dir: 日志文件存放的基础目录
    // @param max_size: 单个日志文件最大大小（字节），默认30MB
    // @param max_files: 保留的日志文件数量，默认3个
    // @param rotate_on_open: 是否在打开时轮转
    BusinessLoggerManager(const filename_t& base_dir,
                          std::size_t max_size = 30 * 1024 * 1024,
                          std::size_t max_files = 3,
                          bool rotate_on_open = false)
        : base_dir_(base_dir),
          max_size_(max_size),
          max_files_(max_files),
          rotate_on_open_(rotate_on_open) {
        // 预创建所有业务类型的日志器
        initialize_loggers();
    }

    // 析构函数
    ~BusinessLoggerManager() {
        flush_all();
    }

    // 获取指定业务类型的日志器
    std::shared_ptr<logger> get_logger(BusinessType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = loggers_.find(type);
        if (it != loggers_.end()) {
            return it->second;
        }
        return nullptr;
    }

    // 写入日志（带业务类型）
    template <typename... Args>
    void log(BusinessType type, level::level_enum lvl, format_string_t<Args...> fmt, Args &&...args) {
        auto lg = get_logger(type);
        if (lg) {
            lg->log(lvl, fmt, std::forward<Args>(args)...);
        }
    }

    // 便捷方法：trace 级别
    template <typename... Args>
    void trace(BusinessType type, format_string_t<Args...> fmt, Args &&...args) {
        log(type, level::trace, fmt, std::forward<Args>(args)...);
    }

    // 便捷方法：debug 级别
    template <typename... Args>
    void debug(BusinessType type, format_string_t<Args...> fmt, Args &&...args) {
        log(type, level::debug, fmt, std::forward<Args>(args)...);
    }

    // 便捷方法：info 级别
    template <typename... Args>
    void info(BusinessType type, format_string_t<Args...> fmt, Args &&...args) {
        log(type, level::info, fmt, std::forward<Args>(args)...);
    }

    // 便捷方法：warn 级别
    template <typename... Args>
    void warn(BusinessType type, format_string_t<Args...> fmt, Args &&...args) {
        log(type, level::warn, fmt, std::forward<Args>(args)...);
    }

    // 便捷方法：error 级别
    template <typename... Args>
    void error(BusinessType type, format_string_t<Args...> fmt, Args &&...args) {
        log(type, level::err, fmt, std::forward<Args>(args)...);
    }

    // 便捷方法：critical 级别
    template <typename... Args>
    void critical(BusinessType type, format_string_t<Args...> fmt, Args &&...args) {
        log(type, level::critical, fmt, std::forward<Args>(args)...);
    }

    // 设置指定业务类型的日志级别
    void set_level(BusinessType type, level::level_enum lvl) {
        auto lg = get_logger(type);
        if (lg) {
            lg->set_level(lvl);
        }
    }

    // 设置所有业务类型的日志级别
    void set_all_levels(level::level_enum lvl) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : loggers_) {
            pair.second->set_level(lvl);
        }
    }

    // 设置指定业务类型的日志格式
    void set_pattern(BusinessType type, const std::string& pattern) {
        auto lg = get_logger(type);
        if (lg) {
            lg->set_pattern(pattern);
        }
    }

    // 设置所有业务类型的日志格式
    void set_all_patterns(const std::string& pattern) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : loggers_) {
            pair.second->set_pattern(pattern);
        }
    }

    // 刷新指定业务类型的日志
    void flush(BusinessType type) {
        auto lg = get_logger(type);
        if (lg) {
            lg->flush();
        }
    }

    // 刷新所有业务类型的日志
    void flush_all() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : loggers_) {
            pair.second->flush();
        }
    }

    // 手动触发指定业务类型的日志轮转
    void rotate(BusinessType type) {
        // 通过重新创建 logger 来触发轮转
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = sinks_.find(type);
        if (it != sinks_.end()) {
            it->second->rotate_now();
        }
    }

    // 获取日志文件路径
    filename_t get_log_file_path(BusinessType type) const {
        return generate_filename_(type);
    }

    // 获取所有业务类型
    std::vector<BusinessType> get_all_business_types() const {
        return {BusinessType::Screen, BusinessType::Desktop, 
                BusinessType::Keyboard, BusinessType::Audio};
    }

private:
    // 初始化所有业务类型的日志器
    void initialize_loggers() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<BusinessType> types = {
            BusinessType::Screen, 
            BusinessType::Desktop, 
            BusinessType::Keyboard, 
            BusinessType::Audio
        };

        for (auto type : types) {
            create_logger_(type);
        }
    }

    // 创建单个业务类型的日志器
    void create_logger_(BusinessType type) {
        filename_t filename = generate_filename_(type);
        std::string logger_name = std::string("business_") + business_type_to_string(type);
        
        // 创建 rotating_file_sink
        auto sink = std::make_shared<sinks::rotating_file_sink_mt>(
            filename, max_size_, max_files_, rotate_on_open_);
        
        // 创建 logger
        auto lg = std::make_shared<logger>(logger_name, sink);
        
        // 设置默认格式
        lg->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        
        sinks_[type] = sink;
        loggers_[type] = lg;
    }

    // 生成业务日志文件名
    filename_t generate_filename_(BusinessType type) const {
        const char* business_name = business_type_to_string(type);
        return fmt_lib::format(SPDLOG_FMT_STRING(SPDLOG_FILENAME_T("{}/{}.log")), 
                               base_dir_, SPDLOG_FILENAME_T(business_name));
    }

    filename_t base_dir_;
    std::size_t max_size_;
    std::size_t max_files_;
    bool rotate_on_open_;
    
    mutable std::mutex mutex_;
    std::unordered_map<BusinessType, std::shared_ptr<sinks::rotating_file_sink_mt>> sinks_;
    std::unordered_map<BusinessType, std::shared_ptr<logger>> loggers_;
};

}  // namespace spdlog
