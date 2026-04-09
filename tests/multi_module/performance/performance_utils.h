#pragma once

#include "spdlog/spdlog.h"
#include <chrono>
#include <string>
#include <functional>
#include <vector>

namespace performance {

using clock = std::chrono::high_resolution_clock;
using duration = std::chrono::duration<double>;
using time_point = std::chrono::time_point<clock>;

struct BenchmarkResult {
    std::string name;
    size_t iterations;
    double elapsed_seconds;
    double messages_per_second;
    double avg_latency_ns;
    size_t thread_count;
    
    void print() const;
};

class Benchmark {
public:
    explicit Benchmark(std::string name);
    
    void run(size_t iterations, const std::function<void()>& func);
    void run_multithreaded(size_t iterations, size_t thread_count, 
                           const std::function<void(size_t)>& func);
    
    BenchmarkResult get_result() const;
    
    static void print_header();
    static void print_separator();

private:
    std::string name_;
    size_t iterations_ = 0;
    size_t thread_count_ = 1;
    time_point start_;
    time_point end_;
};

void prepare_logdir(const std::string& dir_name = "perf_logs");

std::shared_ptr<spdlog::logger> create_sync_logger(const std::string& name, 
                                                      const std::string& filename);

std::shared_ptr<spdlog::logger> create_async_logger(const std::string& name,
                                                       const std::string& filename,
                                                       size_t queue_size = 8192,
                                                       size_t thread_count = 1);

std::shared_ptr<spdlog::logger> create_null_logger(const std::string& name);

std::shared_ptr<spdlog::logger> create_rotating_logger(const std::string& name,
                                                          const std::string& filename,
                                                          size_t max_size = 10 * 1024 * 1024,
                                                          size_t max_files = 5);

void shutdown_logger(const std::string& name);

}
