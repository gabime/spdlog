#include "performance_utils.h"
#include <catch2/catch_all.hpp>
#include <string>

namespace {
const std::string PERF_LOG_DIR = "perf_logs";           // 性能测试日志目录
const size_t SMALL_ITERATIONS = 10000;                  // 小迭代次数（用于格式化测试）
const size_t MEDIUM_ITERATIONS = 100000;                // 中等迭代次数（用于常规测试）
}

/**
 * @brief 测试用例：同步日志器性能 - 基础文件 sink
 * @tags [performance][sync]
 * 
 * 测试目的：测量同步日志器在不同消息大小下的性能表现。
 * 
 * 测试场景：
 * 1. 小消息测试（"Hello"）
 * 2. 中等消息测试（固定长度字符串）
 * 3. 格式化消息测试（包含变量替换）
 * 
 * 性能指标：
 * - 每秒消息数（Msg/s）
 * - 平均延迟（Latency, ns）
 * 
 * 注意事项：
 * - 同步日志器每次调用都会执行 I/O 操作
 * - 性能受磁盘 I/O 速度影响较大
 * - 格式化消息会有额外的 CPU 开销
 */
TEST_CASE("Sync logger performance - basic file sink", "[performance][sync]") {
    // 准备测试环境：清除旧日志，创建新目录
    performance::prepare_logdir(PERF_LOG_DIR);
    
    // 创建同步日志器，使用 basic_file_sink_mt
    auto logger = performance::create_sync_logger("sync_test", PERF_LOG_DIR + "/sync.log");
    
    // 打印性能测试结果表头
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    /**
     * @brief 测试场景：小消息日志
     * 
     * 测试目的：测量最小开销场景下的性能
     * 
     * 消息内容："Hello"（短字符串，无格式化）
     * 迭代次数：100,000 次
     * 
     * 预期结果：
     * - 这应该是最快的场景之一
     * - 主要开销是函数调用和 I/O 操作
     */
    SECTION("Small message logging") {
        performance::Benchmark bench("Sync - Small message (basic_file_sink)");
        bench.run(MEDIUM_ITERATIONS, [&]() {
            logger->info("Hello");
        });
        
        auto result = bench.get_result();
        result.print();
        
        // 验证：每秒消息数应该大于 0
        REQUIRE(result.messages_per_second > 0);
    }
    
    /**
     * @brief 测试场景：中等消息日志
     * 
     * 测试目的：测量中等长度消息的性能
     * 
     * 消息内容：固定长度的描述性字符串
     * 迭代次数：100,000 次
     * 
     * 预期结果：
     * - 比小消息稍慢（更多数据需要写入）
     * - 但仍然没有格式化开销
     */
    SECTION("Medium message logging") {
        performance::Benchmark bench("Sync - Medium message (basic_file_sink)");
        const std::string medium_msg = "This is a medium length log message that contains some data";
        
        bench.run(MEDIUM_ITERATIONS, [&]() {
            logger->info("{}", medium_msg);
        });
        
        auto result = bench.get_result();
        result.print();
        
        REQUIRE(result.messages_per_second > 0);
    }
    
    /**
     * @brief 测试场景：格式化消息日志
     * 
     * 测试目的：测量包含变量替换的格式化消息性能
     * 
     * 消息内容：包含计数器、IP 地址、端口号的格式化字符串
     * 迭代次数：10,000 次（格式化开销较大，减少迭代次数）
     * 
     * 性能特点：
     * - 需要解析格式字符串
     * - 需要进行类型转换和字符串拼接
     * - 这是最接近实际使用场景的测试
     * 
     * 预期结果：
     * - 比非格式化消息慢
     * - 但能反映真实应用的性能
     */
    SECTION("Formatted message logging") {
        performance::Benchmark bench("Sync - Formatted message (basic_file_sink)");
        
        bench.run(SMALL_ITERATIONS, [&]() {
            static int counter = 0;
            logger->info("User {} logged in from {} at {}", counter++, "192.168.1.1", 12345);
        });
        
        auto result = bench.get_result();
        result.print();
        
        REQUIRE(result.messages_per_second > 0);
    }
    
    // 打印分隔线
    performance::Benchmark::print_separator();
    
    // 关闭日志器，确保所有日志都已写入
    performance::shutdown_logger("sync_test");
}

/**
 * @brief 测试用例：同步日志器性能 - 不同日志级别
 * @tags [performance][sync]
 * 
 * 测试目的：测量不同日志级别设置对性能的影响。
 * 
 * 测试场景：
 * 1. INFO 级别日志（实际输出）
 * 2. DEBUG 级别日志（被过滤，不输出）
 * 3. WARN 级别日志（实际输出）
 * 
 * 性能对比：
 * - 被过滤的日志应该比实际输出的日志快很多
 * - 这是因为过滤只需要简单的级别比较，不需要执行 I/O
 * 
 * 实际应用建议：
 * - 生产环境可以设置较高的日志级别（如 INFO 或 WARN）
 * - 调试环境可以设置较低的日志级别（如 DEBUG 或 TRACE）
 */
TEST_CASE("Sync logger performance - different log levels", "[performance][sync]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    auto logger = performance::create_sync_logger("sync_levels", PERF_LOG_DIR + "/sync_levels.log");
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    /**
     * @brief 测试场景：INFO 级别日志
     * 
     * 日志级别设置：INFO
     * 实际日志级别：INFO
     * 
     * 结果：日志会被输出
     * 性能：需要执行完整的 I/O 操作
     */
    SECTION("Info level") {
        logger->set_level(spdlog::level::info);
        performance::Benchmark bench("Sync - Info level logging");
        
        bench.run(MEDIUM_ITERATIONS, [&]() {
            logger->info("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
    }
    
    /**
     * @brief 测试场景：DEBUG 级别日志（被过滤）
     * 
     * 日志级别设置：INFO
     * 实际日志级别：DEBUG
     * 
     * 结果：日志会被过滤，不会输出
     * 性能：只需要级别比较，不需要 I/O，应该快很多
     * 
     * 这是一个重要的性能优化点：
     * - 被过滤的日志几乎没有开销
     * - 可以在代码中保留大量 DEBUG 日志
     * - 生产环境通过设置日志级别来禁用
     */
    SECTION("Debug level (filtered)") {
        logger->set_level(spdlog::level::info);
        performance::Benchmark bench("Sync - Debug level (filtered)");
        
        bench.run(MEDIUM_ITERATIONS, [&]() {
            logger->debug("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
    }
    
    /**
     * @brief 测试场景：WARN 级别日志
     * 
     * 日志级别设置：WARN
     * 实际日志级别：WARN
     * 
     * 结果：日志会被输出
     * 性能：与 INFO 级别类似
     */
    SECTION("Warn level") {
        logger->set_level(spdlog::level::warn);
        performance::Benchmark bench("Sync - Warn level logging");
        
        bench.run(MEDIUM_ITERATIONS, [&]() {
            logger->warn("Test warning");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
    }
    
    performance::Benchmark::print_separator();
    performance::shutdown_logger("sync_levels");
}

/**
 * @brief 测试用例：同步日志器性能 - Null sink（基线测试）
 * @tags [performance][sync]
 * 
 * 测试目的：测量日志框架的纯 CPU 开销（不包括 I/O）。
 * 
 * 测试场景：
 * 1. Null sink 基线测试（简单消息）
 * 2. Null sink 格式化测试（包含变量替换）
 * 
 * Null sink 的特点：
 * - 所有写入操作都会被忽略
 * - 不执行任何 I/O 操作
 * - 但仍然会执行格式处理和函数调用
 * 
 * 性能意义：
 * - 这代表了日志框架的最小开销
 * - 可以用来计算实际 I/O 开销（其他 sink 性能 - Null sink 性能）
 * - 可以用来对比不同日志框架的纯 CPU 开销
 */
TEST_CASE("Sync logger performance - null sink (baseline)", "[performance][sync]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    // 创建 Null 日志器，所有操作都被忽略
    auto logger = performance::create_null_logger("null_sync");
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    /**
     * @brief 测试场景：Null sink 基线
     * 
     * 测试目的：测量最简单的日志调用开销
     * 
     * 消息内容："Hello"（无格式化）
     * 迭代次数：100,000 次
     * 
     * 性能特点：
     * - 这应该是最快的场景
     * - 只测量函数调用和参数传递的开销
     * - 没有 I/O，没有格式化
     */
    SECTION("Null sink baseline") {
        performance::Benchmark bench("Sync - Null sink (baseline)");
        
        bench.run(MEDIUM_ITERATIONS, [&]() {
            logger->info("Hello");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
    }
    
    /**
     * @brief 测试场景：Null sink 格式化
     * 
     * 测试目的：测量格式化处理的开销
     * 
     * 消息内容：包含变量替换的格式化字符串
     * 迭代次数：10,000 次
     * 
     * 性能特点：
     * - 测量格式字符串解析和变量替换的开销
     * - 这是实际应用中最常见的场景
     * - 可以用来评估格式化库的性能
     */
    SECTION("Null sink with formatting") {
        performance::Benchmark bench("Sync - Null sink with formatting");
        
        bench.run(SMALL_ITERATIONS, [&]() {
            static int counter = 0;
            logger->info("Value: {}, String: {}", counter++, "test");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
    }
    
    performance::Benchmark::print_separator();
    performance::shutdown_logger("null_sync");
}
