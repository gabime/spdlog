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

void prepare_logdir()
{
    spdlog::info("Preparing latency_logs directory..");
#ifdef _WIN32
    system("if not exist logs mkdir latency_logs");
    system("del /F /Q logs\\*");
#else
    auto rv = system("mkdir -p latency_logs");
    if (rv != 0)
    {
        throw std::runtime_error("Failed to mkdir -p latency_logs");
    }
    rv = system("rm -f latency_logs/*");
    if (rv != 0)
    {
        throw std::runtime_error("Failed to rm -f latency_logs/*");
    }
#endif
}

void bench_c_string(benchmark::State &state, std::shared_ptr<spdlog::logger> logger)
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

void bench_logger(benchmark::State &state, std::shared_ptr<spdlog::logger> logger)
{
    int i = 0;
    for (auto _ : state)
    {
        logger->info("Hello logger: msg number {}...............", ++i);
    }
}

void bench_disabled_macro(benchmark::State &state, std::shared_ptr<spdlog::logger> logger)
{
    int i = 0;
    benchmark::DoNotOptimize(i);      // prevent unused warnings
    benchmark::DoNotOptimize(logger); // prevent unused warnings
    for (auto _ : state)
    {
        SPDLOG_LOGGER_DEBUG(logger, "Hello logger: msg number {}...............", i++);
        SPDLOG_DEBUG("Hello logger: msg number {}...............", i++);
    }
}

int main(int argc, char *argv[])
{
    
    using spdlog::sinks::basic_file_sink_mt;
    using spdlog::sinks::basic_file_sink_st;
    using spdlog::sinks::null_sink_mt;
    using spdlog::sinks::null_sink_st;

    size_t file_size = 30 * 1024 * 1024;
    size_t rotating_files = 5;
    int n_threads = benchmark::CPUInfo::Get().num_cpus;

    prepare_logdir();

    // disabled loggers
    auto disabled_logger = std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_mt>());
    disabled_logger->set_level(spdlog::level::off);
    benchmark::RegisterBenchmark("disabled-at-compile-time", bench_disabled_macro, disabled_logger);
    benchmark::RegisterBenchmark("disabled-at-runtime", bench_logger, disabled_logger);

    auto null_logger_st = std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_st>());
    benchmark::RegisterBenchmark("null_sink_st (500_bytes c_str)", bench_c_string, std::move(null_logger_st));
    benchmark::RegisterBenchmark("null_sink_st", bench_logger, null_logger_st);

    // basic_st
    auto basic_st = spdlog::basic_logger_st("basic_st", "latency_logs/basic_st.log", true);
    benchmark::RegisterBenchmark("basic_st", bench_logger, std::move(basic_st))->UseRealTime();
    spdlog::drop("basic_st");

    // rotating st
    auto rotating_st = spdlog::rotating_logger_st("rotating_st", "latency_logs/rotating_st.log", file_size, rotating_files);
    benchmark::RegisterBenchmark("rotating_st", bench_logger, std::move(rotating_st))->UseRealTime();
    spdlog::drop("rotating_st");

    // daily st
    auto daily_st = spdlog::daily_logger_mt("daily_st", "latency_logs/daily_st.log");
    benchmark::RegisterBenchmark("daily_st", bench_logger, std::move(daily_st))->UseRealTime();
    spdlog::drop("daily_st");

    //    //
    //    // Multi threaded bench, 10 loggers using same logger concurrently
    //    //
    auto null_logger_mt = std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_mt>());
    benchmark::RegisterBenchmark("null_sink_mt", bench_logger, null_logger_mt)->Threads(n_threads)->UseRealTime();

    // basic_mt
    auto basic_mt = spdlog::basic_logger_mt("basic_mt", "latency_logs/basic_mt.log", true);
    benchmark::RegisterBenchmark("basic_mt", bench_logger, std::move(basic_mt))->Threads(n_threads)->UseRealTime();
    spdlog::drop("basic_mt");

    // rotating mt
    auto rotating_mt = spdlog::rotating_logger_mt("rotating_mt", "latency_logs/rotating_mt.log", file_size, rotating_files);
    benchmark::RegisterBenchmark("rotating_mt", bench_logger, std::move(rotating_mt))->Threads(n_threads)->UseRealTime();
    spdlog::drop("rotating_mt");

    // daily mt
    auto daily_mt = spdlog::daily_logger_mt("daily_mt", "latency_logs/daily_mt.log");
    benchmark::RegisterBenchmark("daily_mt", bench_logger, std::move(daily_mt))->Threads(n_threads)->UseRealTime();
    spdlog::drop("daily_mt");

    // async
    auto queue_size = 1024 * 1024 * 3;
    auto tp = std::make_shared<spdlog::details::thread_pool>(queue_size, 1);
    auto async_logger = std::make_shared<spdlog::async_logger>(
        "async_logger", std::make_shared<null_sink_mt>(), std::move(tp), spdlog::async_overflow_policy::overrun_oldest);
    benchmark::RegisterBenchmark("async_logger", bench_logger, async_logger)->Threads(n_threads)->UseRealTime();

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}
