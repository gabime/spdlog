// Copyright(c) 2025-present.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// 多业务日志管理器
// 提供简洁的 API 来按业务类型记录日志

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/business_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>
#include <string>
#include <vector>

namespace spdlog {

// 业务类型定义（与 business_sink.h 保持一致）
using BusinessType = sinks::BusinessType;

// 多业务日志管理器类
// 封装了 business_sink，提供更简洁的 API
class BusinessLogger {
public:
    // 构造函数
    // @param log_dir: 日志文件存放目录
    // @param max_size: 单个日志文件最大大小（字节），默认 30MB
    // @param max_files: 保留的日志文件数量，默认 3 个
    // @param enable_console: 是否同时输出到控制台
    BusinessLogger(const std::string& log_dir = "logs",
                   size_t max_size = 30 * 1024 * 1024,
                   size_t max_files = 3,
                   bool enable_console = false)
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
    
    // 获取指定业务类型的 logger
    std::shared_ptr<logger> get_logger(BusinessType type) {
        size_t index = static_cast<size_t>(type);
        if (index < business_loggers_.size() && business_loggers_[index]) {
            return business_loggers_[index];
        }
        
        // 如果 logger 不存在，创建一个
        auto new_logger = create_logger_(type);
        if (index >= business_loggers_.size()) {
            business_loggers_.resize(index + 1);
        }
        business_loggers_[index] = new_logger;
        return new_logger;
    }
    
    // 便捷方法：记录屏幕录制日志
    template<typename... Args>
    void screen(level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        auto lg = get_logger(BusinessType::ScreenRecord);
        lg->log(lvl, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void screen_info(format_string_t<Args...> fmt, Args&&... args) {
        screen(level::info, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void screen_debug(format_string_t<Args...> fmt, Args&&... args) {
        screen(level::debug, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void screen_warn(format_string_t<Args...> fmt, Args&&... args) {
        screen(level::warn, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void screen_error(format_string_t<Args...> fmt, Args&&... args) {
        screen(level::err, fmt, std::forward<Args>(args)...);
    }
    
    // 便捷方法：记录桌面操作日志
    template<typename... Args>
    void desktop(level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        auto lg = get_logger(BusinessType::DesktopOpen);
        lg->log(lvl, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void desktop_info(format_string_t<Args...> fmt, Args&&... args) {
        desktop(level::info, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void desktop_debug(format_string_t<Args...> fmt, Args&&... args) {
        desktop(level::debug, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void desktop_warn(format_string_t<Args...> fmt, Args&&... args) {
        desktop(level::warn, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void desktop_error(format_string_t<Args...> fmt, Args&&... args) {
        desktop(level::err, fmt, std::forward<Args>(args)...);
    }
    
    // 便捷方法：记录键盘记录日志
    template<typename... Args>
    void keyboard(level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        auto lg = get_logger(BusinessType::KeyboardRecord);
        lg->log(lvl, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void keyboard_info(format_string_t<Args...> fmt, Args&&... args) {
        keyboard(level::info, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void keyboard_debug(format_string_t<Args...> fmt, Args&&... args) {
        keyboard(level::debug, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void keyboard_warn(format_string_t<Args...> fmt, Args&&... args) {
        keyboard(level::warn, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void keyboard_error(format_string_t<Args...> fmt, Args&&... args) {
        keyboard(level::err, fmt, std::forward<Args>(args)...);
    }
    
    // 便捷方法：记录音频录制日志
    template<typename... Args>
    void audio(level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        auto lg = get_logger(BusinessType::AudioRecord);
        lg->log(lvl, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void audio_info(format_string_t<Args...> fmt, Args&&... args) {
        audio(level::info, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void audio_debug(format_string_t<Args...> fmt, Args&&... args) {
        audio(level::debug, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void audio_warn(format_string_t<Args...> fmt, Args&&... args) {
        audio(level::warn, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void audio_error(format_string_t<Args...> fmt, Args&&... args) {
        audio(level::err, fmt, std::forward<Args>(args)...);
    }
    
    // 通用日志方法
    template<typename... Args>
    void log(BusinessType type, level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        auto lg = get_logger(type);
        lg->log(lvl, fmt, std::forward<Args>(args)...);
    }
    
    // 刷新所有日志
    void flush() {
        business_sink_->flush();
        if (console_logger_) {
            console_logger_->flush();
        }
    }
    
    // 设置全局日志级别
    void set_level(level::level_enum lvl) {
        for (auto& lg : business_loggers_) {
            if (lg) {
                lg->set_level(lvl);
            }
        }
    }
    
    // 设置特定业务的日志级别
    void set_business_level(BusinessType type, level::level_enum lvl) {
        auto lg = get_logger(type);
        lg->set_level(lvl);
    }
    
    // 自定义业务配置
    void set_business_config(BusinessType type, 
                             const std::string& name,
                             const std::string& custom_filename = "") {
        business_sink_->set_business_config(type, name, custom_filename);
    }

private:
    std::shared_ptr<sinks::business_sink_mt> business_sink_;
    std::vector<std::shared_ptr<logger>> business_loggers_;
    std::shared_ptr<logger> console_logger_;
    bool enable_console_;
    
    void init_loggers_() {
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
    
    std::shared_ptr<logger> create_logger_(BusinessType type) {
        std::string name = business_type_to_string(type);
        return std::make_shared<logger>(name, business_sink_);
    }
};

// 单例模式获取全局 BusinessLogger 实例
class BusinessLoggerManager {
public:
    static BusinessLoggerManager& instance() {
        static BusinessLoggerManager inst;
        return inst;
    }
    
    // 初始化
    void initialize(const std::string& log_dir = "logs",
                    size_t max_size = 30 * 1024 * 1024,
                    size_t max_files = 3,
                    bool enable_console = false) {
        logger_ = std::make_shared<BusinessLogger>(log_dir, max_size, max_files, enable_console);
    }
    
    // 获取 BusinessLogger 实例
    std::shared_ptr<BusinessLogger> get_logger() {
        if (!logger_) {
            // 如果未初始化，使用默认配置
            initialize();
        }
        return logger_;
    }
    
    // 便捷方法
    template<typename... Args>
    void screen_info(format_string_t<Args...> fmt, Args&&... args) {
        if (logger_) {
            logger_->screen_info(fmt, std::forward<Args>(args)...);
        }
    }
    
    template<typename... Args>
    void desktop_info(format_string_t<Args...> fmt, Args&&... args) {
        if (logger_) {
            logger_->desktop_info(fmt, std::forward<Args>(args)...);
        }
    }
    
    template<typename... Args>
    void keyboard_info(format_string_t<Args...> fmt, Args&&... args) {
        if (logger_) {
            logger_->keyboard_info(fmt, std::forward<Args>(args)...);
        }
    }
    
    template<typename... Args>
    void audio_info(format_string_t<Args...> fmt, Args&&... args) {
        if (logger_) {
            logger_->audio_info(fmt, std::forward<Args>(args)...);
        }
    }
    
    template<typename... Args>
    void screen_error(format_string_t<Args...> fmt, Args&&... args) {
        if (logger_) {
            logger_->screen_error(fmt, std::forward<Args>(args)...);
        }
    }
    
    template<typename... Args>
    void desktop_error(format_string_t<Args...> fmt, Args&&... args) {
        if (logger_) {
            logger_->desktop_error(fmt, std::forward<Args>(args)...);
        }
    }
    
    template<typename... Args>
    void keyboard_error(format_string_t<Args...> fmt, Args&&... args) {
        if (logger_) {
            logger_->keyboard_error(fmt, std::forward<Args>(args)...);
        }
    }
    
    template<typename... Args>
    void audio_error(format_string_t<Args...> fmt, Args&&... args) {
        if (logger_) {
            logger_->audio_error(fmt, std::forward<Args>(args)...);
        }
    }
    
    void flush() {
        if (logger_) {
            logger_->flush();
        }
    }

private:
    BusinessLoggerManager() = default;
    ~BusinessLoggerManager() = default;
    
    BusinessLoggerManager(const BusinessLoggerManager&) = delete;
    BusinessLoggerManager& operator=(const BusinessLoggerManager&) = delete;
    
    std::shared_ptr<BusinessLogger> logger_;
};

// 全局便捷函数
inline BusinessLoggerManager& business_log_manager() {
    return BusinessLoggerManager::instance();
}

}  // namespace spdlog
