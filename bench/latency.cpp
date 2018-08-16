//
// Copyright(c) 2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// latency.cpp : spdlog latency benchmarks
//
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
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

int main(int, char *[])
{
    std::srand(static_cast<unsigned>(std::time(nullptr))); // use current time as seed for random generator
    int howmany = 1000000;
    int queue_size = howmany + 2;
    int threads = 10;
    size_t file_size = 30 * 1024 * 1024;
    size_t rotating_files = 5;

    try
    {

        cout << "******************************************************************"
                "*************\n";
        cout << "Single thread\n";
        cout << "******************************************************************"
                "*************\n";

        auto basic_st = spdlog::basic_logger_mt("basic_st", "logs/basic_st.log", true);
        bench(howmany, basic_st);

        auto rotating_st = spdlog::rotating_logger_st("rotating_st", "logs/rotating_st.log", file_size, rotating_files);
        bench(howmany, rotating_st);

        auto daily_st = spdlog::daily_logger_st("daily_st", "logs/daily_st.log");
        bench(howmany, daily_st);

        bench(howmany, spdlog::create<null_sink_st>("null_st"));

        cout << "\n****************************************************************"
                "***************\n";
        cout << threads << " threads sharing same logger\n";
        cout << "******************************************************************"
                "*************\n";

        auto basic_mt = spdlog::basic_logger_mt("basic_mt", "logs/basic_mt.log", true);
        bench_mt(howmany, basic_mt, threads);

        auto rotating_mt = spdlog::rotating_logger_mt("rotating_mt", "logs/rotating_mt.log", file_size, rotating_files);
        bench_mt(howmany, rotating_mt, threads);

        auto daily_mt = spdlog::daily_logger_mt("daily_mt", "logs/daily_mt.log");
        bench_mt(howmany, daily_mt, threads);
        bench(howmany, spdlog::create<null_sink_st>("null_mt"));

        cout << "\n****************************************************************"
                "***************\n";
        cout << "async logging.. " << threads << " threads sharing same logger\n";
        cout << "******************************************************************"
                "*************\n";

        for (int i = 0; i < 3; ++i)
        {
            spdlog::init_thread_pool(static_cast<size_t>(queue_size), 1);
            auto as = spdlog::basic_logger_mt<spdlog::async_factory>("async", "logs/basic_async.log", true);
            bench_mt(howmany, as, threads);
            spdlog::drop("async");
        }
    }
    catch (std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        perror("Last error");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void bench(int howmany, std::shared_ptr<spdlog::logger> log)
{
    using namespace std::chrono;
    using chrono::high_resolution_clock;
    using chrono::milliseconds;
    using chrono::nanoseconds;

    cout << log->name() << "...\t\t" << flush;
    nanoseconds total_nanos = nanoseconds::zero();
    for (auto i = 0; i < howmany; ++i)
    {
        auto start = high_resolution_clock::now();
        log->info("Hello logger: msg number {}", i);
        auto delta_nanos = chrono::duration_cast<nanoseconds>(high_resolution_clock::now() - start);
        total_nanos += delta_nanos;
    }

    auto avg = total_nanos.count() / howmany;
    cout << format(avg) << " ns/call" << endl;
}

void bench_mt(int howmany, std::shared_ptr<spdlog::logger> log, int thread_count)
{
    using namespace std::chrono;
    using chrono::high_resolution_clock;
    using chrono::milliseconds;
    using chrono::nanoseconds;

    cout << log->name() << "...\t\t" << flush;
    vector<thread> threads;
    std::atomic<nanoseconds::rep> total_nanos{0};
    for (int t = 0; t < thread_count; ++t)
    {
        threads.push_back(std::thread([&]() {
            for (int j = 0; j < howmany / thread_count; j++)
            {
                auto start = high_resolution_clock::now();
                log->info("Hello logger: msg number {}", j);
                auto delta_nanos = chrono::duration_cast<nanoseconds>(high_resolution_clock::now() - start);
                total_nanos += delta_nanos.count();
            }
        }));
    }

    for (auto &t : threads)
    {
        t.join();
    };

    auto avg = total_nanos / howmany;
    cout << format(avg) << " ns/call" << endl;
}
