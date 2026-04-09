#include "performance_utils.h"
#include <catch2/catch_all.hpp>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

namespace {
const std::string PERF_LOG_DIR = "perf_logs";
const size_t TOTAL_ITERATIONS = 100000;
const std::vector<size_t> THREAD_COUNTS = {1, 2, 4, 8, 16};
}

TEST_CASE("Multithread sync logger performance", "[performance][multithread]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    for (size_t thread_count : THREAD_COUNTS) {
        auto logger = performance::create_sync_logger(
            "mt_sync_" + std::to_string(thread_count),
            PERF_LOG_DIR + "/mt_sync_" + std::to_string(thread_count) + ".log");
        
        std::atomic<size_t> counter{0};
        
        performance::Benchmark bench(
            "Multithread Sync - " + std::to_string(thread_count) + " threads");
        
        bench.run_multithreaded(TOTAL_ITERATIONS, thread_count,
            [&](size_t thread_id) {
                size_t local_counter = counter++;
                logger->info("Thread {}: Message {}", thread_id, local_counter);
            });
        
        logger->flush();
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("mt_sync_" + std::to_string(thread_count));
    }
    
    performance::Benchmark::print_separator();
}

TEST_CASE("Multithread async logger performance", "[performance][multithread]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    for (size_t thread_count : THREAD_COUNTS) {
        auto logger = performance::create_async_logger(
            "mt_async_" + std::to_string(thread_count),
            PERF_LOG_DIR + "/mt_async_" + std::to_string(thread_count) + ".log",
            65536, 2);
        
        std::atomic<size_t> counter{0};
        
        performance::Benchmark bench(
            "Multithread Async - " + std::to_string(thread_count) + " threads");
        
        bench.run_multithreaded(TOTAL_ITERATIONS * 2, thread_count,
            [&](size_t thread_id) {
                size_t local_counter = counter++;
                logger->info("Thread {}: Message {}", thread_id, local_counter);
            });
        
        logger->flush();
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("mt_async_" + std::to_string(thread_count));
    }
    
    performance::Benchmark::print_separator();
}

TEST_CASE("Multithread null sink performance (contention test)", "[performance][multithread]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    SECTION("Single-threaded null sink") {
        auto logger = performance::create_null_logger("null_st");
        
        performance::Benchmark bench("Null Sink - Single thread");
        bench.run(TOTAL_ITERATIONS * 10, [&]() {
            logger->info("Test message");
        });
        
        auto result = bench.get_result();
        result.print();
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("null_st");
    }
    
    for (size_t thread_count : THREAD_COUNTS) {
        auto logger = performance::create_null_logger(
            "null_mt_" + std::to_string(thread_count));
        
        std::atomic<size_t> counter{0};
        
        performance::Benchmark bench(
            "Null Sink - " + std::to_string(thread_count) + " threads");
        
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

TEST_CASE("Multithread logger with high contention", "[performance][multithread]") {
    performance::prepare_logdir(PERF_LOG_DIR);
    
    const size_t HIGH_THREAD_COUNT = 32;
    const size_t ITERATIONS_PER_THREAD = 1000;
    
    performance::Benchmark::print_header();
    performance::Benchmark::print_separator();
    
    SECTION("Sync logger - high contention") {
        auto logger = performance::create_sync_logger(
            "high_cont_sync", PERF_LOG_DIR + "/high_cont_sync.log");
        
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
        
        auto end = performance::clock::now();
        performance::duration elapsed = end - start;
        
        logger->flush();
        
        performance::BenchmarkResult result{
            "High Contention Sync - 32 threads",
            HIGH_THREAD_COUNT * ITERATIONS_PER_THREAD,
            elapsed.count(),
            static_cast<double>(HIGH_THREAD_COUNT * ITERATIONS_PER_THREAD) / elapsed.count(),
            (elapsed.count() * 1e9) / static_cast<double>(HIGH_THREAD_COUNT * ITERATIONS_PER_THREAD),
            HIGH_THREAD_COUNT
        };
        
        result.print();
        REQUIRE(completed == HIGH_THREAD_COUNT);
        REQUIRE(result.messages_per_second > 0);
        
        performance::shutdown_logger("high_cont_sync");
    }
    
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
