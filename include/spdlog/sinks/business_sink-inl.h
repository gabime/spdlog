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

namespace spdlog {
namespace sinks {

// business_file_manager 实现

inline business_file_manager::business_file_manager(filename_t base_filename,
                                                     std::size_t max_size,
                                                     std::size_t max_files,
                                                     const file_event_handlers &event_handlers)
    : base_filename_(std::move(base_filename)),
      max_size_(max_size),
      max_files_(max_files),
      current_size_(0),
      file_helper_{event_handlers} {
    if (max_size == 0) {
        throw_spdlog_ex("business_file_manager constructor: max_size arg cannot be zero");
    }
    if (max_files == 0) {
        throw_spdlog_ex("business_file_manager constructor: max_files arg cannot be zero");
    }
    file_helper_.open(calc_filename_(0));
    current_size_ = file_helper_.size();
}

inline filename_t business_file_manager::calc_filename_(std::size_t index) const {
    if (index == 0U) {
        return base_filename_;
    }
    filename_t basename;
    filename_t ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(base_filename_);
    return fmt_lib::format(SPDLOG_FMT_STRING(SPDLOG_FILENAME_T("{}.{}{}")), basename, index, ext);
}

inline void business_file_manager::write(const memory_buf_t &formatted) {
    auto new_size = current_size_ + formatted.size();
    
    // 如果超过最大大小，执行轮转
    if (new_size > max_size_) {
        file_helper_.flush();
        if (file_helper_.size() > 0) {
            rotate_();
            new_size = formatted.size();
        }
    }
    file_helper_.write(formatted);
    current_size_ = new_size;
}

inline void business_file_manager::flush() {
    file_helper_.flush();
}

inline filename_t business_file_manager::filename() const {
    return file_helper_.filename();
}

inline std::size_t business_file_manager::get_max_size() const {
    return max_size_;
}

inline std::size_t business_file_manager::get_max_files() const {
    return max_files_;
}

inline void business_file_manager::rotate_() {
    using details::os::filename_to_str;
    using details::os::path_exists;

    file_helper_.close();
    
    // 轮转文件：log.txt -> log.1.txt -> log.2.txt -> log.3.txt (删除)
    for (auto i = max_files_; i > 0; --i) {
        filename_t src = calc_filename_(i - 1);
        if (!path_exists(src)) {
            continue;
        }
        filename_t target = calc_filename_(i);

        if (!rename_file_(src, target)) {
            // 如果重命名失败，尝试删除目标文件再重试
            if (path_exists(target)) {
                details::os::remove(target);
                rename_file_(src, target);
            }
        }
    }
    
    file_helper_.reopen(true);
    current_size_ = 0;
}

inline bool business_file_manager::rename_file_(const filename_t &src, const filename_t &target) {
    return details::os::rename(src, target) == 0;
}

// business_sink 实现

template <typename Mutex>
inline business_sink<Mutex>::business_sink(filename_t base_directory,
                                            std::size_t max_size,
                                            std::size_t max_files,
                                            const file_event_handlers &event_handlers)
    : base_directory_(std::move(base_directory)),
      max_size_(max_size),
      max_files_(max_files),
      event_handlers_(event_handlers),
      current_business_(business_type::screen_record) {
    if (max_size == 0) {
        throw_spdlog_ex("business_sink constructor: max_size arg cannot be zero");
    }
    if (max_files == 0) {
        throw_spdlog_ex("business_sink constructor: max_files arg cannot be zero");
    }
}

template <typename Mutex>
inline void business_sink<Mutex>::set_current_business(business_type type) {
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    current_business_ = type;
}

template <typename Mutex>
inline business_type business_sink<Mutex>::get_current_business() {
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    return current_business_;
}

template <typename Mutex>
inline filename_t business_sink<Mutex>::get_business_filename(business_type type) {
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    auto it = managers_.find(type);
    if (it != managers_.end()) {
        return it->second->filename();
    }
    // 返回预期的文件名
    filename_t type_str = SPDLOG_FILENAME_T(business_type_to_string(type));
    filename_t filename = base_directory_;
    filename += SPDLOG_FILENAME_T("/");
    filename += type_str;
    filename += SPDLOG_FILENAME_T(".log");
    return filename;
}

template <typename Mutex>
inline void business_sink<Mutex>::rotate_business(business_type type) {
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    auto it = managers_.find(type);
    if (it != managers_.end()) {
        it->second->flush();
    }
}

template <typename Mutex>
inline void business_sink<Mutex>::sink_it_(const details::log_msg &msg) {
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    
    // 从消息中提取业务类型（如果有），否则使用当前设置的类型
    business_type type = current_business_;
    
    // 获取对应业务的文件管理器并写入
    business_file_manager &manager = get_manager_(type);
    manager.write(formatted);
}

template <typename Mutex>
inline void business_sink<Mutex>::flush_() {
    for (auto &pair : managers_) {
        pair.second->flush();
    }
}

template <typename Mutex>
inline business_file_manager &business_sink<Mutex>::get_manager_(business_type type) {
    auto it = managers_.find(type);
    if (it == managers_.end()) {
        // 创建新的文件管理器
        filename_t type_str = SPDLOG_FILENAME_T(business_type_to_string(type));
        filename_t filename = base_directory_;
        filename += SPDLOG_FILENAME_T("/");
        filename += type_str;
        filename += SPDLOG_FILENAME_T(".log");
        
        auto manager = std::make_unique<business_file_manager>(
            filename, max_size_, max_files_, event_handlers_);
        auto result = managers_.emplace(type, std::move(manager));
        return *result.first->second;
    }
    return *it->second;
}

}  // namespace sinks
}  // namespace spdlog
