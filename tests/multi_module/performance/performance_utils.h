#pragma once

#include "spdlog/spdlog.h"
#include <chrono>
#include <string>
#include <functional>
#include <vector>

/**
 * @brief 性能测试工具命名空间
 * 
 * 提供性能测试所需的通用工具函数和类，包括：
 * - 基准测试框架（Benchmark 类）
 * - 基准测试结果结构体（BenchmarkResult）
 * - 日志器创建函数（同步、异步、Null、Rotating）
 * - 测试环境准备函数
 * 
 * 用于测量 spdlog 在不同配置下的性能表现。
 */
namespace performance {

/**
 * @brief 高精度时钟类型定义
 * 
 * 使用 std::chrono::high_resolution_clock 作为基准测试的时钟源，
 * 提供最高精度的时间测量。
 */
using clock = std::chrono::high_resolution_clock;

/**
 * @brief 持续时间类型定义
 * 
 * 使用 double 类型的秒数表示持续时间，
 * 便于计算和输出。
 */
using duration = std::chrono::duration<double>;

/**
 * @brief 时间点类型定义
 * 
 * 表示一个具体的时间点，用于记录开始和结束时间。
 */
using time_point = std::chrono::time_point<clock>;

/**
 * @brief 基准测试结果结构体
 * 
 * 存储单次基准测试的结果数据，包括：
 * - 测试名称
 * - 迭代次数
 * - 耗时（秒）
 * - 每秒消息数
 * - 平均延迟（纳秒）
 * - 线程数
 * 
 * 提供 print() 方法用于格式化输出结果。
 */
struct BenchmarkResult {
    std::string name;           ///< 基准测试名称
    size_t iterations;          ///< 迭代次数
    double elapsed_seconds;     ///< 耗时（秒）
    double messages_per_second; ///< 每秒处理的消息数
    double avg_latency_ns;      ///< 平均延迟（纳秒）
    size_t thread_count;        ///< 使用的线程数
    
    /**
     * @brief 打印基准测试结果
     * 
     * 以表格形式格式化输出结果，包括：
     * - 测试名称（左对齐，40 字符）
     * - 迭代次数（右对齐，12 字符）
     * - 耗时（秒，保留 3 位小数）
     * - 每秒消息数（整数）
     * - 平均延迟（纳秒，保留 2 位小数）
     * - 线程数
     */
    void print() const;
};

/**
 * @brief 基准测试类
 * 
 * 提供基准测试的核心功能，包括：
 * - 单线程基准测试
 * - 多线程基准测试
 * - 结果计算和输出
 * 
 * 使用 RAII 模式管理测试生命周期。
 */
class Benchmark {
public:
    /**
     * @brief 构造函数
     * @param name 基准测试名称
     * 
     * 初始化基准测试名称，其他参数在 run() 方法中设置。
     */
    explicit Benchmark(std::string name);
    
    /**
     * @brief 运行单线程基准测试
     * @param iterations 迭代次数
     * @param func 要测试的函数对象
     * 
     * 执行流程：
     * 1. 记录开始时间
     * 2. 循环执行 func() 指定次数
     * 3. 记录结束时间
     * 
     * 注意：func() 应该是无状态的，以便多次执行。
     */
    void run(size_t iterations, const std::function<void()>& func);
    
    /**
     * @brief 运行多线程基准测试
     * @param iterations 总迭代次数
     * @param thread_count 线程数
     * @param func 要测试的函数对象，参数为线程 ID
     * 
     * 执行流程：
     * 1. 计算每个线程的迭代次数（iterations / thread_count）
     * 2. 创建指定数量的线程
     * 3. 每个线程执行分配的迭代次数
     * 4. 等待所有线程完成
     * 5. 记录总耗时
     * 
     * 注意：
     * - 总迭代次数会被线程数整除，余数会被忽略
     * - func() 必须是线程安全的
     */
    void run_multithreaded(size_t iterations, size_t thread_count, 
                           const std::function<void(size_t)>& func);
    
    /**
     * @brief 获取基准测试结果
     * @return BenchmarkResult 基准测试结果结构体
     * 
     * 计算并返回基准测试结果，包括：
     * - 测试名称
     * - 迭代次数
     * - 耗时（秒）
     * - 每秒消息数（iterations / elapsed_seconds）
     * - 平均延迟（纳秒，elapsed_seconds * 1e9 / iterations）
     * - 线程数
     * 
     * 注意：必须在 run() 或 run_multithreaded() 之后调用。
     */
    BenchmarkResult get_result() const;
    
    /**
     * @brief 打印结果表格表头
     * 
     * 以固定格式打印表头，包括：
     * - Benchmark: 测试名称
     * - Iterations: 迭代次数
     * - Time(s): 耗时（秒）
     * - Msg/s: 每秒消息数
     * - Latency(ns): 平均延迟（纳秒）
     * - Threads: 线程数
     * 
     * 与 print_separator() 和 BenchmarkResult::print() 配合使用。
     */
    static void print_header();
    
    /**
     * @brief 打印分隔线
     * 
     * 打印 104 个 '-' 字符作为表格分隔线，
     * 用于分隔表头和数据行，或分隔不同的测试组。
     */
    static void print_separator();

private:
    std::string name_;       ///< 基准测试名称
    size_t iterations_ = 0;  ///< 迭代次数
    size_t thread_count_ = 1; ///< 线程数
    time_point start_;        ///< 开始时间点
    time_point end_;          ///< 结束时间点
};

/**
 * @brief 准备性能测试日志目录
 * @param dir_name 目录名称，默认为 "perf_logs"
 * 
 * 执行流程：
 * 1. 清除所有已注册的 spdlog 日志器（避免测试间干扰）
 * 2. 删除已存在的日志目录
 * 3. 创建新的空日志目录
 * 
 * 跨平台实现：
 * - Windows: 使用 rmdir /S /Q 和 mkdir 命令
 * - Unix/Linux/macOS: 使用 rm -rf 和 mkdir -p 命令
 * 
 * 注意：应该在每个性能测试开始时调用，确保测试环境干净。
 */
void prepare_logdir(const std::string& dir_name = "perf_logs");

/**
 * @brief 创建同步日志器
 * @param name 日志器名称
 * @param filename 日志文件路径
 * @return std::shared_ptr<spdlog::logger> 日志器共享指针
 * 
 * 创建一个同步日志器，使用 basic_file_sink_mt 作为 sink。
 * 
 * 配置：
 * - Sink: basic_file_sink_mt（线程安全，追加模式）
 * - 日志格式: "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
 * - 自动注册到 spdlog 全局注册表
 * 
 * 同步日志器的特点：
 * - 日志调用会阻塞直到写入完成
 * - 延迟较高，但可靠性好
 * - 适合日志量不大的场景
 */
std::shared_ptr<spdlog::logger> create_sync_logger(const std::string& name, 
                                                      const spdlog::filename_t& filename);

/**
 * @brief 创建异步日志器
 * @param name 日志器名称
 * @param filename 日志文件路径
 * @param queue_size 异步队列大小，默认为 8192
 * @param thread_count 后台线程数，默认为 1
 * @return std::shared_ptr<spdlog::logger> 日志器共享指针
 * 
 * 创建一个异步日志器，使用 async_logger 和 thread_pool。
 * 
 * 配置：
 * - Sink: basic_file_sink_mt（线程安全，追加模式）
 * - 线程池: 可配置队列大小和线程数
 * - 溢出策略: block（队列满时阻塞）
 * - 日志格式: "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
 * - 自动注册到 spdlog 全局注册表
 * 
 * 异步日志器的特点：
 * - 日志调用不会阻塞在 I/O 操作上
 * - 延迟较低，吞吐量高
 * - 适合高并发、高日志量的场景
 * 
 * 队列大小说明：
 * - 队列大小决定了可以缓冲的日志消息数量
 * - 如果队列满了，新的日志消息会阻塞（block 策略）
 * - 建议根据预期的日志量设置合适的队列大小
 * 
 * 线程数说明：
 * - 单线程：简单场景，顺序处理日志
 * - 多线程：高并发场景，并行处理日志
 */
std::shared_ptr<spdlog::logger> create_async_logger(const std::string& name,
                                                       const spdlog::filename_t& filename,
                                                       size_t queue_size = 8192,
                                                       size_t thread_count = 1);

/**
 * @brief 创建 Null 日志器
 * @param name 日志器名称
 * @return std::shared_ptr<spdlog::logger> 日志器共享指针
 * 
 * 创建一个 Null 日志器，使用 null_sink_mt 作为 sink。
 * 
 * 配置：
 * - Sink: null_sink_mt（线程安全，不执行任何操作）
 * - 日志格式: "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
 * - 自动注册到 spdlog 全局注册表
 * 
 * Null 日志器的特点：
 * - 所有日志操作都会被忽略
 * - 用于测量日志框架的开销（不包括 I/O 开销）
 * - 适合作为性能基准测试的基线
 * 
 * 使用场景：
 * - 测量日志调用的纯 CPU 开销
 * - 对比不同日志器实现的性能差异
 * - 验证日志框架在高并发下的线程安全性
 */
std::shared_ptr<spdlog::logger> create_null_logger(const std::string& name);

/**
 * @brief 创建轮转日志器
 * @param name 日志器名称
 * @param filename 日志文件路径
 * @param max_size 单个日志文件最大大小，默认为 10MB
 * @param max_files 最大文件数，默认为 5
 * @return std::shared_ptr<spdlog::logger> 日志器共享指针
 * 
 * 创建一个轮转日志器，使用 rotating_file_sink_mt 作为 sink。
 * 
 * 配置：
 * - Sink: rotating_file_sink_mt（线程安全，轮转模式）
 * - 轮转策略: 当文件大小超过 max_size 时创建新文件
 * - 保留文件数: 最多保留 max_files 个文件
 * - 日志格式: "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
 * - 自动注册到 spdlog 全局注册表
 * 
 * 轮转日志器的特点：
 * - 自动管理日志文件大小和数量
 * - 防止单个日志文件过大
 * - 适合长期运行的服务
 * 
 * 轮转策略说明：
 * - 当当前日志文件大小超过 max_size 时：
 *   1. 关闭当前文件
 *   2. 重命名为 filename.1, filename.2 等
 *   3. 创建新的日志文件
 *   4. 如果文件数超过 max_files，删除最旧的文件
 * 
 * 默认配置：
 * - max_size: 10 * 1024 * 1024 (10MB)
 * - max_files: 5
 */
std::shared_ptr<spdlog::logger> create_rotating_logger(const std::string& name,
                                                          const spdlog::filename_t& filename,
                                                          size_t max_size = 10 * 1024 * 1024,
                                                          size_t max_files = 5);

/**
 * @brief 关闭日志器
 * @param name 日志器名称
 * 
 * 执行流程：
 * 1. 从 spdlog 注册表获取日志器
 * 2. 如果存在，刷新所有缓冲的日志消息
 * 3. 从注册表中移除日志器
 * 
 * 注意：
 * - 对于异步日志器，flush() 会等待队列中的所有消息处理完成
 * - 应该在性能测试结束时调用，确保所有日志都已写入
 */
void shutdown_logger(const std::string& name);

}
