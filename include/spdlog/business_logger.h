// Copyright(c) 2025 spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/common.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace spdlog {

// 业务类型枚举
enum class business_type {
    screen_record,   // 录制屏幕
    desktop_open,    // 打开桌面
    keyboard_record, // 录制键盘
    sound_record     // 录制声音
};

// 业务日志管理器
class business_logger_manager {
public:
    // 单例模式获取实例
    static business_logger_manager& instance();

    // 初始化业务日志管理器，设置日志文件基础目录
    // 支持 std::string (所有平台) 和 std::wstring (Windows wchar 模式)
    void initialize(const filename_t& base_dir);

    // 获取指定业务的logger
    std::shared_ptr<logger> get_logger(business_type type);

    // 便捷方法：写入业务日志
    void log(business_type type, level::level_enum lvl, const std::string& msg);

    // 设置所有业务日志的日志级别
    void set_level(level::level_enum lvl);

    // 刷新所有业务日志
    void flush_all();

    // 关闭所有业务日志
    void shutdown();

private:
    business_logger_manager() = default;
    ~business_logger_manager() = default;
    business_logger_manager(const business_logger_manager&) = delete;
    business_logger_manager& operator=(const business_logger_manager&) = delete;

    // 创建业务logger
    std::shared_ptr<logger> create_logger(business_type type);

    // 获取业务类型对应的名称
    // 返回 std::string，因为业务名称是 ASCII 字符
    std::string get_business_name(business_type type);

    // 获取业务类型对应的日志文件名
    // 返回 filename_t 以支持 Windows 宽字符路径
    filename_t get_log_filename(business_type type);

private:
    filename_t base_dir_;
    std::unordered_map<business_type, std::shared_ptr<logger>> loggers_;
    std::mutex mutex_;
    bool initialized_ = false;

    // 日志配置参数
    static constexpr size_t max_file_size = 30 * 1024 * 1024;  // 30MB
    static constexpr size_t max_files = 3;                     // 最多3个文件
};

// 便捷函数：获取业务logger
inline std::shared_ptr<logger> business_logger(business_type type) {
    return business_logger_manager::instance().get_logger(type);
}

// 便捷宏定义，用于快速记录业务日志
// 使用 spdlog::fmt_lib 以兼容外部 fmt、内置 fmt 或 std::format
#define SPDLOG_BUSINESS_SCREEN_RECORD(lvl, ...) \
    spdlog::business_logger_manager::instance().log(spdlog::business_type::screen_record, lvl, spdlog::fmt_lib::format(__VA_ARGS__))

#define SPDLOG_BUSINESS_DESKTOP_OPEN(lvl, ...) \
    spdlog::business_logger_manager::instance().log(spdlog::business_type::desktop_open, lvl, spdlog::fmt_lib::format(__VA_ARGS__))

#define SPDLOG_BUSINESS_KEYBOARD_RECORD(lvl, ...) \
    spdlog::business_logger_manager::instance().log(spdlog::business_type::keyboard_record, lvl, spdlog::fmt_lib::format(__VA_ARGS__))

#define SPDLOG_BUSINESS_SOUND_RECORD(lvl, ...) \
    spdlog::business_logger_manager::instance().log(spdlog::business_type::sound_record, lvl, spdlog::fmt_lib::format(__VA_ARGS__))

}  // namespace spdlog
