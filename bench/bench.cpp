//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// bench.cpp : spdlog benchmarks
//
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include "utils.h"
#include <atomic>
#include <cstdlib> // EXIT_FAILURE
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
void bench_default_api(int howmany, std::shared_ptr<spdlog::logger> log);
void bench_c_string(int howmany, std::shared_ptr<spdlog::logger> log);

int main(int argc, char *argv[])
{

    spdlog::default_logger()->set_pattern("[%^%l%$] %v");
    int howmany = 1000000;
    int queue_size = howmany + 2;
    int threads = 10;
    size_t file_size = 30 * 1024 * 1024;
    size_t rotating_files = 5;

    try
    {

        if (argc > 1)
            howmany = atoi(argv[1]);
        if (argc > 2)
            threads = atoi(argv[2]);
        if (argc > 3)
            queue_size = atoi(argv[3]);

        spdlog::info("**************************************************************");
        spdlog::info("Single thread, {:n} iterations", howmany);
        spdlog::info("**************************************************************");

        auto basic_st = spdlog::basic_logger_st("basic_st", "logs/basic_st.log", true);
        bench(howmany, std::move(basic_st));

        basic_st.reset();
        auto rotating_st = spdlog::rotating_logger_st("rotating_st", "logs/rotating_st.log", file_size, rotating_files);
        bench(howmany, std::move(rotating_st));

        auto daily_st = spdlog::daily_logger_st("daily_st", "logs/daily_st.log");
        bench(howmany, std::move(daily_st));

        bench(howmany, spdlog::create<null_sink_st>("null_st"));

        spdlog::info("**************************************************************");
        spdlog::info("C-string (400 bytes). Single thread, {:n} iterations", howmany);
        spdlog::info("**************************************************************");

        basic_st = spdlog::basic_logger_st("basic_st", "logs/basic_cs.log", true);
        bench_c_string(howmany, std::move(basic_st));

        rotating_st = spdlog::rotating_logger_st("rotating_st", "logs/rotating_cs.log", file_size, rotating_files);
        bench_c_string(howmany, std::move(rotating_st));

        daily_st = spdlog::daily_logger_st("daily_st", "logs/daily_cs.log");
        bench_c_string(howmany, std::move(daily_st));

        bench_c_string(howmany, spdlog::create<null_sink_st>("null_st"));

        spdlog::info("**************************************************************");
        spdlog::info("{:n} threads sharing same logger, {:n} iterations", threads, howmany);
        spdlog::info("**************************************************************");

        auto basic_mt = spdlog::basic_logger_mt("basic_mt", "logs/basic_mt.log", true);
        bench_mt(howmany, std::move(basic_mt), threads);

        auto rotating_mt = spdlog::rotating_logger_mt("rotating_mt", "logs/rotating_mt.log", file_size, rotating_files);
        bench_mt(howmany, std::move(rotating_mt), threads);

        auto daily_mt = spdlog::daily_logger_mt("daily_mt", "logs/daily_mt.log");
        bench_mt(howmany, std::move(daily_mt), threads);
        bench_mt(howmany, spdlog::create<null_sink_mt>("null_mt"), threads);

        spdlog::info("**************************************************************");
        spdlog::info("Asyncronous.. {:n} threads sharing same logger, {:n} iterations", threads, howmany);
        spdlog::info("**************************************************************");

        for (int i = 0; i < 3; ++i)
        {
            spdlog::init_thread_pool(static_cast<size_t>(queue_size), 1);
            auto as = spdlog::basic_logger_mt<spdlog::async_factory>("async", "logs/basic_async.log", true);
            bench_mt(howmany, std::move(as), threads);
        }
    }
    catch (std::exception &ex)
    {
        spdlog::error(ex.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void bench(int howmany, std::shared_ptr<spdlog::logger> log)
{
    using std::chrono::high_resolution_clock;
    auto start = high_resolution_clock::now();
    for (auto i = 0; i < howmany; ++i)
    {
        log->info("Hello logger: msg number {}", i);
    }

    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();

    spdlog::info("{:<16} Elapsed: {:0.2f} secs {:>16n}/sec", log->name(), delta_d, int(howmany / delta_d));
    spdlog::drop(log->name());
}

void bench_mt(int howmany, std::shared_ptr<spdlog::logger> log, int thread_count)
{
    using std::chrono::high_resolution_clock;
    vector<thread> threads;
    auto start = high_resolution_clock::now();
    for (int t = 0; t < thread_count; ++t)
    {
        threads.push_back(std::thread([&]() {
            for (int j = 0; j < howmany / thread_count; j++)
            {
                log->info("Hello logger: msg number {}", j);
            }
        }));
    }

    for (auto &t : threads)
    {
        t.join();
    };

    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();
    spdlog::info("{:<16} Elapsed: {:0.2f} secs {:>16n}/sec", log->name(), delta_d, int(howmany / delta_d));
    spdlog::drop(log->name());
}

void bench_default_api(int howmany, std::shared_ptr<spdlog::logger> log)
{
    using std::chrono::high_resolution_clock;
    auto orig_default = spdlog::default_logger();
    spdlog::set_default_logger(log);
    auto start = high_resolution_clock::now();
    for (auto i = 0; i < howmany; ++i)
    {
        spdlog::info("Hello logger: msg number {}", i);
    }

    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();
    spdlog::drop(log->name());
    spdlog::set_default_logger(std::move(orig_default));
    spdlog::info("{:<16} Elapsed: {:0.2f} secs {:>16n}/sec", log->name(), delta_d, int(howmany / delta_d));
}

void bench_c_string(int howmany, std::shared_ptr<spdlog::logger> log)
{
    const char *msg = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum pharetra metus cursus "
                      "lacus placerat congue. Nulla egestas, mauris a tincidunt tempus, enim lectus volutpat mi, eu consequat sem "
                      "libero nec massa. In dapibus ipsum a diam rhoncus gravida. Etiam non dapibus eros. Donec fringilla dui sed "
                      "augue pretium, nec scelerisque est maximus. Nullam convallis, sem nec blandit maximus, nisi turpis ornare "
                      "nisl, sit amet volutpat neque massa eu odio. Maecenas malesuada quam ex, posuere congue nibh turpis duis.";
    using std::chrono::high_resolution_clock;
    auto orig_default = spdlog::default_logger();
    spdlog::set_default_logger(log);
    auto start = high_resolution_clock::now();
    for (auto i = 0; i < howmany; ++i)
    {
        spdlog::log(level::info, msg);
    }

    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();
    spdlog::drop(log->name());
    spdlog::set_default_logger(std::move(orig_default));
    spdlog::info("{:<16} Elapsed: {:0.2f} secs {:>16n}/sec", log->name(), delta_d, int(howmany / delta_d));
}
