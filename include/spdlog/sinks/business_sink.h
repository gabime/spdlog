// Copyright(c) 2025-present.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// 多业务日志分类 Sink
// 根据业务类型将日志写入不同的文件，每个业务类型有独立的日志轮转

#pragma once

#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/os.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/common.h>
#include <spdlog/fmt/fmt.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <tuple>

namespace spdlog {
namespace sinks {

// 业务类型枚举
enum class BusinessType {
    Unknown = 0,
    ScreenRecord,   // 录制屏幕
    DesktopOpen,    // 打开桌面
    KeyboardRecord, // 录制键盘
    AudioRecord,    // 录制声音
    Count
};

// 业务类型配置
struct BusinessConfig {
    std::string name;           // 业务名称
    std::string filename;       // 日志文件名
    size_t max_size = 30 * 1024 * 1024;  // 最大文件大小，默认30MB
    size_t max_files = 3;       // 最大保留文件数
};

// 单个业务的日志文件 Sink
class BusinessFileSink {
public:
    BusinessFileSink(const BusinessConfig& config, const file_event_handlers& event_handlers)
        : config_(config), file_helper_(event_handlers), event_handlers_(event_handlers) {
        if (config_.max_size == 0) {
            throw_spdlog_ex("BusinessFileSink: max_size cannot be zero");
        }
        
        // 创建日志目录
        details::os::create_dir(details::os::dir_name(config_.filename));
        
        // 打开当前日志文件
        file_helper_.open(calc_filename_(0));
        current_size_ = file_helper_.size();
    }
    
    void log(const details::log_msg& msg) {
        memory_buf_t formatted;
        // 使用默认的 pattern formatter
        pattern_formatter formatter;
        formatter.format(msg, formatted);
        
        auto new_size = current_size_ + formatted.size();
        
        // 检查是否需要轮转
        if (new_size > config_.max_size) {
            file_helper_.flush();
            if (file_helper_.size() > 0) {
                rotate_files_();
                new_size = formatted.size();
            }
        }
        
        file_helper_.write(formatted);
        current_size_ = new_size;
    }
    
    void flush() {
        file_helper_.flush();
    }
    
    void rotate() {
        rotate_files_();
    }
    
private:
    filename_t calc_filename_(size_t index) const {
        if (index == 0) {
            return config_.filename;
        }
        
        filename_t basename;
        filename_t ext;
        std::tie(basename, ext) = details::file_helper::split_by_extension(config_.filename);
        return fmt_lib::format(SPDLOG_FMT_STRING(SPDLOG_FILENAME_T("{}.{}{}")), basename, index, ext);
    }
    
    void rotate_files_() {
        using details::os::filename_to_str;
        using details::os::path_exists;
        
        file_helper_.close();
        
        // 轮转文件：log.txt -> log.1.txt -> log.2.txt -> log.3.txt (删除)
        for (auto i = config_.max_files; i > 0; --i) {
            filename_t src = calc_filename_(i - 1);
            if (!path_exists(src)) {
                continue;
            }
            
            filename_t target = calc_filename_(i);
            
            if (!rename_file_(src, target)) {
                // Windows 下可能需要重试
                details::os::sleep_for_millis(100);
                if (!rename_file_(src, target)) {
                    file_helper_.reopen(true);
                    current_size_ = 0;
                    throw_spdlog_ex("BusinessFileSink: failed renaming " + filename_to_str(src) +
                                    " to " + filename_to_str(target), errno);
                }
            }
        }
        
        file_helper_.reopen(true);
        current_size_ = 0;
    }
    
    bool rename_file_(const filename_t& src_filename, const filename_t& target_filename) {
        (void)details::os::remove(target_filename);
        return details::os::rename(src_filename, target_filename) == 0;
    }
    
    BusinessConfig config_;
    details::file_helper file_helper_;
    size_t current_size_ = 0;
    file_event_handlers event_handlers_;
};

// 多业务路由 Sink
// 根据日志消息中的业务标记，将日志路由到对应的业务日志文件
template <typename Mutex>
class business_sink : public base_sink<Mutex> {
public:
    explicit business_sink(const std::string& log_dir = "logs",
                          size_t max_size = 30 * 1024 * 1024,  // 30MB
                          size_t max_files = 3,
                          const file_event_handlers& event_handlers = {})
        : log_dir_(log_dir),
          max_size_(max_size),
          max_files_(max_files),
          event_handlers_(event_handlers) {
        // 创建日志目录
        details::os::create_dir(log_dir_);
        
        // 初始化默认业务配置
        business_configs_[BusinessType::ScreenRecord] = get_default_config_(BusinessType::ScreenRecord);
        business_configs_[BusinessType::DesktopOpen] = get_default_config_(BusinessType::DesktopOpen);
        business_configs_[BusinessType::KeyboardRecord] = get_default_config_(BusinessType::KeyboardRecord);
        business_configs_[BusinessType::AudioRecord] = get_default_config_(BusinessType::AudioRecord);
    }
    
    // 设置业务配置
    void set_business_config(BusinessType type, const std::string& name, 
                             const std::string& custom_filename = "") {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        
        auto& config = business_configs_[type];
        config.name = name;
        config.max_size = max_size_;
        config.max_files = max_files_;
        
        if (!custom_filename.empty()) {
            config.filename = custom_filename;
        } else {
            config.filename = log_dir_ + "/" + name + ".log";
        }
        
        // 如果 sink 已存在，需要重新初始化
        if (business_sinks_.find(type) != business_sinks_.end()) {
            business_sinks_.erase(type);
        }
    }
    
    // 获取业务 logger（用于直接写入特定业务日志）
    std::shared_ptr<logger> get_business_logger(BusinessType type) {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        
        auto it = business_loggers_.find(type);
        if (it != business_loggers_.end()) {
            return it->second;
        }
        
        // 创建专门的业务 logger
        std::string logger_name = get_business_type_name_(type);
        auto new_logger = std::make_shared<spdlog::logger>(logger_name);
        
        // 创建一个只写入特定业务的 sink
        ensure_sink_initialized_(type);
        
        business_loggers_[type] = new_logger;
        return new_logger;
    }
    
    // 静态方法：从日志消息中提取业务类型
    static BusinessType extract_business_type(const details::log_msg& msg) {
        // 从 logger 名称中提取业务类型
        std::string logger_name = msg.logger_name.data();
        
        if (logger_name.find("screen") != std::string::npos ||
            logger_name.find("SCREEN") != std::string::npos) {
            return BusinessType::ScreenRecord;
        }
        if (logger_name.find("desktop") != std::string::npos ||
            logger_name.find("DESKTOP") != std::string::npos) {
            return BusinessType::DesktopOpen;
        }
        if (logger_name.find("keyboard") != std::string::npos ||
            logger_name.find("KEYBOARD") != std::string::npos) {
            return BusinessType::KeyboardRecord;
        }
        if (logger_name.find("audio") != std::string::npos ||
            logger_name.find("AUDIO") != std::string::npos) {
            return BusinessType::AudioRecord;
        }
        
        return BusinessType::Unknown;
    }
    
protected:
    void sink_it_(const details::log_msg& msg) override {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        
        // 从消息中提取业务类型（可以通过消息内容或 logger 名称）
        BusinessType type = extract_business_type(msg);
        
        // 确保 sink 已初始化
        ensure_sink_initialized_(type);
        
        // 写入对应的业务日志
        auto it = business_sinks_.find(type);
        if (it != business_sinks_.end()) {
            it->second->log(msg);
        }
    }
    
    void flush_() override {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        
        for (auto& pair : business_sinks_) {
            pair.second->flush();
        }
    }
    
private:
    std::string log_dir_;
    size_t max_size_;
    size_t max_files_;
    file_event_handlers event_handlers_;
    
    // 业务类型到 Sink 的映射
    std::map<BusinessType, std::unique_ptr<BusinessFileSink>> business_sinks_;
    std::map<BusinessType, std::shared_ptr<logger>> business_loggers_;
    std::map<BusinessType, BusinessConfig> business_configs_;
    
    void ensure_sink_initialized_(BusinessType type) {
        if (business_sinks_.find(type) == business_sinks_.end()) {
            auto it = business_configs_.find(type);
            if (it != business_configs_.end()) {
                business_sinks_[type] = details::make_unique<BusinessFileSink>(it->second, event_handlers_);
            }
        }
    }
    
    BusinessConfig get_default_config_(BusinessType type) {
        BusinessConfig config;
        config.max_size = max_size_;
        config.max_files = max_files_;
        
        switch (type) {
            case BusinessType::ScreenRecord:
                config.name = "screen_record";
                config.filename = log_dir_ + "/screen_record.log";
                break;
            case BusinessType::DesktopOpen:
                config.name = "desktop_open";
                config.filename = log_dir_ + "/desktop_open.log";
                break;
            case BusinessType::KeyboardRecord:
                config.name = "keyboard_record";
                config.filename = log_dir_ + "/keyboard_record.log";
                break;
            case BusinessType::AudioRecord:
                config.name = "audio_record";
                config.filename = log_dir_ + "/audio_record.log";
                break;
            default:
                config.name = "unknown";
                config.filename = log_dir_ + "/unknown.log";
                break;
        }
        
        return config;
    }
    
    std::string get_business_type_name_(BusinessType type) {
        switch (type) {
            case BusinessType::ScreenRecord:   return "screen_record";
            case BusinessType::DesktopOpen:    return "desktop_open";
            case BusinessType::KeyboardRecord: return "keyboard_record";
            case BusinessType::AudioRecord:    return "audio_record";
            default:                           return "unknown";
        }
    }
};

using business_sink_mt = business_sink<std::mutex>;
using business_sink_st = business_sink<details::null_mutex>;

// 工厂函数
template <typename Factory = spdlog::synchronous_factory>
std::shared_ptr<logger> business_logger_mt(const std::string& logger_name,
                                           const std::string& log_dir = "logs",
                                           size_t max_size = 30 * 1024 * 1024,
                                           size_t max_files = 3,
                                           const file_event_handlers& event_handlers = {}) {
    return Factory::template create<sinks::business_sink_mt>(
        logger_name, log_dir, max_size, max_files, event_handlers);
}

template <typename Factory = spdlog::synchronous_factory>
std::shared_ptr<logger> business_logger_st(const std::string& logger_name,
                                           const std::string& log_dir = "logs",
                                           size_t max_size = 30 * 1024 * 1024,
                                           size_t max_files = 3,
                                           const file_event_handlers& event_handlers = {}) {
    return Factory::template create<sinks::business_sink_st>(
        logger_name, log_dir, max_size, max_files, event_handlers);
}

}  // namespace sinks

// 便捷函数：获取业务类型名称
inline std::string business_type_to_string(sinks::BusinessType type) {
    switch (type) {
        case sinks::BusinessType::ScreenRecord:   return "screen_record";
        case sinks::BusinessType::DesktopOpen:    return "desktop_open";
        case sinks::BusinessType::KeyboardRecord: return "keyboard_record";
        case sinks::BusinessType::AudioRecord:    return "audio_record";
        default:                                  return "unknown";
    }
}

inline sinks::BusinessType string_to_business_type(const std::string& str) {
    if (str == "screen_record" || str == "SCREEN")   return sinks::BusinessType::ScreenRecord;
    if (str == "desktop_open" || str == "DESKTOP")   return sinks::BusinessType::DesktopOpen;
    if (str == "keyboard_record" || str == "KEYBOARD") return sinks::BusinessType::KeyboardRecord;
    if (str == "audio_record" || str == "AUDIO")     return sinks::BusinessType::AudioRecord;
    return sinks::BusinessType::Unknown;
}

}  // namespace spdlog
