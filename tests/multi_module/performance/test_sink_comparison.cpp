#include "performance_utils.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <catch2/catch_all.hpp>
#include <string>
#include <memory>

namespace {
const std::string PERF_LOG_DIR = "perf_logs";           // 性能测试日志目录
const size_t ITERATIONS = 100000;                       // 迭代次数
}

/**
 * @brief 测试用例：Sink 性能对比 - 单线程
 * @tags [performance][sink]
 * 
 * 测试目的：测量不同类型 sink 在单线程场景下的性能表现。
 * 
 * 测试场景：
 * 1. Null sink（基线）
 * 2. Basic file sink
 * 3. Rotating file sink
 * 4. Daily file sink
 * 5. Stdout color sink（过滤模式）
 * 
 * Sink 类型说明：
 * 
 * Null sink:
 * - 所有写入操作都会被忽略
 * - 不执行任何 I/O 操作
 * - 作为性能基线，测量纯 CPU 开销
 * 
 * Basic file sink:
 * - 最简单的文件 sink
 * - 直接写入文件，不进行轮转
 * - 适合日志量不大的场景
 * 
 * Rotating file sink:
 * - 按文件大小轮转
 * - 当文件大小超过阈值时创建新文件
 * - 保留指定数量的历史文件
 * - 适合需要控制日志文件大小的场景
 * 
 * Daily file sink:
 * - 按日期轮转
 * - 每天创建新的日志文件
 * - 适合按日期归档日志的场景
 * 
 * Stdout color sink:
 * - 输出到控制台
 * - 支持彩色输出
 * - 测试中设置为 WARN 级别，INFO 日志会被过滤
 * - 用于测量过滤开销
 * 
 * 性能指标：
 * - 每秒消息数（Msg/s）
 * - 平均延迟（Latency, ns）
 * 
 * 性能对比意义：
 * - 帮助选择适合场景的 sink 类型
 * - 了解不同 sink 的性能开销
 * - 为生产环境配置提供参考
 */
TEST_CASE("Sink comparison - single threaded", "[performance][sink]") {
    // 准备测试环境：清除旧日志，创建新目录
    performance::prepare_logdir(PERF_LOG_DIR);
    
    // 打印性能测试结果表头
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    /**
     * @brief 测试场景：Null sink（基线）
     * 
     * 测试目的：建立性能基线
     * 
     * 性能特点：
     * - 这应该是最快的 sink
     * - 只测量函数调用和格式处理开销
     * - 没有任何 I/O 开销
     * - 作为其他 sink 的对比基准
     */
    SECTION("Null sink (baseline)") {
        auto logger = performance::create_null_logger("sink_null");
        
        performance::Benchmark bench("Sink - Null (baseline)");
        bench.run(ITERATIONS, [&]() {
            logger->info("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("sink_null");
    }
    
    /**
     * @brief 测试场景：Basic file sink
     * 
     * 测试目的：测量基础文件 sink 的性能
     * 
     * 性能特点：
     * - 直接写入文件
     * - 没有轮转开销
     * - 性能应该优于轮转 sink
     * - 适合日志量不大的场景
     */
    SECTION("Basic file sink") {
        auto logger = performance::create_sync_logger(
            "sink_basic", PERF_LOG_DIR + "/sink_basic.log");
        
        performance::Benchmark bench("Sink - Basic File");
        bench.run(ITERATIONS, [&]() {
            logger->info("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("sink_basic");
    }
    
    /**
     * @brief 测试场景：Rotating file sink
     * 
     * 测试目的：测量轮转文件 sink 的性能
     * 
     * 配置：
     * - 最大文件大小：10MB
     * - 最大文件数：5
     * 
     * 性能特点：
     * - 需要检查文件大小
     * - 可能需要轮转文件
     * - 比 basic sink 稍慢
     * - 适合需要控制日志文件大小的场景
     * 
     * 注意：
     * - 测试中使用 10MB 阈值，100,000 次迭代不会触发轮转
     * - 主要测量检查文件大小的开销
     */
    SECTION("Rotating file sink") {
        auto logger = performance::create_rotating_logger(
            "sink_rotating", PERF_LOG_DIR + "/sink_rotating.log",
            10 * 1024 * 1024, 5);
        
        performance::Benchmark bench("Sink - Rotating File");
        bench.run(ITERATIONS, [&]() {
            logger->info("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("sink_rotating");
    }
    
    /**
     * @brief 测试场景：Daily file sink
     * 
     * 测试目的：测量按日期轮转的 sink 性能
     * 
     * 配置：
     * - 轮转时间：00:00（午夜）
     * 
     * 性能特点：
     * - 需要检查日期
     * - 可能需要创建新文件
     * - 适合按日期归档日志的场景
     * 
     * 注意：
     * - 测试中不会触发日期轮转
     * - 主要测量检查日期的开销
     */
    SECTION("Daily file sink") {
        // 创建按日期轮转的 sink
        // 参数：日志文件路径、轮转小时（0=午夜）、轮转分钟（0）
        auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
            PERF_LOG_DIR + "/sink_daily.log", 0, 0);
        auto logger = std::make_shared<spdlog::logger>("sink_daily", daily_sink);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        spdlog::register_logger(logger);
        
        performance::Benchmark bench("Sink - Daily File");
        bench.run(ITERATIONS, [&]() {
            logger->info("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("sink_daily");
    }
    
    /**
     * @brief 测试场景：Stdout color sink（过滤模式）
     * 
     * 测试目的：测量控制台 sink 在过滤模式下的性能
     * 
     * 配置：
     * - 日志级别：WARN
     * - 测试日志级别：INFO（会被过滤）
     * 
     * 性能特点：
     * - 日志会被过滤，不会实际输出
     * - 测量级别检查的开销
     * - 可以用来评估过滤的性能影响
     * 
     * 实际应用：
     * - 生产环境可以设置较高的日志级别
     * - 被过滤的日志几乎没有开销
     * - 这是一个重要的性能优化点
     */
    SECTION("Stdout color sink") {
        // 创建控制台 sink（带颜色输出）
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>("sink_stdout", stdout_sink);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        // 设置日志级别为 WARN，INFO 日志会被过滤
        logger->set_level(spdlog::level::warn);
        spdlog::register_logger(logger);
        
        performance::Benchmark bench("Sink - Stdout Color (filtered)");
        bench.run(ITERATIONS, [&]() {
            // INFO 级别日志会被过滤
            logger->info("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("sink_stdout");
    }
    
    // 打印分隔线
    performance::Benchmark::print_separator();
}

/**
 * @brief 测试用例：Sink 性能对比 - 多 sink 组合
 * @tags [performance][sink]
 * 
 * 测试目的：测量使用多个 sink 时的性能开销。
 * 
 * 测试场景：
 * 1. 单个 sink
 * 2. 两个 sink
 * 3. 三个 sink
 * 
 * 多 sink 场景说明：
 * - 一条日志需要写入多个目标
 * - 例如：同时写入文件和控制台
 * - 或者：同时写入多个文件
 * 
 * 性能分析：
 * - 每个 sink 都会处理日志消息
 * - 性能开销与 sink 数量成正比
 * - 需要权衡功能需求和性能开销
 * 
 * 实际应用建议：
 * - 只在必要时使用多个 sink
 * - 考虑使用异步日志器减少主线程阻塞
 * - 生产环境可以只使用文件 sink
 * - 开发环境可以同时使用文件和控制台 sink
 */
TEST_CASE("Sink comparison - multiple sinks", "[performance][sink]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    /**
     * @brief 测试场景：单个 sink
     * 
     * 测试目的：建立单 sink 性能基线
     * 
     * 配置：
     * - 1 个 basic file sink
     */
    SECTION("Single sink") {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            PERF_LOG_DIR + "/multi_sink_1.log", true);
        auto logger = std::make_shared<spdlog::logger>("multi_1", file_sink);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        spdlog::register_logger(logger);
        
        performance::Benchmark bench("Multiple Sinks - 1 sink");
        bench.run(ITERATIONS, [&]() {
            logger->info("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("multi_1");
    }
    
    /**
     * @brief 测试场景：两个 sink
     * 
     * 测试目的：测量两个 sink 的性能开销
     * 
     * 配置：
     * - 2 个 basic file sink
     * - 一条日志写入两个文件
     * 
     * 性能预期：
     * - 性能应该是单 sink 的约 2 倍
     * - 每条日志需要写入两个文件
     */
    SECTION("Two sinks") {
        auto file_sink1 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            PERF_LOG_DIR + "/multi_sink_2a.log", true);
        auto file_sink2 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            PERF_LOG_DIR + "/multi_sink_2b.log", true);
        
        // 组合两个 sink
        std::vector<spdlog::sink_ptr> sinks = {file_sink1, file_sink2};
        auto logger = std::make_shared<spdlog::logger>("multi_2", sinks.begin(), sinks.end());
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        spdlog::register_logger(logger);
        
        performance::Benchmark bench("Multiple Sinks - 2 sinks");
        bench.run(ITERATIONS, [&]() {
            logger->info("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("multi_2");
    }
    
    /**
     * @brief 测试场景：三个 sink
     * 
     * 测试目的：测量三个 sink 的性能开销
     * 
     * 配置：
     * - 2 个 basic file sink
     * - 1 个 null sink
     * 
     * 性能预期：
     * - 性能应该是单 sink 的约 3 倍
     * - null sink 虽然不执行 I/O，但仍有函数调用开销
     */
    SECTION("Three sinks") {
        auto file_sink1 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            PERF_LOG_DIR + "/multi_sink_3a.log", true);
        auto file_sink2 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            PERF_LOG_DIR + "/multi_sink_3b.log", true);
        auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        
        // 组合三个 sink
        std::vector<spdlog::sink_ptr> sinks = {file_sink1, file_sink2, null_sink};
        auto logger = std::make_shared<spdlog::logger>("multi_3", sinks.begin(), sinks.end());
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        spdlog::register_logger(logger);
        
        performance::Benchmark bench("Multiple Sinks - 3 sinks");
        bench.run(ITERATIONS, [&]() {
            logger->info("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("multi_3");
    }
    
    performance::Benchmark::print_separator();
}

/**
 * @brief 测试用例：Sink 性能对比 - 异步模式
 * @tags [performance][sink][async]
 * 
 * 测试目的：测量异步日志器配合不同 sink 的性能表现。
 * 
 * 测试场景：
 * 1. Async + Null sink
 * 2. Async + Basic file sink
 * 3. Async + Rotating file sink
 * 
 * 异步模式的特点：
 * - 调用线程只将消息放入队列
 * - 实际 I/O 由后台线程执行
 * - 调用线程的延迟更低
 * - 吞吐量更高
 * 
 * 性能对比：
 * - 异步模式 vs 同步模式
 * - 不同 sink 在异步模式下的差异
 * 
 * 实际应用建议：
 * - 高并发场景推荐使用异步日志器
 * - 队列大小需要根据预期日志量配置
 * - 后台线程数需要根据硬件配置调整
 */
TEST_CASE("Sink comparison - async with different sinks", "[performance][sink][async]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    /**
     * @brief 测试场景：异步 + Null sink
     * 
     * 测试目的：测量异步模式下的最小开销
     * 
     * 配置：
     * - 队列大小：8192
     * - 后台线程数：1
     * - Sink: null sink
     * 
     * 性能特点：
     * - 这应该是异步模式下最快的场景
     * - 只测量队列操作开销
     * - 作为异步模式的性能基线
     */
    SECTION("Async - Null sink") {
        // 创建线程池
        auto tp = std::make_shared<spdlog::details::thread_pool>(8192, 1);
        // 创建 null sink
        auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        // 创建异步日志器
        auto logger = std::make_shared<spdlog::async_logger>(
            "async_null", null_sink, tp, spdlog::async_overflow_policy::block);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        spdlog::register_logger(logger);
        
        performance::Benchmark bench("Async - Null Sink");
        // 异步更快，使用 2 倍迭代次数
        bench.run(ITERATIONS * 2, [&]() {
            logger->info("Test message");
        });
        // 异步日志器需要 flush() 确保队列中的消息都被处理
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("async_null");
    }
    
    /**
     * @brief 测试场景：异步 + Basic file sink
     * 
     * 测试目的：测量异步模式下基础文件 sink 的性能
     * 
     * 配置：
     * - 队列大小：8192
     * - 后台线程数：1
     * - Sink: basic file sink
     * 
     * 性能特点：
     * - 调用线程只需要将消息放入队列
     * - 实际文件写入由后台线程执行
     * - 调用线程的延迟应该显著低于同步模式
     */
    SECTION("Async - Basic file sink") {
        auto logger = performance::create_async_logger(
            "async_basic", PERF_LOG_DIR + "/async_basic.log", 8192, 1);
        
        performance::Benchmark bench("Async - Basic File Sink");
        bench.run(ITERATIONS * 2, [&]() {
            logger->info("Test message");
        });
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("async_basic");
    }
    
    /**
     * @brief 测试场景：异步 + Rotating file sink
     * 
     * 测试目的：测量异步模式下轮转文件 sink 的性能
     * 
     * 配置：
     * - 队列大小：8192
     * - 后台线程数：1
     * - Sink: rotating file sink
     * - 最大文件大小：10MB
     * - 最大文件数：5
     * 
     * 性能特点：
     * - 轮转检查由后台线程执行
     * - 不影响调用线程的性能
     * - 适合需要轮转日志的高并发场景
     */
    SECTION("Async - Rotating file sink") {
        // 创建线程池
        auto tp = std::make_shared<spdlog::details::thread_pool>(8192, 1);
        // 创建轮转文件 sink
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            PERF_LOG_DIR + "/async_rotating.log", 10 * 1024 * 1024, 5);
        // 创建异步日志器
        auto logger = std::make_shared<spdlog::async_logger>(
            "async_rotating", rotating_sink, tp, spdlog::async_overflow_policy::block);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        spdlog::register_logger(logger);
        
        performance::Benchmark bench("Async - Rotating File Sink");
        bench.run(ITERATIONS * 2, [&]() {
            logger->info("Test message");
        });
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("async_rotating");
    }
    
    performance::Benchmark::print_separator();
}
