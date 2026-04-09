#include "performance_utils.h"
#include <catch2/catch_all.hpp>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

namespace {
const std::string PERF_LOG_DIR = "perf_logs";                // 性能测试日志目录
const size_t TOTAL_ITERATIONS = 100000;                      // 总迭代次数
const std::vector<size_t> THREAD_COUNTS = {1, 2, 4, 8, 16};  // 测试的线程数配置
}

/**
 * @brief 测试用例：多线程同步日志器性能
 * @tags [performance][multithread]
 * 
 * 测试目的：测量同步日志器在多线程并发场景下的性能表现。
 * 
 * 测试场景：
 * - 1 个线程
 * - 2 个线程
 * - 4 个线程
 * - 8 个线程
 * - 16 个线程
 * 
 * 同步日志器的多线程特点：
 * - 所有线程共享同一个日志器
 * - 日志器内部使用互斥锁保证线程安全
 * - 高并发场景下可能存在锁竞争
 * - 性能随线程数增加可能下降（锁竞争加剧）
 * 
 * 性能指标：
 * - 每秒消息数（Msg/s）
 * - 平均延迟（Latency, ns）
 * 
 * 测试配置：
 * - 日志器类型：同步
 * - Sink: basic_file_sink_mt（线程安全版本）
 * - 消息格式：包含线程 ID 和消息计数器
 */
TEST_CASE("Multithread sync logger performance", "[performance][multithread]") {
    // 准备测试环境：清除旧日志，创建新目录
    performance::prepare_logdir(PERF_LOG_DIR);
    
    // 打印性能测试结果表头
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    // 遍历不同的线程数配置
    for (size_t thread_count : THREAD_COUNTS) {
        // 创建同步日志器，每个线程数配置使用独立的日志文件
        auto logger = performance::create_sync_logger(
            "mt_sync_" + std::to_string(thread_count),
            PERF_LOG_DIR + "/mt_sync_" + std::to_string(thread_count) + ".log");
        
        // 使用原子计数器生成唯一的消息编号
        // 确保每条日志消息都有唯一的标识符
        std::atomic<size_t> counter{0};
        
        // 创建基准测试实例，名称包含线程数
        performance::Benchmark bench(
            "Multithread Sync - " + std::to_string(thread_count) + " threads");
        
        // 运行多线程基准测试
        // 每个线程执行 TOTAL_ITERATIONS / thread_count 次迭代
        bench.run_multithreaded(TOTAL_ITERATIONS, thread_count,
            [&](size_t thread_id) {
                // 原子递增计数器，获取唯一的消息编号
                size_t local_counter = counter++;
                // 记录日志，包含线程 ID 和消息编号
                logger->info("Thread {}: Message {}", thread_id, local_counter);
            });
        
        // 刷新日志器，确保所有消息都写入文件
        logger->flush();
        
        // 获取并打印测试结果
        auto result = bench.get_result();
        result.print();
        
        // 验证：每秒消息数应该大于 0
        REQUIRE(result.messages_per_second > 0);
        
        // 关闭日志器，释放资源
        performance::shutdown_logger("mt_sync_" + std::to_string(thread_count));
    }
    
    // 打印分隔线
    performance::Benchmark::print_separator();
}

/**
 * @brief 测试用例：多线程异步日志器性能
 * @tags [performance][multithread]
 * 
 * 测试目的：测量异步日志器在多线程并发场景下的性能表现。
 * 
 * 测试场景：
 * - 1 个线程
 * - 2 个线程
 * - 4 个线程
 * - 8 个线程
 * - 16 个线程
 * 
 * 异步日志器的多线程特点：
 * - 调用线程只将消息放入队列，不执行 I/O
 * - 后台线程从队列取出消息并执行 I/O
 * - 高并发场景下性能通常优于同步日志器
 * - 队列大小需要足够大以避免阻塞
 * 
 * 性能指标：
 * - 每秒消息数（Msg/s）
 * - 平均延迟（Latency, ns）
 * 
 * 测试配置：
 * - 日志器类型：异步
 * - 队列大小：65536（较大队列，减少阻塞概率）
 * - 后台线程数：2（并行处理日志）
 * - Sink: basic_file_sink_mt
 */
TEST_CASE("Multithread async logger performance", "[performance][multithread]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    for (size_t thread_count : THREAD_COUNTS) {
        // 创建异步日志器
        // 队列大小：65536（较大队列，适应高并发）
        // 后台线程数：2（并行处理）
        auto logger = performance::create_async_logger(
            "mt_async_" + std::to_string(thread_count),
            PERF_LOG_DIR + "/mt_async_" + std::to_string(thread_count) + ".log",
            65536, 2);
        
        std::atomic<size_t> counter{0};
        
        performance::Benchmark bench(
            "Multithread Async - " + std::to_string(thread_count) + " threads");
        
        // 异步日志器更快，所以迭代次数加倍
        bench.run_multithreaded(TOTAL_ITERATIONS * 2, thread_count,
            [&](size_t thread_id) {
                size_t local_counter = counter++;
                logger->info("Thread {}: Message {}", thread_id, local_counter);
            });
        
        // 异步日志器需要 flush() 确保队列中的消息都被处理
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("mt_async_" + std::to_string(thread_count));
    }
    
    performance::Benchmark::print_separator();
}

/**
 * @brief 测试用例：多线程 Null sink 性能（锁竞争测试）
 * @tags [performance][multithread]
 * 
 * 测试目的：测量日志框架在多线程场景下的纯 CPU 开销（不包括 I/O）。
 * 
 * 测试场景：
 * - 单线程（基线）
 * - 1 个线程
 * - 2 个线程
 * - 4 个线程
 * - 8 个线程
 * - 16 个线程
 * 
 * Null sink 的特点：
 * - 所有写入操作都会被忽略
 * - 不执行任何 I/O 操作
 * - 但仍然会执行格式处理和函数调用
 * - 可以用来测量锁竞争的开销
 * 
 * 性能分析：
 * - 单线程：测量最小开销
 * - 多线程：测量锁竞争带来的额外开销
 * - 性能下降程度反映了锁竞争的严重程度
 * 
 * 实际应用意义：
 * - 帮助理解日志框架的线程安全实现效率
 * - 可以用来对比不同日志框架的锁竞争特性
 * - 为实际应用中的线程数配置提供参考
 */
TEST_CASE("Multithread null sink performance (contention test)", "[performance][multithread]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    /**
     * @brief 测试场景：单线程 Null sink
     * 
     * 测试目的：建立单线程性能基线
     * 
     * 迭代次数：1,000,000 次（10 倍于多线程测试）
     * 
     * 性能特点：
     * - 没有锁竞争
     * - 测量纯函数调用和格式处理开销
     * - 作为多线程测试的对比基准
     */
    SECTION("Single-threaded null sink") {
        auto logger = performance::create_null_logger("null_st");
        
        performance::Benchmark bench("Null Sink - Single thread");
        // 单线程测试使用更多迭代次数，获得更稳定的结果
        bench.run(TOTAL_ITERATIONS * 10, [&]() {
            logger->info("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("null_st");
    }
    
    // 遍历不同的线程数配置
    for (size_t thread_count : THREAD_COUNTS) {
        auto logger = performance::create_null_logger(
            "null_mt_" + std::to_string(thread_count));
        
        std::atomic<size_t> counter{0};
        
        performance::Benchmark bench(
            "Null Sink - " + std::to_string(thread_count) + " threads");
        
        // 多线程测试使用 5 倍迭代次数
        bench.run_multithreaded(TOTAL_ITERATIONS * 5, thread_count,
            [&](size_t thread_id) {
                size_t local_counter = counter++;
                logger->info("Thread {}: Message {}", thread_id, local_counter);
            });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("null_mt_" + std::to_string(thread_count));
    }
    
    performance::Benchmark::print_separator();
}

/**
 * @brief 测试用例：高并发场景下的多线程日志器性能
 * @tags [performance][multithread]
 * 
 * 测试目的：测量日志器在极端高并发场景下的性能表现。
 * 
 * 测试场景：
 * - 32 个并发线程
 * - 每个线程执行 1000 次迭代
 * - 总迭代次数：32,000 次
 * 
 * 高并发场景的特点：
 * - 锁竞争非常激烈
 * - 上下文切换开销增加
 * - 同步日志器性能可能显著下降
 * - 异步日志器可能表现更好
 * 
 * 测试配置：
 * - 同步日志器：basic_file_sink_mt
 * - 异步日志器：队列大小 65536，后台线程 4 个
 * 
 * 性能对比：
 * - 同步 vs 异步在高并发下的性能差异
 * - 锁竞争对吞吐量的影响
 * - 异步队列的缓冲效果
 * 
 * 实际应用意义：
 * - 帮助选择适合高并发场景的日志方案
 * - 为生产环境的日志配置提供参考
 * - 验证异步日志在高并发下的优势
 */
TEST_CASE("Multithread logger with high contention", "[performance][multithread]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    // 高并发配置：32 个线程
    const size_t HIGH_THREAD_COUNT = 32;
    // 每个线程执行 1000 次迭代
    const size_t ITERATIONS_PER_THREAD = 1000;
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    /**
     * @brief 测试场景：同步日志器 - 高并发
     * 
     * 测试目的：测量同步日志器在 32 线程高并发下的性能
     * 
     * 预期结果：
     * - 由于锁竞争，性能可能显著下降
     * - 平均延迟可能大幅增加
     * - 吞吐量可能低于低并发场景
     * 
     * 实现细节：
     * - 手动管理线程生命周期
     * - 使用 atomic 计数器跟踪完成的线程数
     * - 手动计算性能指标
     */
    SECTION("Sync logger - high contention") {
        auto logger = performance::create_sync_logger(
            "high_cont_sync", PERF_LOG_DIR + "/high_cont_sync.log");
        
        // 原子计数器：跟踪完成的线程数
        std::atomic<size_t> completed{0};
        // 线程向量：存储所有线程对象
        std::vector<std::thread> threads;
        // 预分配空间，避免重新分配
        threads.reserve(HIGH_THREAD_COUNT);
        
        // 记录开始时间
        auto start = performance::clock::now();
        
        // 创建并启动 32 个线程
        for (size_t t = 0; t < HIGH_THREAD_COUNT; ++t) {
            threads.emplace_back([&, t]() {
                // 每个线程执行 1000 次日志记录
                for (size_t i = 0; i < ITERATIONS_PER_THREAD; ++i) {
                    logger->info("Thread {}: High contention message {}", t, i);
                }
                // 原子递增完成计数器
                completed++;
            });
        }
        
        // 等待所有线程完成
        for (auto& thread : threads) {
            thread.join();
        }
        
        // 记录结束时间
        auto end = performance::clock::now();
        // 计算耗时
        performance::duration elapsed = end - start;
        
        // 刷新日志器
        logger->flush();
        
        // 构造并打印结果
        performance::BenchmarkResult result{
            "High Contention Sync - 32 threads",
            HIGH_THREAD_COUNT * ITERATIONS_PER_THREAD,
            elapsed.count(),
            static_cast<double>(HIGH_THREAD_COUNT * ITERATIONS_PER_THREAD) / elapsed.count(),
            (elapsed.count() * 1e9) / static_cast<double>(HIGH_THREAD_COUNT * ITERATIONS_PER_THREAD),
            HIGH_THREAD_COUNT
        };
        
        result.print();
        // 验证：所有线程都完成了
        REQUIRE(completed == HIGH_THREAD_COUNT);
        // 验证：每秒消息数大于 0
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("high_cont_sync");
    }
    
    /**
     * @brief 测试场景：异步日志器 - 高并发
     * 
     * 测试目的：测量异步日志器在 32 线程高并发下的性能
     * 
     * 预期结果：
     * - 由于队列缓冲，性能应该优于同步日志器
     * - 调用线程只需要将消息放入队列
     * - 实际 I/O 由后台线程执行
     * 
     * 配置说明：
     * - 队列大小：65536（足够大以避免阻塞）
     * - 后台线程数：4（并行处理日志）
     */
    SECTION("Async logger - high contention") {
        auto logger = performance::create_async_logger(
            "high_cont_async", PERF_LOG_DIR + "/high_cont_async.log", 65536, 4);
        
        std::atomic<size_t> completed{0};
        std::vector<std::thread> threads;
        threads.reserve(HIGH_THREAD_COUNT);
        
        auto start = performance::clock::now();
        
        for (size_t t = 0; t < HIGH_THREAD_COUNT; ++t) {
            threads.emplace_back([&, t]() {
                for (size_t i = 0; i < ITERATIONS_PER_THREAD; ++i) {
                    logger->info("Thread {}: High contention message {}", t, i);
                }
                completed++;
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        // 异步日志器需要 flush() 确保队列中的消息都被处理
        logger->flush();
        auto end = performance::clock::now();
        performance::duration elapsed = end - start;
        
        performance::BenchmarkResult result{
            "High Contention Async - 32 threads",
            HIGH_THREAD_COUNT * ITERATIONS_PER_THREAD,
            elapsed.count(),
            static_cast<double>(HIGH_THREAD_COUNT * ITERATIONS_PER_THREAD) / elapsed.count(),
            (elapsed.count() * 1e9) / static_cast<double>(HIGH_THREAD_COUNT * ITERATIONS_PER_THREAD),
            HIGH_THREAD_COUNT
        };
        
        result.print();
        REQUIRE(completed == HIGH_THREAD_COUNT);
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("high_cont_async");
    }
    
    performance::Benchmark::print_separator();
}
