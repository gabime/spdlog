#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <memory>
#include <string>
#include <vector>

/**
 * @brief 网络模块
 * 
 * 该模块模拟网络通信功能，提供网络操作相关的日志记录能力。
 * 用于测试多模块环境下的日志隔离和集成场景。
 * 
 * 主要特性：
 * - 独立的网络日志器（默认 INFO 级别）
 * - 网络连接/断开事件日志
 * - 数据收发统计日志
 * - 网络模拟器类用于测试
 */
namespace module_network {

/**
 * @brief 网络日志器单例类
 * 
 * 封装 spdlog 日志器，提供网络模块专用的日志功能。
 * 与核心模块不同，网络模块默认日志级别为 INFO，
 * 且只在 ERROR 级别时自动刷新。
 * 
 * 日志文件名为 "network.log"。
 */
class NetworkLogger {
public:
    /**
     * @brief 获取单例实例
     * @return NetworkLogger& 单例引用
     */
    static NetworkLogger& instance();
    
    /**
     * @brief 初始化网络日志器
     * @param log_dir 日志文件目录路径
     * 
     * 初始化流程：
     * 1. 检查是否已初始化
     * 2. 拼接日志文件路径（network.log）
     * 3. 创建文件 sink 和控制台 sink
     * 4. 设置日志格式和级别（默认 INFO）
     * 5. 设置自动刷新级别（ERROR）
     * 6. 注册到 spdlog 全局注册表
     * 
     * 注意：网络模块默认日志级别为 INFO，DEBUG 级别的日志
     *（如数据收发详情）默认不会输出。
     */
    void init(const spdlog::filename_t& log_dir);
    
    /**
     * @brief 关闭日志器并释放资源
     * 
     * 刷新缓冲区，从注册表移除，释放日志器实例。
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
     * @brief 记录网络连接事件
     * @param host 目标主机地址
     * @param port 目标端口号
     * 
     * 日志级别：INFO
     * 消息格式：[Network] Connected to {host}:{port}
     */
    void log_connection(const std::string& host, int port);
    
    /**
     * @brief 记录网络断开事件
     * @param host 目标主机地址
     * @param port 目标端口号
     * 
     * 日志级别：INFO
     * 消息格式：[Network] Disconnected from {host}:{port}
     */
    void log_disconnection(const std::string& host, int port);
    
    /**
     * @brief 记录数据发送事件
     * @param host 目标主机地址
     * @param bytes 发送的字节数
     * 
     * 日志级别：DEBUG
     * 消息格式：[Network] Sent {bytes} bytes to {host}
     * 
     * 注意：由于默认日志级别为 INFO，此日志默认不会输出。
     * 需要显式设置日志级别为 DEBUG 才能看到。
     */
    void log_data_sent(const std::string& host, size_t bytes);
    
    /**
     * @brief 记录数据接收事件
     * @param host 源主机地址
     * @param bytes 接收的字节数
     * 
     * 日志级别：DEBUG
     * 消息格式：[Network] Received {bytes} bytes from {host}
     */
    void log_data_received(const std::string& host, size_t bytes);
    
    /**
     * @brief 记录网络错误事件
     * @param error_msg 错误描述信息
     * 
     * 日志级别：ERROR
     * 消息格式：[Network] Error: {error_msg}
     * 
     * 注意：ERROR 级别的日志会触发自动刷新。
     */
    void log_error(const std::string& error_msg);
    
    /**
     * @brief 设置日志级别
     * @param level 新的日志级别
     * 
     * 可用于动态调整日志详细程度，例如：
     * - 生产环境：INFO 或 WARN
     * - 调试环境：DEBUG 或 TRACE
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
    NetworkLogger() = default;
    
    /**
     * @brief 私有析构函数
     */
    ~NetworkLogger() = default;
    
    /**
     * @brief 禁用拷贝构造
     */
    NetworkLogger(const NetworkLogger&) = delete;
    
    /**
     * @brief 禁用赋值操作
     */
    NetworkLogger& operator=(const NetworkLogger&) = delete;
    
    std::shared_ptr<spdlog::logger> logger_;  ///< 底层 spdlog 日志器实例
    spdlog::filename_t log_file_;              ///< 日志文件完整路径
    bool initialized_ = false;                  ///< 初始化标志
};

/**
 * @brief 网络模拟器类
 * 
 * 模拟网络通信操作，用于测试网络模块的日志功能。
 * 不进行真实的网络连接，只是记录操作日志。
 * 
 * 主要功能：
 * - 模拟连接/断开操作
 * - 模拟数据发送/接收
 * - 维护连接状态列表
 * - 支持错误模拟
 */
class NetworkSimulator {
public:
    /**
     * @brief 构造函数
     */
    NetworkSimulator();
    
    /**
     * @brief 析构函数
     */
    ~NetworkSimulator() = default;
    
    /**
     * @brief 模拟连接到远程主机
     * @param host 目标主机地址
     * @param port 目标端口号
     * 
     * 执行操作：
     * 1. 记录连接事件日志
     * 2. 将连接信息添加到连接列表
     */
    void connect(const std::string& host, int port);
    
    /**
     * @brief 模拟断开连接
     * @param host 目标主机地址
     * @param port 目标端口号
     * 
     * 执行操作：
     * 1. 记录断开事件日志
     * 2. 从连接列表中移除指定连接
     */
    void disconnect(const std::string& host, int port);
    
    /**
     * @brief 模拟发送数据
     * @param host 目标主机地址
     * @param data 要发送的数据内容
     * 
     * 执行操作：
     * 1. 记录数据发送日志（DEBUG 级别）
     * 2. 不实际发送数据
     */
    void send_data(const std::string& host, const std::string& data);
    
    /**
     * @brief 模拟接收数据
     * @param host 源主机地址
     * @param max_bytes 最大接收字节数
     * @return std::string 接收到的数据（随机生成的测试数据）
     * 
     * 执行操作：
     * 1. 生成 1 到 max_bytes 之间的随机字节数
     * 2. 记录数据接收日志（DEBUG 级别）
     * 3. 返回由 'X' 字符组成的测试数据
     */
    std::string receive_data(const std::string& host, size_t max_bytes);
    
    /**
     * @brief 获取当前连接数量
     * @return size_t 活跃连接数
     */
    size_t get_connections_count() const;
    
    /**
     * @brief 模拟网络错误
     * @param error_msg 错误描述信息
     * 
     * 用于测试错误日志功能。
     * 记录 ERROR 级别的日志，会触发自动刷新。
     */
    void simulate_error(const std::string& error_msg);

private:
    /**
     * @brief 连接列表
     * 
     * 存储当前活跃的网络连接信息。
     * 每个元素是一个 (host, port) 对。
     */
    std::vector<std::pair<std::string, int>> connections_;
};

/**
 * @brief 测试网络操作
 * 
 * 用于验证网络模块基本功能的测试函数。
 * 执行一系列模拟网络操作：
 * 1. 连接到 192.168.1.1:8080
 * 2. 发送数据 "Hello, World!"
 * 3. 接收最多 1024 字节数据
 * 4. 断开连接
 */
void test_network_operations();

/**
 * @brief 获取模块 ID
 * @return int 模块唯一标识符 (2)
 * 
 * 用于多模块环境中标识日志来源。
 * 各模块的 ID 定义：
 * - Core: 1
 * - Network: 2
 * - Data: 3
 */
int get_module_id();

}
