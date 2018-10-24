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

void bench(int howmany, std::shared_ptr<spdlog::logger> log);
void bench_mt(int howmany, std::shared_ptr<spdlog::logger> log, int thread_count);

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
}

void bench_formatted_int(benchmark::State& state, std::shared_ptr<spdlog::logger> logger)
{

    int i = 0;
    for (auto _ : state)
    {
        logger->info("Hello message {} {} {}", ++i, i , i);
    }
}




int main(int argc, char *argv[])
{

    using spdlog::sinks::null_sink_st ;
    using spdlog::sinks::null_sink_mt ;

    auto null_logger = std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_st>());
    benchmark::RegisterBenchmark("null_sink_st-500_bytes_cstr", bench_c_string, null_logger);
    benchmark::RegisterBenchmark("null_sink_st-formatted_int", bench_formatted_int, null_logger);

    // 10 threads
    int n_threads = 10;
    null_logger = std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_st>());
    benchmark::RegisterBenchmark("null_sink_mt-500_bytes_cstr", bench_c_string, null_logger) -> Threads(n_threads);
    benchmark::RegisterBenchmark("null_sink_mt-formatted_int", bench_formatted_int, null_logger) ->Threads(n_threads);
    
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

}
