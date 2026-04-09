#include "performance_utils.h"
#include <catch2/catch_all.hpp>
#include <string>
#include <thread>

namespace {
const std::string PERF_LOG_DIR = "perf_logs";
const size_t SMALL_ITERATIONS = 10000;
const size_t MEDIUM_ITERATIONS = 100000;
const size_t LARGE_ITERATIONS = 500000;
}

TEST_CASE("Async logger performance - basic file sink", "[performance][async]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    auto logger = performance::create_async_logger(
        "async_test", PERF_LOG_DIR + "/async.log", 8192, 1);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    SECTION("Small message logging") {
        performance::Benchmark bench("Async - Small message (basic_file_sink)");
        bench.run(LARGE_ITERATIONS, [&]() {
            logger->info("Hello");
        });
        logger->flush();
        
        auto result = bench.get_result();
        result.print();
        
        REQUIRE(result.messages_per_second > 0);
    }
    
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
    
    performance::Benchmark::print_separator();
    performance::shutdown_logger("async_test");
}

TEST_CASE("Async logger performance - different queue sizes", "[performance][async]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
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

TEST_CASE("Async logger performance - different thread counts", "[performance][async]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
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

TEST_CASE("Async vs Sync performance comparison", "[performance][comparison]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
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
