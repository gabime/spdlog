// Copyright(c) 2025-present.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/sinks/base_sink.h>

#include <chrono>
#include <map>
#include <mutex>
#include <string>

namespace spdlog {
namespace sinks {

// 业务类型枚举
enum class business_type {
    screen_record,   // 录制屏幕
    desktop_open,    // 打开桌面
    keyboard_record, // 录制键盘
    sound_record     // 录制声音
};

// 业务类型工具函数
inline std::string business_type_to_string(business_type type) {
    switch (type) {
        case business_type::screen_record:   return "screen_record";
        case business_type::desktop_open:    return "desktop_open";
        case business_type::keyboard_record: return "keyboard_record";
        case business_type::sound_record:    return "sound_record";
        default:                             return "unknown";
    }
}

// 单个业务日志文件管理器
class business_file_manager {
public:
    static constexpr std::size_t default_max_files = 3;
    static constexpr std::size_t default_max_size = 30 * 1024 * 1024; // 30MB

    business_file_manager(filename_t base_filename,
                          std::size_t max_size = default_max_size,
                          std::size_t max_files = default_max_files,
                          const file_event_handlers &event_handlers = {});

    void write(const memory_buf_t &formatted);
    void flush();
    filename_t filename() const;
    std::size_t get_max_size() const;
    std::size_t get_max_files() const;

private:
    void rotate_();
    filename_t calc_filename_(std::size_t index) const;
    bool rename_file_(const filename_t &src, const filename_t &target);

    filename_t base_filename_;
    std::size_t max_size_;
    std::size_t max_files_;
    std::size_t current_size_;
    details::file_helper file_helper_;
};

// 业务日志 sink
// 根据不同的业务类型将日志写入不同的文件，每个业务独立轮转
template <typename Mutex>
class business_sink final : public base_sink<Mutex> {
public:
    static constexpr std::size_t default_max_files = 3;
    static constexpr std::size_t default_max_size = 30 * 1024 * 1024; // 30MB

    business_sink(filename_t base_directory,
                  std::size_t max_size = default_max_size,
                  std::size_t max_files = default_max_files,
                  const file_event_handlers &event_handlers = {});

    // 设置当前业务类型（线程安全）
    void set_current_business(business_type type);
    
    // 获取当前业务类型
    business_type get_current_business();
    
    // 获取指定业务的日志文件名
    filename_t get_business_filename(business_type type);
    
    // 强制轮转指定业务的日志文件
    void rotate_business(business_type type);

protected:
    void sink_it_(const details::log_msg &msg) override;
    void flush_() override;

private:
    // 获取或创建业务文件管理器
    business_file_manager &get_manager_(business_type type);

    filename_t base_directory_;
    std::size_t max_size_;
    std::size_t max_files_;
    file_event_handlers event_handlers_;
    
    // 每个业务类型对应一个文件管理器
    std::map<business_type, std::unique_ptr<business_file_manager>> managers_;
    
    // 当前业务类型（线程本地存储）
    business_type current_business_;
};

using business_sink_mt = business_sink<std::mutex>;
using business_sink_st = business_sink<details::null_mutex>;

}  // namespace sinks

// 工厂函数
template <typename Factory = spdlog::synchronous_factory>
std::shared_ptr<logger> business_logger_mt(const std::string &logger_name,
                                           const filename_t &base_directory,
                                           size_t max_file_size = sinks::business_sink_mt::default_max_size,
                                           size_t max_files = sinks::business_sink_mt::default_max_files,
                                           const file_event_handlers &event_handlers = {}) {
    return Factory::template create<sinks::business_sink_mt>(
        logger_name, base_directory, max_file_size, max_files, event_handlers);
}

template <typename Factory = spdlog::synchronous_factory>
std::shared_ptr<logger> business_logger_st(const std::string &logger_name,
                                           const filename_t &base_directory,
                                           size_t max_file_size = sinks::business_sink_st::default_max_size,
                                           size_t max_files = sinks::business_sink_st::default_max_files,
                                           const file_event_handlers &event_handlers = {}) {
    return Factory::template create<sinks::business_sink_st>(
        logger_name, base_directory, max_file_size, max_files, event_handlers);
}

}  // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include <spdlog/sinks/business_sink-inl.h>
#endif
