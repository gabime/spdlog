#include "performance_utils.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/async.h"
#include <iostream>
#include <iomanip>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#endif

namespace performance {

void BenchmarkResult::print() const {
    std::cout << std::left << std::setw(40) << name
              << std::right << std::setw(12) << iterations
              << std::setw(12) << std::fixed << std::setprecision(3) << elapsed_seconds
              << std::setw(15) << std::fixed << std::setprecision(0) << messages_per_second
              << std::setw(15) << std::fixed << std::setprecision(2) << avg_latency_ns
              << std::setw(10) << thread_count
              << std::endl;
}

Benchmark::Benchmark(std::string name) : name_(std::move(name)) {}

void Benchmark::run(size_t iterations, const std::function<void()>& func) {
    iterations_ = iterations;
    thread_count_ = 1;
    
    start_ = clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        func();
    }
    end_ = clock::now();
}

void Benchmark::run_multithreaded(size_t iterations, size_t thread_count,
                                   const std::function<void(size_t)>& func) {
    iterations_ = iterations;
    thread_count_ = thread_count;
    
    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    
    size_t iterations_per_thread = iterations / thread_count;
    
    start_ = clock::now();
    for (size_t t = 0; t < thread_count; ++t) {
        threads.emplace_back([&, t, iterations_per_thread]() {
            for (size_t i = 0; i < iterations_per_thread; ++i) {
                func(t);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    end_ = clock::now();
}

BenchmarkResult Benchmark::get_result() const {
    duration elapsed = end_ - start_;
    double elapsed_sec = elapsed.count();
    
    return {
        name_,
        iterations_,
        elapsed_sec,
        static_cast<double>(iterations_) / elapsed_sec,
        (elapsed_sec * 1e9) / static_cast<double>(iterations_),
        thread_count_
    };
}

void Benchmark::print_header() {
    std::cout << std::left << std::setw(40) << "Benchmark"
              << std::right << std::setw(12) << "Iterations"
              << std::setw(12) << "Time(s)"
              << std::setw(15) << "Msg/s"
              << std::setw(15) << "Latency(ns)"
              << std::setw(10) << "Threads"
              << std::endl;
}

void Benchmark::print_separator() {
    std::cout << std::string(104, '-') << std::endl;
}

void prepare_logdir(const std::string& dir_name) {
    spdlog::drop_all();
    
#ifdef _WIN32
    std::string cmd = "rmdir /S /Q " + dir_name + " 2>nul";
    system(cmd.c_str());
    cmd = "mkdir " + dir_name;
    system(cmd.c_str());
#else
    std::string cmd = "rm -rf " + dir_name;
    auto rv = system(cmd.c_str());
    (void)rv;
    cmd = "mkdir -p " + dir_name;
    rv = system(cmd.c_str());
    if (rv != 0) {
        throw std::runtime_error("Failed to create directory: " + dir_name);
    }
#endif
}

std::shared_ptr<spdlog::logger> create_sync_logger(const std::string& name,
                                                      const std::string& filename) {
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
    auto logger = std::make_shared<spdlog::logger>(name, sink);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::register_logger(logger);
    return logger;
}

std::shared_ptr<spdlog::logger> create_async_logger(const std::string& name,
                                                       const std::string& filename,
                                                       size_t queue_size,
                                                       size_t thread_count) {
    auto tp = std::make_shared<spdlog::details::thread_pool>(queue_size, thread_count);
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
    auto logger = std::make_shared<spdlog::async_logger>(
        name, sink, tp, spdlog::async_overflow_policy::block);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::register_logger(logger);
    return logger;
}

std::shared_ptr<spdlog::logger> create_null_logger(const std::string& name) {
    auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>(name, sink);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::register_logger(logger);
    return logger;
}

std::shared_ptr<spdlog::logger> create_rotating_logger(const std::string& name,
                                                          const std::string& filename,
                                                          size_t max_size,
                                                          size_t max_files) {
    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        filename, max_size, max_files);
    auto logger = std::make_shared<spdlog::logger>(name, sink);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::register_logger(logger);
    return logger;
}

void shutdown_logger(const std::string& name) {
    auto logger = spdlog::get(name);
    if (logger) {
        logger->flush();
    }
    spdlog::drop(name);
}

}
