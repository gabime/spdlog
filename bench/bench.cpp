//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// bench.cpp : spdlog benchmarks
//
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
#include "utils.h"
#include <atomic>
#include <cstdlib> // EXIT_FAILURE
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

int main(int argc, char *argv[])
{

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

        cout << "******************************************************************"
                "*************\n";
        cout << "Single thread, " << format(howmany) << " iterations" << endl;
        cout << "******************************************************************"
                "*************\n";

        auto basic_st = spdlog::basic_logger_st("basic_st", "logs/basic_st.log", true);
        bench(howmany, basic_st);

        auto rotating_st = spdlog::rotating_logger_st("rotating_st", "logs/rotating_st.log", file_size, rotating_files);
        bench(howmany, rotating_st);

        auto daily_st = spdlog::daily_logger_st("daily_st", "logs/daily_st.log");
        bench(howmany, daily_st);

        bench(howmany, spdlog::create<null_sink_st>("null_st"));

        cout << "\n****************************************************************"
                "***************\n";
        cout << threads << " threads sharing same logger, " << format(howmany) << " iterations" << endl;
        cout << "******************************************************************"
                "*************\n";

        auto basic_mt = spdlog::basic_logger_mt("basic_mt", "logs/basic_mt.log", true);
        bench_mt(howmany, basic_mt, threads);

        auto rotating_mt = spdlog::rotating_logger_mt("rotating_mt", "logs/rotating_mt.log", file_size, rotating_files);
        bench_mt(howmany, rotating_mt, threads);

        auto daily_mt = spdlog::daily_logger_mt("daily_mt", "logs/daily_mt.log");
        bench_mt(howmany, daily_mt, threads);
        bench_mt(howmany, spdlog::create<null_sink_mt>("null_mt"), threads);

        cout << "\n****************************************************************"
                "***************\n";
        cout << "async logging.. " << threads << " threads sharing same logger, " << format(howmany) << " iterations " << endl;
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
    using std::chrono::high_resolution_clock;
    cout << log->name() << "...\t\t" << flush;
    auto start = high_resolution_clock::now();
    for (auto i = 0; i < howmany; ++i)
    {
        log->info("Hello logger: msg number {}", i);
    }

    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();

    cout << "Elapsed: " << delta_d << "\t" << format(int(howmany / delta_d)) << "/sec" << endl;
    spdlog::drop(log->name());
}

void bench_mt(int howmany, std::shared_ptr<spdlog::logger> log, int thread_count)
{
    using std::chrono::high_resolution_clock;
    cout << log->name() << "...\t\t" << flush;
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
    cout << "Elapsed: " << delta_d << "\t" << format(int(howmany / delta_d)) << "/sec" << endl;
}
