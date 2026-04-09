#include "performance_utils.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/async.h"
#include <iostream>
#include <iomanip>
#include <thread>

// 跨平台头文件包含
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#endif

namespace performance {

/**
 * @brief 打印基准测试结果
 * 
 * 以表格形式格式化输出结果，使用 std::setw 和 std::setprecision 控制格式。
 * 
 * 输出格式：
 * - 测试名称：左对齐，40 字符宽度
 * - 迭代次数：右对齐，12 字符宽度
 * - 耗时（秒）：右对齐，12 字符宽度，保留 3 位小数
 * - 每秒消息数：右对齐，15 字符宽度，整数
 * - 平均延迟（纳秒）：右对齐，15 字符宽度，保留 2 位小数
 * - 线程数：右对齐，10 字符宽度
 * 
 * 使用 std::fixed 确保浮点数以固定小数格式输出。
 */
void BenchmarkResult::print() const {
    std::cout << std::left << std::setw(40) << name
              << std::right << std::setw(12) << iterations
              << std::setw(12) << std::fixed << std::setprecision(3) << elapsed_seconds
              << std::setw(15) << std::fixed << std::setprecision(0) << messages_per_second
              << std::setw(15) << std::fixed << std::setprecision(2) << avg_latency_ns
              << std::setw(10) << thread_count
              << std::endl;
}

/**
 * @brief 构造函数
 * @param name 基准测试名称
 * 
 * 使用 std::move 转移字符串所有权，避免不必要的拷贝。
 * 其他成员变量使用默认初始化值。
 */
Benchmark::Benchmark(std::string name) : name_(std::move(name)) {}

/**
 * @brief 运行单线程基准测试
 * @param iterations 迭代次数
 * @param func 要测试的函数对象
 * 
 * 执行流程：
 * 1. 保存迭代次数和线程数（单线程为 1）
 * 2. 记录开始时间点
 * 3. 循环执行 func() 指定次数
 * 4. 记录结束时间点
 * 
 * 注意事项：
 * - func() 应该是无状态的，以便多次执行
 * - 时间测量包括函数调用的开销
 * - 不包括任何预热阶段（假设 JIT 编译或缓存预热已完成）
 * 
 * 时间测量精度：
 * - 使用 high_resolution_clock，精度通常为纳秒级
 * - 开始和结束时间点都在循环外部记录，减少测量误差
 */
void Benchmark::run(size_t iterations, const std::function<void()>& func) {
    iterations_ = iterations;
    thread_count_ = 1;
    
    // 记录开始时间
    start_ = clock::now();
    
    // 循环执行测试函数
    for (size_t i = 0; i < iterations; ++i) {
        func();
    }
    
    // 记录结束时间
    end_ = clock::now();
}

/**
 * @brief 运行多线程基准测试
 * @param iterations 总迭代次数
 * @param thread_count 线程数
 * @param func 要测试的函数对象，参数为线程 ID
 * 
 * 执行流程：
 * 1. 保存总迭代次数和线程数
 * 2. 计算每个线程的迭代次数（iterations / thread_count）
 * 3. 预分配线程向量空间（避免重新分配）
 * 4. 记录开始时间点
 * 5. 创建指定数量的线程，每个线程执行分配的迭代次数
 * 6. 等待所有线程完成（join）
 * 7. 记录结束时间点
 * 
 * 注意事项：
 * - 总迭代次数会被线程数整除，余数会被忽略
 * - func() 必须是线程安全的
 * - 时间测量包括线程创建和销毁的开销
 * - 每个线程接收自己的线程 ID（从 0 到 thread_count-1）
 * 
 * 线程安全考虑：
 * - func() 可能被多个线程同时调用
 * - 如果 func() 访问共享资源，需要适当的同步
 * - 测试日志器时，日志器本身应该是线程安全的（spdlog 的 _mt 后缀 sink 是线程安全的）
 */
void Benchmark::run_multithreaded(size_t iterations, size_t thread_count,
                                   const std::function<void(size_t)>& func) {
    iterations_ = iterations;
    thread_count_ = thread_count;
    
    // 创建线程向量，预分配空间以避免重新分配
    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    
    // 计算每个线程的迭代次数
    // 注意：余数会被忽略，总迭代次数可能略小于指定值
    size_t iterations_per_thread = iterations / thread_count;
    
    // 记录开始时间
    start_ = clock::now();
    
    // 创建并启动线程
    for (size_t t = 0; t < thread_count; ++t) {
        // 使用 lambda 表达式作为线程函数
        // 捕获列表：
        // - [&]: 引用捕获外部变量
        // - [t]: 值捕获线程 ID
        // - [iterations_per_thread]: 值捕获每个线程的迭代次数
        threads.emplace_back([&, t, iterations_per_thread]() {
            for (size_t i = 0; i < iterations_per_thread; ++i) {
                func(t);  // 传递线程 ID 给测试函数
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 记录结束时间
    end_ = clock::now();
}

/**
 * @brief 获取基准测试结果
 * @return BenchmarkResult 基准测试结果结构体
 * 
 * 计算并返回基准测试结果，包括：
 * - 测试名称：直接使用 name_
 * - 迭代次数：直接使用 iterations_
 * - 耗时（秒）：end_ - start_，转换为 double 类型的秒数
 * - 每秒消息数：iterations_ / elapsed_seconds
 * - 平均延迟（纳秒）：(elapsed_seconds * 1e9) / iterations_
 * - 线程数：直接使用 thread_count_
 * 
 * 计算公式说明：
 * - 每秒消息数 = 总迭代次数 / 总耗时（秒）
 * - 平均延迟 = 总耗时（纳秒） / 总迭代次数
 * 
 * 注意事项：
 * - 必须在 run() 或 run_multithreaded() 之后调用
 * - 假设 start_ 和 end_ 已正确设置
 * - 对于多线程测试，每秒消息数是所有线程的总吞吐量
 */
BenchmarkResult Benchmark::get_result() const {
    // 计算耗时（秒）
    // duration 是 double 类型的秒数
    duration elapsed = end_ - start_;
    double elapsed_sec = elapsed.count();
    
    // 构造并返回结果结构体
    return {
        name_,
        iterations_,
        elapsed_sec,
        static_cast<double>(iterations_) / elapsed_sec,  // 每秒消息数
        (elapsed_sec * 1e9) / static_cast<double>(iterations_),  // 平均延迟（纳秒）
        thread_count_
    };
}

/**
 * @brief 打印结果表格表头
 * 
 * 以固定格式打印表头，与 BenchmarkResult::print() 的格式对应。
 * 
 * 输出格式：
 * - Benchmark: 左对齐，40 字符宽度
 * - Iterations: 右对齐，12 字符宽度
 * - Time(s): 右对齐，12 字符宽度
 * - Msg/s: 右对齐，15 字符宽度
 * - Latency(ns): 右对齐，15 字符宽度
 * - Threads: 右对齐，10 字符宽度
 * 
 * 使用场景：
 * - 在打印多个测试结果之前调用
 * - 与 print_separator() 和 BenchmarkResult::print() 配合使用
 * 
 * 典型用法：
 * @code
 * Benchmark::print_header();
 * Benchmark::print_separator();
 * result1.print();
 * result2.print();
 * Benchmark::print_separator();
 * @endcode
 */
void Benchmark::print_header() {
    std::cout << std::left << std::setw(40) << "Benchmark"
              << std::right << std::setw(12) << "Iterations"
              << std::setw(12) << "Time(s)"
              << std::setw(15) << "Msg/s"
              << std::setw(15) << "Latency(ns)"
              << std::setw(10) << "Threads"
              << std::endl;
}

/**
 * @brief 打印分隔线
 * 
 * 打印 104 个 '-' 字符作为表格分隔线。
 * 
 * 长度计算：
 * - 40 (Benchmark) + 12 (Iterations) + 12 (Time) + 15 (Msg/s) + 15 (Latency) + 10 (Threads) = 104
 * 
 * 使用场景：
 * - 在表头和数据行之间打印
 * - 在不同测试组之间打印
 * - 在所有测试结果之后打印
 */
void Benchmark::print_separator() {
    std::cout << std::string(104, '-') << std::endl;
}

/**
 * @brief 准备性能测试日志目录
 * @param dir_name 目录名称
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
 * Windows 平台说明：
 * - rmdir /S /Q: 递归删除目录树，安静模式（不询问确认）
 * - 2>nul: 重定向错误输出到 nul，避免目录不存在时的错误提示
 * - mkdir: 创建目录
 * 
 * Unix/Linux/macOS 平台说明：
 * - rm -rf: 强制递归删除，忽略不存在的文件
 * - mkdir -p: 创建目录，包括所有不存在的父目录
 * - 如果目录已存在，mkdir -p 不会报错
 * 
 * 注意事项：
 * - 应该在每个性能测试开始时调用，确保测试环境干净
 * - 使用 system() 调用 shell 命令，可能存在安全风险
 * - 对于生产环境，建议使用更安全的 API（如 std::filesystem）
 */
void prepare_logdir(const std::string& dir_name) {
    // 清除所有已注册的日志器，避免测试间干扰
    spdlog::drop_all();
    
#ifdef _WIN32
    // Windows 平台：使用 cmd 命令
    // rmdir /S /Q: 递归删除目录树，安静模式
    // 2>nul: 抑制错误输出（目录不存在时）
    std::string cmd = "rmdir /S /Q " + dir_name + " 2>nul";
    system(cmd.c_str());
    
    // 创建新目录
    cmd = "mkdir " + dir_name;
    system(cmd.c_str());
#else
    // Unix/Linux/macOS 平台：使用 shell 命令
    // rm -rf: 强制递归删除
    std::string cmd = "rm -rf " + dir_name;
    auto rv = system(cmd.c_str());
    (void)rv;  // 显式忽略返回值，避免编译器警告
    
    // mkdir -p: 创建目录，包括父目录
    // 如果目录已存在，不会报错
    cmd = "mkdir -p " + dir_name;
    rv = system(cmd.c_str());
    if (rv != 0) {
        throw std::runtime_error("Failed to create directory: " + dir_name);
    }
#endif
}

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
 *   - _mt 后缀表示多线程安全
 *   - 第二个参数 true 表示追加模式（不截断现有文件）
 * - 日志格式: "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
 *   - %Y-%m-%d %H:%M:%S.%e: 时间戳（年-月-日 时:分:秒.毫秒）
 *   - %l: 日志级别
 *   - %v: 实际日志消息
 * - 自动注册到 spdlog 全局注册表
 * 
 * 同步日志器的特点：
 * - 日志调用会阻塞直到写入完成
 * - 延迟较高，但可靠性好
 * - 适合日志量不大的场景
 * - 线程安全（使用 _mt 后缀的 sink）
 * 
 * 与异步日志器的对比：
 * - 同步：每次日志调用都执行 I/O 操作
 * - 异步：日志调用先放入队列，由后台线程执行 I/O
 */
std::shared_ptr<spdlog::logger> create_sync_logger(const std::string& name,
                                                      const spdlog::filename_t& filename) {
    // 创建文件 sink：追加模式（true），线程安全版本（_mt 后缀）
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
    
    // 创建日志器
    auto logger = std::make_shared<spdlog::logger>(name, sink);
    
    // 设置日志格式
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    
    // 注册到全局注册表，可通过 spdlog::get(name) 获取
    spdlog::register_logger(logger);
    
    return logger;
}

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
 * - 线程池: 可配置队列大小和线程数
 *   - queue_size: 队列中可以缓冲的最大消息数
 *   - thread_count: 处理队列消息的后台线程数
 * - Sink: basic_file_sink_mt（线程安全，追加模式）
 * - 溢出策略: block（队列满时阻塞）
 *   - block: 队列满时阻塞等待
 *   - overrun_oldest: 队列满时丢弃最旧的消息
 *   - discard_new: 队列满时丢弃新消息
 * - 日志格式: "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
 * - 自动注册到 spdlog 全局注册表
 * 
 * 异步日志器的特点：
 * - 日志调用不会阻塞在 I/O 操作上
 * - 延迟较低，吞吐量高
 * - 适合高并发、高日志量的场景
 * - 需要正确配置队列大小和线程数
 * 
 * 队列大小说明：
 * - 队列大小决定了可以缓冲的日志消息数量
 * - 如果队列满了，新的日志消息会根据 overflow_policy 处理
 * - 建议根据预期的日志量设置合适的队列大小
 * - 队列太小可能导致阻塞或消息丢失
 * - 队列太大可能浪费内存
 * 
 * 线程数说明：
 * - 单线程：简单场景，顺序处理日志
 * - 多线程：高并发场景，并行处理日志
 * - 多线程可能引入额外的同步开销
 * - 建议根据实际性能测试选择最优线程数
 */
std::shared_ptr<spdlog::logger> create_async_logger(const std::string& name,
                                                       const spdlog::filename_t& filename,
                                                       size_t queue_size,
                                                       size_t thread_count) {
    // 创建线程池：队列大小、后台线程数
    auto tp = std::make_shared<spdlog::details::thread_pool>(queue_size, thread_count);
    
    // 创建文件 sink：追加模式（true），线程安全版本
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
    
    // 创建异步日志器
    // 参数：日志器名称、sink、线程池、溢出策略
    auto logger = std::make_shared<spdlog::async_logger>(
        name, sink, tp, spdlog::async_overflow_policy::block);
    
    // 设置日志格式
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    
    // 注册到全局注册表
    spdlog::register_logger(logger);
    
    return logger;
}

/**
 * @brief 创建 Null 日志器
 * @param name 日志器名称
 * @return std::shared_ptr<spdlog::logger> 日志器共享指针
 * 
 * 创建一个 Null 日志器，使用 null_sink_mt 作为 sink。
 * 
 * 配置：
 * - Sink: null_sink_mt（线程安全，不执行任何操作）
 *   - 所有写入操作都会被忽略
 *   - 线程安全版本
 * - 日志格式: "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
 * - 自动注册到 spdlog 全局注册表
 * 
 * Null 日志器的特点：
 * - 所有日志操作都会被忽略
 * - 用于测量日志框架的开销（不包括 I/O 开销）
 * - 适合作为性能基准测试的基线
 * - 线程安全
 * 
 * 使用场景：
 * - 测量日志调用的纯 CPU 开销
 * - 对比不同日志器实现的性能差异
 * - 验证日志框架在高并发下的线程安全性
 * - 作为性能测试的对照组
 * 
 * 性能测试中的作用：
 * - Null 日志器的性能代表了日志框架的最小开销
 * - 其他日志器的性能与 Null 日志器的差异代表了实际 I/O 开销
 * - 可以用来计算日志框架的"纯开销"
 */
std::shared_ptr<spdlog::logger> create_null_logger(const std::string& name) {
    // 创建 Null sink：所有操作都被忽略
    auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    
    // 创建日志器
    auto logger = std::make_shared<spdlog::logger>(name, sink);
    
    // 设置日志格式（虽然不会实际输出，但格式处理仍会执行）
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    
    // 注册到全局注册表
    spdlog::register_logger(logger);
    
    return logger;
}

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
 *   - 轮转策略: 当文件大小超过 max_size 时创建新文件
 *   - 保留文件数: 最多保留 max_files 个文件
 * - 日志格式: "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
 * - 自动注册到 spdlog 全局注册表
 * 
 * 轮转日志器的特点：
 * - 自动管理日志文件大小和数量
 * - 防止单个日志文件过大
 * - 适合长期运行的服务
 * - 线程安全
 * 
 * 轮转策略说明：
 * - 当当前日志文件大小超过 max_size 时：
 *   1. 关闭当前文件
 *   2. 重命名为 filename.1, filename.2 等
 *   3. 创建新的日志文件
 *   4. 如果文件数超过 max_files，删除最旧的文件
 * 
 * 文件命名规则：
 * - 当前日志文件: filename
 * - 归档日志文件: filename.1, filename.2, ..., filename.N
 * - 数字越小表示文件越新
 * 
 * 默认配置：
 * - max_size: 10 * 1024 * 1024 (10MB)
 * - max_files: 5
 * 
 * 使用建议：
 * - 根据磁盘空间和日志保留需求设置 max_size 和 max_files
 * - 对于高日志量场景，可以增大 max_size 或 max_files
 * - 对于需要长期保留日志的场景，可以增大 max_files
 */
std::shared_ptr<spdlog::logger> create_rotating_logger(const std::string& name,
                                                          const spdlog::filename_t& filename,
                                                          size_t max_size,
                                                          size_t max_files) {
    // 创建轮转文件 sink：文件名、最大大小、最大文件数
    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        filename, max_size, max_files);
    
    // 创建日志器
    auto logger = std::make_shared<spdlog::logger>(name, sink);
    
    // 设置日志格式
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    
    // 注册到全局注册表
    spdlog::register_logger(logger);
    
    return logger;
}

/**
 * @brief 关闭日志器
 * @param name 日志器名称
 * 
 * 执行流程：
 * 1. 从 spdlog 注册表获取日志器
 * 2. 如果存在，刷新所有缓冲的日志消息
 * 3. 从注册表中移除日志器
 * 
 * 注意事项：
 * - 对于异步日志器，flush() 会等待队列中的所有消息处理完成
 * - 应该在性能测试结束时调用，确保所有日志都已写入
 * - 即使日志器不存在，spdlog::drop() 也不会报错
 * 
 * 与 spdlog::drop() 的区别：
 * - shutdown_logger() 会先刷新日志器
 * - spdlog::drop() 只是从注册表中移除，不刷新
 * - 对于同步日志器，两者差异不大
 * - 对于异步日志器，shutdown_logger() 确保所有消息都被处理
 * 
 * 典型用法：
 * @code
 * auto logger = create_sync_logger("test", "test.log");
 * // ... 执行测试 ...
 * shutdown_logger("test");  // 确保所有日志都已写入
 * @endcode
 */
void shutdown_logger(const std::string& name) {
    // 从注册表获取日志器
    auto logger = spdlog::get(name);
    
    // 如果存在，刷新所有缓冲的日志消息
    if (logger) {
        logger->flush();
    }
    
    // 从注册表中移除日志器
    spdlog::drop(name);
}

}
