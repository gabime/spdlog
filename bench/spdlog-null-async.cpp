//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// bench.cpp : spdlog benchmarks
//
#include "spdlog/async.h"
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

    int howmany;
    int tp_queue_size;
    int tp_threads = 1;
    int client_threads = 10;
    int iters = 10;

    try
    {
        if (argc < 2)
        {
            cout << "Usage: " << argv[0] << " <howmany> [client_threads] [q_size] [tp_threads]" << endl;
            return (1);
        }

        howmany = atoi(argv[1]);
        if (argc > 2)
            client_threads = atoi(argv[2]);

        if (argc > 3)
            tp_queue_size = atoi(argv[3]);
        else
            tp_queue_size = howmany;

        if (argc > 4)
            tp_threads = atoi(argv[4]);

        cout << "\n*******************************************************************************\n";
        cout << "messages:\t" << format(howmany) << endl;
        cout << "client_threads:\t" << client_threads << endl;
        cout << "tp queue:\t" << format(tp_queue_size) << endl;
        cout << "tp threads:\t" << tp_threads << endl;
        cout << "*******************************************************************************\n";

        size_t total_rate = 0;

        for (int i = 0; i < iters; ++i)
        {
            spdlog::init_thread_pool(tp_queue_size, tp_threads);
            auto as = spdlog::create_async_logger<null_sink_mt>("async(null-sink)");
            total_rate += bench_as(howmany, as, client_threads);
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
