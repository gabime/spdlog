#include "performance_utils.h"
#include <catch2/catch_all.hpp>
#include <string>
#include <thread>

namespace {
const std::string PERF_LOG_DIR = "perf_logs";           // 性能测试日志目录
const size_t SMALL_ITERATIONS = 10000;                  // 小迭代次数（用于格式化测试）
const size_t MEDIUM_ITERATIONS = 100000;                // 中等迭代次数（用于常规测试）
const size_t LARGE_ITERATIONS = 500000;                 // 大迭代次数（用于异步测试，因为异步更快）
}

/**
 * @brief 测试用例：异步日志器性能 - 基础文件 sink
 * @tags [performance][async]
 * 
 * 测试目的：测量异步日志器在不同消息大小下的性能表现。
 * 
 * 测试场景：
 * 1. 小消息测试（"Hello"）
 * 2. 中等消息测试（固定长度字符串）
 * 3. 格式化消息测试（包含变量替换）
 * 
 * 异步日志器的特点：
 * - 日志调用不会阻塞在 I/O 操作上
 * - 消息先放入队列，由后台线程处理
 * - 延迟较低，吞吐量高
 * - 适合高并发、高日志量的场景
 * 
 * 性能指标：
 * - 每秒消息数（Msg/s）
 * - 平均延迟（Latency, ns）
 * 
 * 注意事项：
 * - 异步日志器需要在测试结束后调用 flush() 确保所有消息都被处理
 * - 迭代次数比同步测试多，因为异步更快
 */
TEST_CASE("Async logger performance - basic file sink", "[performance][async]") {
    // 准备测试环境：清除旧日志，创建新目录
    performance::prepare_logdir(PERF_LOG_DIR);
    
    // 创建异步日志器
    // 参数：日志器名称、日志文件路径、队列大小（8192）、后台线程数（1）
    auto logger = performance::create_async_logger(
        "async_test", PERF_LOG_DIR + "/async.log", 8192, 1);
    
    // 打印性能测试结果表头
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    /**
     * @brief 测试场景：小消息日志
     * 
     * 测试目的：测量最小开销场景下的异步性能
     * 
     * 消息内容："Hello"（短字符串，无格式化）
     * 迭代次数：500,000 次（异步更快，所以迭代次数更多）
     * 
     * 性能特点：
     * - 这应该是最快的场景之一
     * - 主要开销是队列操作和线程同步
     * - 实际 I/O 由后台线程执行
     */
    SECTION("Small message logging") {
        performance::Benchmark bench("Async - Small message (basic_file_sink)");
        bench.run(LARGE_ITERATIONS, [&]() {
            logger->info("Hello");
        });
        // 异步日志器需要 flush() 确保所有消息都被处理
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        
        // 验证：每秒消息数应该大于 0
        REQUIRE(result.messages_per_second > 0);
    }
    
    /**
     * @brief 测试场景：中等消息日志
     * 
     * 测试目的：测量中等长度消息的异步性能
     * 
     * 消息内容：固定长度的描述性字符串
     * 迭代次数：500,000 次
     * 
     * 预期结果：
     * - 比小消息稍慢（更多数据需要复制到队列）
     * - 但仍然比同步日志快很多
     */
    SECTION("Medium message logging") {
        performance::Benchmark bench("Async - Medium message (basic_file_sink)");
        const std::string medium_msg = "This is a medium length log message for async testing";
        
        bench.run(LARGE_ITERATIONS, [&]() {
            logger->info("{}", medium_msg);
        });
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        
        REQUIRE(result.messages_per_second > 0);
    }
    
    /**
     * @brief 测试场景：格式化消息日志
     * 
     * 测试目的：测量包含变量替换的格式化消息异步性能
     * 
     * 消息内容：包含计数器的格式化字符串
     * 迭代次数：100,000 次（格式化开销较大，减少迭代次数）
     * 
     * 性能特点：
     * - 需要解析格式字符串
     * - 需要进行类型转换和字符串拼接
     * - 这是最接近实际使用场景的测试
     * 
     * 注意：
     * - 格式化在调用线程中执行，然后才放入队列
     * - 所以格式化开销仍然会影响调用线程的性能
     */
    SECTION("Formatted message logging") {
        performance::Benchmark bench("Async - Formatted message (basic_file_sink)");
        
        bench.run(MEDIUM_ITERATIONS, [&]() {
            static int counter = 0;
            logger->info("Request {} processed in {}ms", counter++, 15);
        });
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        
        REQUIRE(result.messages_per_second > 0);
    }
    
    // 打印分隔线
    performance::Benchmark::print_separator();
    
    // 关闭日志器，确保所有日志都已写入
    performance::shutdown_logger("async_test");
}

/**
 * @brief 测试用例：异步日志器性能 - 不同队列大小
 * @tags [performance][async]
 * 
 * 测试目的：测量不同队列大小对异步日志器性能的影响。
 * 
 * 测试场景：
 * 1. 小队列（1024）
 * 2. 中队列（8192）
 * 3. 大队列（65536）
 * 
 * 队列大小的影响：
 * - 小队列：内存占用小，但可能导致更多阻塞
 * - 中队列：平衡内存和性能
 * - 大队列：内存占用大，但减少阻塞概率
 * 
 * 性能分析：
 * - 队列大小决定了可以缓冲的消息数量
 * - 如果队列满了，新的日志消息会根据 overflow_policy 处理
 * - 当前使用 block 策略：队列满时阻塞等待
 * 
 * 实际应用建议：
 * - 根据预期的日志量选择合适的队列大小
 * - 高并发场景建议使用较大的队列
 * - 内存受限场景使用较小的队列
 */
TEST_CASE("Async logger performance - different queue sizes", "[performance][async]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    /**
     * @brief 测试场景：小队列（1024）
     * 
     * 队列大小：1024 条消息
     * 后台线程：1 个
     * 
     * 性能特点：
     * - 内存占用最小
     * - 在高并发场景下可能导致更多阻塞
     * - 适合低日志量场景
     */
    SECTION("Small queue (1024)") {
        auto logger = performance::create_async_logger(
            "async_small_q", PERF_LOG_DIR + "/async_small_q.log", 1024, 1);
        
        performance::Benchmark bench("Async - Queue size 1024");
        bench.run(MEDIUM_ITERATIONS, [&]() {
            logger->info("Test message");
        });
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("async_small_q");
    }
    
    /**
     * @brief 测试场景：中队列（8192）
     * 
     * 队列大小：8192 条消息（spdlog 默认值）
     * 后台线程：1 个
     * 
     * 性能特点：
     * - 内存占用适中
     * - 性能和内存的平衡选择
     * - 适合大多数场景
     */
    SECTION("Medium queue (8192)") {
        auto logger = performance::create_async_logger(
            "async_medium_q", PERF_LOG_DIR + "/async_medium_q.log", 8192, 1);
        
        performance::Benchmark bench("Async - Queue size 8192");
        bench.run(MEDIUM_ITERATIONS, [&]() {
            logger->info("Test message");
        });
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("async_medium_q");
    }
    
    /**
     * @brief 测试场景：大队列（65536）
     * 
     * 队列大小：65536 条消息
     * 后台线程：1 个
     * 
     * 性能特点：
     * - 内存占用较大
     * - 减少阻塞概率
     * - 适合高并发、高日志量场景
     */
    SECTION("Large queue (65536)") {
        auto logger = performance::create_async_logger(
            "async_large_q", PERF_LOG_DIR + "/async_large_q.log", 65536, 1);
        
        performance::Benchmark bench("Async - Queue size 65536");
        bench.run(MEDIUM_ITERATIONS, [&]() {
            logger->info("Test message");
        });
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("async_large_q");
    }
    
    performance::Benchmark::print_separator();
}

/**
 * @brief 测试用例：异步日志器性能 - 不同后台线程数
 * @tags [performance][async]
 * 
 * 测试目的：测量不同后台线程数对异步日志器性能的影响。
 * 
 * 测试场景：
 * 1. 1 个后台线程
 * 2. 2 个后台线程
 * 3. 4 个后台线程
 * 
 * 后台线程数的影响：
 * - 单线程：简单场景，顺序处理日志
 * - 多线程：高并发场景，并行处理日志
 * 
 * 性能分析：
 * - 增加后台线程数可以提高 I/O 并行度
 * - 但也会增加线程同步开销
 * - 最优线程数取决于具体的硬件和工作负载
 * 
 * 实际应用建议：
 * - 单线程适合大多数场景
 * - 多线程适合高吞吐量场景
 * - 建议根据实际性能测试选择最优线程数
 */
TEST_CASE("Async logger performance - different thread counts", "[performance][async]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    /**
     * @brief 测试场景：1 个后台线程
     * 
     * 后台线程数：1 个
     * 队列大小：8192
     * 
     * 性能特点：
     * - 实现简单
     * - 没有线程同步开销
     * - 适合大多数场景
     */
    SECTION("1 background thread") {
        auto logger = performance::create_async_logger(
            "async_1t", PERF_LOG_DIR + "/async_1t.log", 8192, 1);
        
        performance::Benchmark bench("Async - 1 background thread");
        bench.run(LARGE_ITERATIONS, [&]() {
            logger->info("Test message");
        });
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("async_1t");
    }
    
    /**
     * @brief 测试场景：2 个后台线程
     * 
     * 后台线程数：2 个
     * 队列大小：8192
     * 
     * 性能特点：
     * - 可以并行处理日志
     * - 有一定的线程同步开销
     * - 适合中等并发场景
     */
    SECTION("2 background threads") {
        auto logger = performance::create_async_logger(
            "async_2t", PERF_LOG_DIR + "/async_2t.log", 8192, 2);
        
        performance::Benchmark bench("Async - 2 background threads");
        bench.run(LARGE_ITERATIONS, [&]() {
            logger->info("Test message");
        });
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("async_2t");
    }
    
    /**
     * @brief 测试场景：4 个后台线程
     * 
     * 后台线程数：4 个
     * 队列大小：8192
     * 
     * 性能特点：
     * - 更高的并行度
     * - 线程同步开销也更大
     * - 适合高吞吐量场景
     * 
     * 注意：
     * - 线程数不是越多越好
     * - 过多的线程可能导致上下文切换开销增加
     */
    SECTION("4 background threads") {
        auto logger = performance::create_async_logger(
            "async_4t", PERF_LOG_DIR + "/async_4t.log", 8192, 4);
        
        performance::Benchmark bench("Async - 4 background threads");
        bench.run(LARGE_ITERATIONS, [&]() {
            logger->info("Test message");
        });
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("async_4t");
    }
    
    performance::Benchmark::print_separator();
}

/**
 * @brief 测试用例：异步 vs 同步性能对比
 * @tags [performance][comparison]
 * 
 * 测试目的：直接对比异步日志器和同步日志器的性能差异。
 * 
 * 测试场景：
 * 1. 同步日志器
 * 2. 异步日志器
 * 
 * 性能对比：
 * - 同步日志器：每次调用都执行 I/O，延迟高，吞吐量低
 * - 异步日志器：调用只放入队列，延迟低，吞吐量高
 * 
 * 预期结果：
 * - 异步日志器的每秒消息数应该显著高于同步日志器
 * - 异步日志器的平均延迟应该显著低于同步日志器
 * 
 * 实际应用建议：
 * - 低延迟要求的场景使用异步日志器
 * - 可靠性要求极高的场景可以考虑同步日志器
 * - 大多数场景推荐使用异步日志器
 */
TEST_CASE("Async vs Sync performance comparison", "[performance][comparison]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    /**
     * @brief 测试场景：同步日志器
     * 
     * 日志器类型：同步
     * 迭代次数：100,000 次
     * 
     * 性能特点：
     * - 每次日志调用都执行 I/O 操作
     * - 延迟高，但可靠性好
     * - 适合日志量不大的场景
     */
    SECTION("Sync logging") {
        auto sync_logger = performance::create_sync_logger(
            "comp_sync", PERF_LOG_DIR + "/comp_sync.log");
        
        performance::Benchmark bench("Comparison - Sync logging");
        bench.run(MEDIUM_ITERATIONS, [&]() {
            sync_logger->info("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("comp_sync");
    }
    
    /**
     * @brief 测试场景：异步日志器
     * 
     * 日志器类型：异步
     * 队列大小：8192
     * 后台线程：1 个
     * 迭代次数：100,000 次
     * 
     * 性能特点：
     * - 日志调用只放入队列，不执行 I/O
     * - 延迟低，吞吐量高
     * - 适合高并发、高日志量场景
     */
    SECTION("Async logging") {
        auto async_logger = performance::create_async_logger(
            "comp_async", PERF_LOG_DIR + "/comp_async.log", 8192, 1);
        
        performance::Benchmark bench("Comparison - Async logging");
        bench.run(MEDIUM_ITERATIONS, [&]() {
            async_logger->info("Test message");
        });
        async_logger->flush();
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("comp_async");
    }
    
    performance::Benchmark::print_separator();
}
