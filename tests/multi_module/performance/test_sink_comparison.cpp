#include "performance_utils.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <catch2/catch_all.hpp>
#include <string>
#include <memory>

namespace {
const std::string PERF_LOG_DIR = "perf_logs";
const size_t ITERATIONS = 100000;
}

TEST_CASE("Sink comparison - single threaded", "[performance][sink]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
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
    
    SECTION("Daily file sink") {
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
    
    SECTION("Stdout color sink") {
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>("sink_stdout", stdout_sink);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        logger->set_level(spdlog::level::warn);
        spdlog::register_logger(logger);
        
        performance::Benchmark bench("Sink - Stdout Color (filtered)");
        bench.run(ITERATIONS, [&]() {
            logger->info("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("sink_stdout");
    }
    
    performance::Benchmark::print_separator();
}

TEST_CASE("Sink comparison - multiple sinks", "[performance][sink]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
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
    
    SECTION("Two sinks") {
        auto file_sink1 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            PERF_LOG_DIR + "/multi_sink_2a.log", true);
        auto file_sink2 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            PERF_LOG_DIR + "/multi_sink_2b.log", true);
        
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
    
    SECTION("Three sinks") {
        auto file_sink1 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            PERF_LOG_DIR + "/multi_sink_3a.log", true);
        auto file_sink2 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            PERF_LOG_DIR + "/multi_sink_3b.log", true);
        auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        
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

TEST_CASE("Sink comparison - async with different sinks", "[performance][sink][async]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    SECTION("Async - Null sink") {
        auto tp = std::make_shared<spdlog::details::thread_pool>(8192, 1);
        auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        auto logger = std::make_shared<spdlog::async_logger>(
            "async_null", null_sink, tp, spdlog::async_overflow_policy::block);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        spdlog::register_logger(logger);
        
        performance::Benchmark bench("Async - Null Sink");
        bench.run(ITERATIONS * 2, [&]() {
            logger->info("Test message");
        });
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("async_null");
    }
    
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
    
    SECTION("Async - Rotating file sink") {
        auto tp = std::make_shared<spdlog::details::thread_pool>(8192, 1);
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            PERF_LOG_DIR + "/async_rotating.log", 10 * 1024 * 1024, 5);
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
