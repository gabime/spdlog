#include "performance_utils.h"
#include <catch2/catch_all.hpp>
#include <string>

namespace {
const std::string PERF_LOG_DIR = "perf_logs";
const size_t SMALL_ITERATIONS = 10000;
const size_t MEDIUM_ITERATIONS = 100000;
}

TEST_CASE("Sync logger performance - basic file sink", "[performance][sync]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    auto logger = performance::create_sync_logger("sync_test", PERF_LOG_DIR + "/sync.log");
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    SECTION("Small message logging") {
        performance::Benchmark bench("Sync - Small message (basic_file_sink)");
        bench.run(MEDIUM_ITERATIONS, [&]() {
            logger->info("Hello");
        });
        
        auto result = bench.get_result();
        result.print();
        
        REQUIRE(result.messages_per_second > 0);
    }
    
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
    
    performance::Benchmark::print_separator();
    performance::shutdown_logger("sync_test");
}

TEST_CASE("Sync logger performance - different log levels", "[performance][sync]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    auto logger = performance::create_sync_logger("sync_levels", PERF_LOG_DIR + "/sync_levels.log");
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
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

TEST_CASE("Sync logger performance - null sink (baseline)", "[performance][sync]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    auto logger = performance::create_null_logger("null_sync");
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    SECTION("Null sink baseline") {
        performance::Benchmark bench("Sync - Null sink (baseline)");
        
        bench.run(MEDIUM_ITERATIONS, [&]() {
            logger->info("Hello");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
    }
    
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
