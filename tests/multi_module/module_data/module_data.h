#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <memory>
#include <string>
#include <vector>
#include <chrono>

/**
 * @brief 数据处理模块
 * 
 * 该模块模拟数据处理功能，使用异步日志器进行日志记录。
 * 与核心模块和网络模块不同，数据模块使用异步日志模式，
 * 适合高吞吐量的数据处理场景。
 * 
 * 主要特性：
 * - 异步日志记录（使用线程池）
 * - 可配置的队列大小和后台线程数
 * - 数据处理事件日志（开始/结束/记录处理）
 * - 数据处理器类用于测试
 * 
 * 异步日志的优势：
 * - 主线程不阻塞，提高性能
 * - 适合大量日志输出的场景
 * - 可通过调整队列大小和线程数优化性能
 */
namespace module_data {

/**
 * @brief 数据日志器单例类
 * 
 * 封装 spdlog 异步日志器，提供数据处理模块专用的日志功能。
 * 使用 async_logger 和 thread_pool 实现异步日志记录。
 * 
 * 与同步日志器的区别：
 * - 日志消息先放入队列，由后台线程处理
 * - 主线程调用日志函数时不会阻塞在 I/O 操作上
 * - 需要配置队列大小和后台线程数
 * 
 * 日志文件名为 "data.log"。
 */
class DataLogger {
public:
    /**
     * @brief 获取单例实例
     * @return DataLogger& 单例引用
     */
    static DataLogger& instance();
    
    /**
     * @brief 初始化数据日志器
     * @param log_dir 日志文件目录路径
     * @param queue_size 异步队列大小，默认 8192
     * @param thread_count 后台线程数，默认 1
     * 
     * 初始化流程：
     * 1. 检查是否已初始化
     * 2. 拼接日志文件路径（data.log）
     * 3. 创建线程池（thread_pool）
     * 4. 创建文件 sink 和控制台 sink
     * 5. 创建异步日志器（async_logger）
     * 6. 设置日志格式、级别和自动刷新策略
     * 7. 注册到 spdlog 全局注册表
     * 
     * 队列大小说明：
     * - 队列大小决定了可以缓冲的日志消息数量
     * - 如果队列满了，新的日志消息会根据 overflow_policy 处理
     * - 默认使用 block 策略：阻塞等待队列有空间
     * 
     * 线程数说明：
     * - 单线程：简单场景，顺序处理日志
     * - 多线程：高并发场景，并行处理日志
     */
    void init(const spdlog::filename_t& log_dir, size_t queue_size = 8192, size_t thread_count = 1);
    
    /**
     * @brief 关闭日志器并释放资源
     * 
     * 清理流程：
     * 1. 刷新日志缓冲区
     * 2. 从注册表移除日志器
     * 3. 释放日志器实例
     * 4. 释放线程池（重要！异步日志器需要显式释放）
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
     * @brief 记录数据处理开始事件
     * @param dataset_name 数据集名称
     * 
     * 日志级别：INFO
     * 消息格式：[Data] Starting processing dataset: {dataset_name}
     * 
     * 在数据处理开始时调用，标记处理起点。
     */
    void log_processing_start(const std::string& dataset_name);
    
    /**
     * @brief 记录数据处理结束事件
     * @param dataset_name 数据集名称
     * @param duration 处理耗时（毫秒）
     * @param records_processed 处理的记录数
     * 
     * 日志级别：INFO
     * 消息格式：[Data] Finished processing dataset: {name} in {duration} ms, records: {count}
     * 
     * 在数据处理完成时调用，提供处理统计信息。
     */
    void log_processing_end(const std::string& dataset_name, 
                            const std::chrono::milliseconds& duration,
                            size_t records_processed);
    
    /**
     * @brief 记录单条记录处理事件
     * @param record_id 记录 ID
     * @param status 处理状态（如 "success", "warning"）
     * 
     * 日志级别：DEBUG
     * 消息格式：[Data] Record {record_id} processed: {status}
     * 
     * 注意：由于是 DEBUG 级别，默认日志级别为 DEBUG 时才会输出。
     * 适合追踪每条记录的处理状态。
     */
    void log_record_processed(size_t record_id, const std::string& status);
    
    /**
     * @brief 记录数据处理错误
     * @param error_msg 错误描述信息
     * @param record_id 相关记录 ID（可选，默认为 0）
     * 
     * 日志级别：ERROR
     * 消息格式：
     * - 有 record_id: [Data] Error processing record {record_id}: {error_msg}
     * - 无 record_id: [Data] Error: {error_msg}
     * 
     * 注意：ERROR 级别的日志会触发自动刷新。
     */
    void log_error(const std::string& error_msg, size_t record_id = 0);
    
    /**
     * @brief 记录数据处理警告
     * @param warning_msg 警告描述信息
     * 
     * 日志级别：WARN
     * 消息格式：[Data] Warning: {warning_msg}
     * 
     * 用于记录非致命但需要注意的情况，如数据异常、格式问题等。
     */
    void log_warning(const std::string& warning_msg);
    
    /**
     * @brief 设置日志级别
     * @param level 新的日志级别
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
     * 对于异步日志器，flush() 会等待队列中的所有消息处理完成。
     * 在程序退出前或需要确保日志已持久化时调用。
     */
    void flush();
    
    /**
     * @brief 获取队列溢出计数器
     * @return size_t 溢出次数
     * 
     * 当使用 overrun_oldest 策略时，队列满时会丢弃最旧的消息。
     * 此计数器记录被丢弃的消息数量。
     * 
     * 注意：当前使用的是 block 策略，此计数器通常为 0。
     */
    size_t get_overrun_counter() const;
    
    /**
     * @brief 获取消息丢弃计数器
     * @return size_t 丢弃次数
     * 
     * 当使用 discard_new 策略时，队列满时会丢弃新消息。
     * 此计数器记录被丢弃的消息数量。
     * 
     * 注意：当前使用的是 block 策略，此计数器通常为 0。
     */
    size_t get_discard_counter() const;

private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    DataLogger() = default;
    
    /**
     * @brief 私有析构函数
     */
    ~DataLogger() = default;
    
    /**
     * @brief 禁用拷贝构造
     */
    DataLogger(const DataLogger&) = delete;
    
    /**
     * @brief 禁用赋值操作
     */
    DataLogger& operator=(const DataLogger&) = delete;
    
    std::shared_ptr<spdlog::logger> logger_;              ///< 底层 spdlog 异步日志器实例
    std::shared_ptr<spdlog::details::thread_pool> thread_pool_; ///< 异步日志线程池
    spdlog::filename_t log_file_;                          ///< 日志文件完整路径
    bool initialized_ = false;                              ///< 初始化标志
};

/**
 * @brief 数据处理器类
 * 
 * 模拟数据处理操作，用于测试数据模块的异步日志功能。
 * 不进行真实的数据处理，只是模拟处理流程并记录日志。
 * 
 * 主要功能：
 * - 加载数据集
 * - 处理所有记录
 * - 处理单条记录
 * - 支持错误和警告模拟
 * - 维护处理统计信息
 */
class DataProcessor {
public:
    /**
     * @brief 构造函数
     */
    DataProcessor();
    
    /**
     * @brief 析构函数
     */
    ~DataProcessor() = default;
    
    /**
     * @brief 加载数据集
     * @param name 数据集名称
     * @param record_count 总记录数
     * 
     * 执行操作：
     * 1. 保存数据集名称和记录数
     * 2. 重置已处理记录计数
     * 3. 记录处理开始日志
     */
    void load_dataset(const std::string& name, size_t record_count);
    
    /**
     * @brief 处理所有记录
     * 
     * 执行操作：
     * 1. 记录开始时间
     * 2. 循环调用 process_record() 处理每条记录
     * 3. 计算处理耗时
     * 4. 记录处理结束日志（包含耗时和处理数量）
     */
    void process_all_records();
    
    /**
     * @brief 处理单条记录
     * @param record_id 记录 ID
     * 
     * 处理逻辑：
     * 1. 检查记录是否在错误列表中 → 记录 ERROR 日志
     * 2. 检查记录是否在警告列表中 → 记录 WARN 和 DEBUG 日志
     * 3. 正常记录 → 记录 DEBUG 日志
     * 4. 增加已处理计数（错误记录除外）
     */
    void process_record(size_t record_id);
    
    /**
     * @brief 获取总记录数
     * @return size_t 数据集总记录数
     */
    size_t get_total_records() const;
    
    /**
     * @brief 获取已处理记录数
     * @return size_t 已成功处理的记录数
     * 
     * 注意：错误记录不计入已处理计数。
     */
    size_t get_processed_records() const;
    
    /**
     * @brief 获取数据集名称
     * @return const std::string& 数据集名称引用
     */
    const std::string& get_dataset_name() const;
    
    /**
     * @brief 模拟指定记录的处理错误
     * @param record_id 记录 ID
     * 
     * 用于测试错误日志功能。
     * 被标记的记录在处理时会记录 ERROR 级别日志。
     */
    void simulate_error_on_record(size_t record_id);
    
    /**
     * @brief 模拟指定记录的处理警告
     * @param record_id 记录 ID
     * 
     * 用于测试警告日志功能。
     * 被标记的记录在处理时会记录 WARN 级别日志。
     */
    void simulate_warning_on_record(size_t record_id);

private:
    std::string dataset_name_;           ///< 数据集名称
    size_t total_records_ = 0;           ///< 总记录数
    size_t processed_records_ = 0;       ///< 已处理记录数
    std::vector<size_t> error_records_;  ///< 需要模拟错误的记录 ID 列表
    std::vector<size_t> warning_records_; ///< 需要模拟警告的记录 ID 列表
};

/**
 * @brief 测试数据处理功能
 * 
 * 用于验证数据模块基本功能的测试函数。
 * 执行流程：
 * 1. 创建数据处理器实例
 * 2. 加载测试数据集（100 条记录）
 * 3. 标记记录 42 为错误
 * 4. 标记记录 75 为警告
 * 5. 处理所有记录
 */
void test_data_processing();

/**
 * @brief 获取模块 ID
 * @return int 模块唯一标识符 (3)
 * 
 * 用于多模块环境中标识日志来源。
 * 各模块的 ID 定义：
 * - Core: 1
 * - Network: 2
 * - Data: 3
 */
int get_module_id();

}
