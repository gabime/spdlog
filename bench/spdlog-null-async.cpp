//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// bench.cpp : spdlog benchmarks
//
#include "spdlog/async_logger.h"
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

size_t bench_as(int howmany, std::shared_ptr<spdlog::logger> log, int thread_count);

int main(int argc, char *argv[])
{

    int queue_size = 1048576;
    int howmany = 1000000;
    int threads = 10;
    int iters = 10;

    try
    {

        if (argc > 1)
            howmany = atoi(argv[1]);
        if (argc > 2)
            threads = atoi(argv[2]);
        if (argc > 3)
            queue_size = atoi(argv[3]);

        cout << "\n*******************************************************************************\n";
        cout << "async logging.. " << threads << " threads sharing same logger, " << format(howmany) << " messages " << endl;
        cout << "*******************************************************************************\n";

        spdlog::set_async_mode(queue_size);

        size_t total_rate = 0;

        for (int i = 0; i < iters; ++i)
        {
            // auto as = spdlog::daily_logger_st("as", "logs/daily_async");
            auto as = spdlog::create<null_sink_st>("async(null-sink)");
            total_rate += bench_as(howmany, as, threads);
            spdlog::drop("async(null-sink)");
        }
        std::cout << endl;
        std::cout << "Avg rate: " << format(total_rate / iters) << "/sec" << std::endl;
    }
    catch (std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        perror("Last error");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// return rate/sec
size_t bench_as(int howmany, std::shared_ptr<spdlog::logger> log, int thread_count)
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
    }

    auto delta = system_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();
    auto per_sec = size_t(howmany / delta_d);
    cout << format(per_sec) << "/sec" << endl;
    return per_sec;
}
