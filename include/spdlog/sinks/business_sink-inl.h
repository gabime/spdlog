// Copyright(c) 2025-present.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/sinks/business_sink.h>
#endif

#include <spdlog/common.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/details/os.h>
#include <spdlog/fmt/fmt.h>

#include <cerrno>
#include <ctime>
#include <mutex>
#include <string>
#include <tuple>

namespace spdlog {
namespace sinks {

// BusinessFileSink 实现
inline BusinessFileSink::BusinessFileSink(const BusinessConfig& config,
                                          const file_event_handlers& event_handlers)
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

inline filename_t BusinessFileSink::calc_filename_(size_t index) const {
    if (index == 0) {
        return config_.filename;
    }
    
    filename_t basename;
    filename_t ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(config_.filename);
    return fmt_lib::format(SPDLOG_FMT_STRING(SPDLOG_FILENAME_T("{}.{}{}")), basename, index, ext);
}

inline void BusinessFileSink::log(const details::log_msg& msg) {
    memory_buf_t formatted;
    // 使用默认的 pattern formatter
    spdlog::pattern_formatter formatter;
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

inline void BusinessFileSink::flush() {
    file_helper_.flush();
}

inline void BusinessFileSink::rotate() {
    rotate_files_();
}

inline void BusinessFileSink::rotate_files_() {
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

inline bool BusinessFileSink::rename_file_(const filename_t& src_filename,
                                           const filename_t& target_filename) {
    (void)details::os::remove(target_filename);
    return details::os::rename(src_filename, target_filename) == 0;
}

// business_sink 实现
template <typename Mutex>
inline business_sink<Mutex>::business_sink(const std::string& log_dir,
                                           size_t max_size,
                                           size_t max_files,
                                           const file_event_handlers& event_handlers)
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

template <typename Mutex>
inline BusinessConfig business_sink<Mutex>::get_default_config_(BusinessType type) {
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

template <typename Mutex>
inline void business_sink<Mutex>::set_business_config(BusinessType type,
                                                       const std::string& name,
                                                       const std::string& custom_filename) {
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

template <typename Mutex>
inline void business_sink<Mutex>::ensure_sink_initialized_(BusinessType type) {
    if (business_sinks_.find(type) == business_sinks_.end()) {
        auto it = business_configs_.find(type);
        if (it != business_configs_.end()) {
            business_sinks_[type] = std::make_unique<BusinessFileSink>(it->second, event_handlers_);
        }
    }
}

template <typename Mutex>
inline void business_sink<Mutex>::sink_it_(const details::log_msg& msg) {
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

template <typename Mutex>
inline void business_sink<Mutex>::flush_() {
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    
    for (auto& pair : business_sinks_) {
        pair.second->flush();
    }
}

template <typename Mutex>
inline BusinessType business_sink<Mutex>::extract_business_type(const details::log_msg& msg) {
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

template <typename Mutex>
inline std::shared_ptr<logger> business_sink<Mutex>::get_business_logger(BusinessType type) {
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    
    auto it = business_loggers_.find(type);
    if (it != business_loggers_.end()) {
        return it->second;
    }
    
    // 创建专门的业务 logger
    std::string logger_name = business_type_to_string(type);
    auto logger = std::make_shared<spdlog::logger>(logger_name);
    
    // 创建一个只写入特定业务的 sink
    ensure_sink_initialized_(type);
    
    business_loggers_[type] = logger;
    return logger;
}

template <typename Mutex>
template<typename... Args>
inline void business_sink<Mutex>::log_business(BusinessType type,
                                               level::level_enum lvl,
                                               format_string_t<Args...> fmt,
                                               Args&&... args) {
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    
    ensure_sink_initialized_(type);
    
    auto it = business_sinks_.find(type);
    if (it != business_sinks_.end()) {
        // 创建日志消息
        details::log_msg msg;
        msg.level = lvl;
        msg.time = log_clock::now();
        msg.logger_name = business_type_to_string(type);
        
        // 格式化消息
        fmt_lib::format_to(std::back_inserter(msg.payload), fmt, std::forward<Args>(args)...);
        
        it->second->log(msg);
    }
}

}  // namespace sinks
}  // namespace spdlog
