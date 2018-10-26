//
// Copyright(c) 2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// latency.cpp : spdlog latency benchmarks
//

#include "benchmark/benchmark.h"

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <chrono>

#include "utils.h"
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using namespace std;
using namespace std::chrono;
using namespace spdlog;
using namespace spdlog::sinks;
using namespace utils;


void prepare_logdir()
{
    spdlog::info("Preparing logs_bench directory..");
#ifdef _WIN32
    system("if not exist logs mkdir logs_bench");
    system("del /F /Q logs\\*");
#else
    auto rv = system("mkdir -p logs_bench");
    if (rv != 0)
    {
        throw std::runtime_error("Failed to mkdir -p logs_bench");
    }
    rv = system("rm -f logs_bench/*");
    if (rv != 0)
    {
        throw std::runtime_error("Failed to rm -f logs_bench/*");
    }
#endif
}


void add_rate(benchmark::State &state)
{
    state.counters["Rate"] = benchmark::Counter(1, benchmark::Counter::kIsIterationInvariantRate);
}

void bench_c_string(benchmark::State& state, std::shared_ptr<spdlog::logger> logger)
{
    const char *msg = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum pharetra metus cursus "
                      "lacus placerat congue. Nulla egestas, mauris a tincidunt tempus, enim lectus volutpat mi, eu consequat sem "
                      "libero nec massa. In dapibus ipsum a diam rhoncus gravida. Etiam non dapibus eros. Donec fringilla dui sed "
                      "augue pretium, nec scelerisque est maximus. Nullam convallis, sem nec blandit maximus, nisi turpis ornare "
                      "nisl, sit amet volutpat neque massa eu odio. Maecenas malesuada quam ex, posuere congue nibh turpis duis.";


    for (auto _ : state)
    {
        logger->info(msg);
    }
    add_rate(state);
}

void bench_logger(benchmark::State& state, std::shared_ptr<spdlog::logger> logger)
{
    int i = 0;
    for (auto _ : state)
    {
        logger->info("Hello logger: msg number {}...............", ++i);

    }
    add_rate(state);

}




int main(int argc, char *argv[])
{

    using spdlog::sinks::null_sink_st ;
    using spdlog::sinks::null_sink_mt ;
    using spdlog::sinks::basic_file_sink_st;
    using spdlog::sinks::basic_file_sink_mt;

    size_t file_size = 30 * 1024 * 1024;
    size_t rotating_files = 5;
    int n_threads = 10;

    prepare_logdir();

    //
    // Single threaded bench
    //
    auto null_logger = std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_st>());

    benchmark::RegisterBenchmark("null_sink_st-500_bytes_cstr", bench_c_string, null_logger);
    benchmark::RegisterBenchmark("null_sink_st", bench_logger, null_logger);
    benchmark::RegisterBenchmark("null_sink_st", bench_logger, null_logger)->UseRealTime();

    // basic_st
    auto basic_st = spdlog::basic_logger_st("basic_st", "logs_bench/basic_st.log", true);
    benchmark::RegisterBenchmark("basic_st", bench_logger, std::move(basic_st));
    benchmark::RegisterBenchmark("basic_st", bench_logger, std::move(basic_st))->UseRealTime();
    spdlog::drop("basic_st");


    // rotating st
    auto rotating_st = spdlog::rotating_logger_st("rotating_st", "logs_bench/rotating_st.log", file_size, rotating_files);
    benchmark::RegisterBenchmark("rotating_st", bench_logger, std::move(rotating_st));
    benchmark::RegisterBenchmark("rotating_st", bench_logger, std::move(rotating_st))->UseRealTime();
    spdlog::drop("rotating_st");


    // daily st
    auto daily_st = spdlog::daily_logger_mt("daily_st", "logs_bench/daily_st.log");
    benchmark::RegisterBenchmark("daily_st", bench_logger, std::move(daily_st));
    benchmark::RegisterBenchmark("daily_st", bench_logger, std::move(daily_st))->UseRealTime();
    spdlog::drop("daily_st");

    //
    // Multi threaded bench, 10 using same logger concurrently
    //
    auto null_logger_mt = std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_mt>());
    benchmark::RegisterBenchmark("null_sink_mt", bench_logger, null_logger_mt)->Threads(n_threads);
    benchmark::RegisterBenchmark("null_sink_mt", bench_logger, null_logger_mt)->Threads(n_threads)->UseRealTime();

    // basic_mt
    auto basic_mt = spdlog::basic_logger_mt("basic_mt", "logs_bench/basic_mt.log", true);
    benchmark::RegisterBenchmark("basic_mt", bench_logger, std::move(basic_mt))->Threads(n_threads);
    benchmark::RegisterBenchmark("basic_mt", bench_logger, std::move(basic_mt))->Threads(n_threads)->UseRealTime();
    spdlog::drop("basic_mt");


    // rotating mt
    auto rotating_mt = spdlog::rotating_logger_mt("rotating_mt", "logs_bench/rotating_mt.log", file_size, rotating_files);
    benchmark::RegisterBenchmark("rotating_mt", bench_logger, std::move(rotating_mt))->Threads(n_threads);
    benchmark::RegisterBenchmark("rotating_mt", bench_logger, std::move(rotating_mt))->Threads(n_threads)->UseRealTime();
    spdlog::drop("rotating_mt");


    // daily mt
    auto daily_mt = spdlog::daily_logger_mt("daily_mt", "logs_bench/daily_mt.log");
    benchmark::RegisterBenchmark("daily_mt", bench_logger, std::move(daily_mt))->Threads(n_threads);
    benchmark::RegisterBenchmark("daily_mt", bench_logger, std::move(daily_mt))->Threads(n_threads)->UseRealTime();
    spdlog::drop("daily_mt");

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

}
