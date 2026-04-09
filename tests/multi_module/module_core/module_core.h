#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <memory>
#include <string>

/**
 * @brief 核心日志模块
 * 
 * 该模块提供应用程序核心功能的日志记录能力。
 * 使用单例模式确保全局唯一的日志器实例。
 * 
 * 主要特性：
 * - 同步日志记录
 * - 同时输出到文件和控制台
 * - 独立的日志级别控制
 * - 跨平台文件名支持
 */
namespace module_core {

/**
 * @brief 核心日志器单例类
 * 
 * 封装 spdlog 日志器，提供核心模块专用的日志功能。
 * 使用 Meyer's Singleton 模式确保线程安全的延迟初始化。
 */
class CoreLogger {
public:
    /**
     * @brief 获取单例实例
     * @return CoreLogger& 单例引用
     */
    static CoreLogger& instance();
    
    /**
     * @brief 初始化日志器
     * @param log_dir 日志文件目录路径
     * 
     * 创建文件 sink 和控制台 sink，设置日志格式和级别。
     * 日志文件名为 "core.log"。
     */
    void init(const spdlog::filename_t& log_dir);
    
    /**
     * @brief 关闭日志器
     * 
     * 刷新所有缓冲区，从 spdlog 注册表中移除日志器，
     * 释放相关资源。
     */
    void shutdown();
    
    /**
     * @brief 获取底层 spdlog 日志器指针
     * @return std::shared_ptr<spdlog::logger> 日志器共享指针
     */
    std::shared_ptr<spdlog::logger> get_logger() const;
    
    /**
     * @brief 获取日志文件路径
     * @return spdlog::filename_t 日志文件完整路径
     */
    spdlog::filename_t get_log_file() const;
    
    /**
     * @brief 记录 INFO 级别日志
     * @param message 日志消息内容
     * 
     * 消息会自动添加 "[Core]" 前缀标识模块来源。
     */
    void log_info(const std::string& message);
    
    /**
     * @brief 记录 WARN 级别日志
     * @param message 日志消息内容
     */
    void log_warn(const std::string& message);
    
    /**
     * @brief 记录 ERROR 级别日志
     * @param message 日志消息内容
     */
    void log_error(const std::string& message);
    
    /**
     * @brief 记录 DEBUG 级别日志
     * @param message 日志消息内容
     */
    void log_debug(const std::string& message);
    
    /**
     * @brief 设置日志级别
     * @param level 新的日志级别
     * 
     * 只有级别高于或等于设置值的日志才会被输出。
     */
    void set_level(spdlog::level::level_enum level);
    
    /**
     * @brief 获取当前日志级别
     * @return spdlog::level::level_enum 当前日志级别
     */
    spdlog::level::level_enum get_level() const;
    
    /**
     * @brief 手动刷新日志缓冲区
     * 
     * 将所有缓冲的日志消息立即写入目标 sink。
     */
    void flush();

private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    CoreLogger() = default;
    
    /**
     * @brief 私有析构函数
     */
    ~CoreLogger() = default;
    
    /**
     * @brief 禁用拷贝构造
     */
    CoreLogger(const CoreLogger&) = delete;
    
    /**
     * @brief 禁用赋值操作
     */
    CoreLogger& operator=(const CoreLogger&) = delete;
    
    std::shared_ptr<spdlog::logger> logger_;  ///< 底层 spdlog 日志器实例
    spdlog::filename_t log_file_;              ///< 日志文件完整路径
    bool initialized_ = false;                  ///< 初始化标志
};

/**
 * @brief 测试函数
 * 
 * 用于验证模块基本功能的简单测试函数。
 */
void test_function();

/**
 * @brief 获取模块 ID
 * @return int 模块唯一标识符 (1)
 * 
 * 用于多模块环境中标识日志来源。
 */
int get_module_id();

}
