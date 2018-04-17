//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// bench.cpp : spdlog benchmarks
//
#include "spdlog/async_logger.h"
#include "spdlog/sinks/file_sinks.h"
#include "spdlog/sinks/null_sink.h"
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

    int queue_size = 1048576;
    int howmany = 1000000;
    int threads = 10;
    int file_size = 30 * 1024 * 1024;
    int rotating_files = 5;

    try
    {

        if (argc > 1)
            howmany = atoi(argv[1]);
        if (argc > 2)
            threads = atoi(argv[2]);
        if (argc > 3)
            queue_size = atoi(argv[3]);

        cout << "*******************************************************************************\n";
        cout << "Single thread, " << format(howmany) << " iterations" << endl;
        cout << "*******************************************************************************\n";

        auto rotating_st = spdlog::rotating_logger_st("rotating_st", "logs/rotating_st.log", file_size, rotating_files);
        bench(howmany, rotating_st);
        auto daily_st = spdlog::daily_logger_st("daily_st", "logs/daily_st.log");
        bench(howmany, daily_st);
        bench(howmany, spdlog::create<null_sink_st>("null_st"));

        cout << "\n*******************************************************************************\n";
        cout << threads << " threads sharing same logger, " << format(howmany) << " iterations" << endl;
        cout << "*******************************************************************************\n";

        auto rotating_mt = spdlog::rotating_logger_mt("rotating_mt", "logs/rotating_mt.log", file_size, rotating_files);
        bench_mt(howmany, rotating_mt, threads);

        auto daily_mt = spdlog::daily_logger_mt("daily_mt", "logs/daily_mt.log");
        bench_mt(howmany, daily_mt, threads);
        bench(howmany, spdlog::create<null_sink_st>("null_mt"));

        cout << "\n*******************************************************************************\n";
        cout << "async logging.. " << threads << " threads sharing same logger, " << format(howmany) << " iterations " << endl;
        cout << "*******************************************************************************\n";

        spdlog::set_async_mode(queue_size);

        for (int i = 0; i < 3; ++i)
        {
            auto as = spdlog::daily_logger_st("as", "logs/daily_async.log");
            bench_mt(howmany, as, threads);
            spdlog::drop("as");
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
    cout << log->name() << "...\t\t" << flush;
    auto start = system_clock::now();
    for (auto i = 0; i < howmany; ++i)
    {
        log->info("Hello logger: msg number {}", i);
    }

    auto delta = system_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();
    cout << format(int(howmany / delta_d)) << "/sec" << endl;
}

void bench_mt(int howmany, std::shared_ptr<spdlog::logger> log, int thread_count)
{

    cout << log->name() << "...\t\t" << flush;
    std::atomic<int> msg_counter{0};
    vector<thread> threads;
    auto start = system_clock::now();
    for (int t = 0; t < thread_count; ++t)
    {
        threads.push_back(std::thread([&]() {
            for (;;)
            {
                int counter = ++msg_counter;
                if (counter > howmany)
                    break;
                log->info("Hello logger: msg number {}", counter);
            }
        }));
    }

    for (auto &t : threads)
    {
        t.join();
    };

    auto delta = system_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();
    cout << format(int(howmany / delta_d)) << "/sec" << endl;
}
