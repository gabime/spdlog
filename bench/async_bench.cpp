//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// bench.cpp : spdlog benchmarks
//
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "utils.h"
#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using namespace std;
using namespace std::chrono;
using namespace spdlog;
using namespace spdlog::sinks;
using namespace utils;

void bench_mt(int howmany, std::shared_ptr<spdlog::logger> log, int thread_count);

int count_lines(const char *filename)
{
    int counter = 0;
    auto *infile = fopen(filename, "r");
    int ch;
    while (EOF != (ch = getc(infile)))
    {
        if ('\n' == ch)
            counter++;
    }
    fclose(infile);

    return counter;
}
int main(int argc, char *argv[])
{

    int howmany = 1000000;
    int queue_size = howmany + 2;
    int threads = 10;
    int iters = 3;

    try
    {
        auto console = spdlog::stdout_color_mt("console");
        if (argc == 1)
        {
            console->set_pattern("%v");
            console->info("Usage: {} <message_count> <threads> <q_size> <iterations>", argv[0]);
            return 0;
        }

        if (argc > 1)
            howmany = atoi(argv[1]);
        if (argc > 2)
            threads = atoi(argv[2]);
        if (argc > 3)
            queue_size = atoi(argv[3]);

        if (argc > 4)
            iters = atoi(argv[4]);

        console->info("-------------------------------------------------");
        console->info("Messages: {:14n}", howmany);
        console->info("Threads : {:14n}", threads);
        console->info("Queue   : {:14n}", queue_size);
        console->info("Iters   : {:>14n}", iters);
        console->info("-------------------------------------------------");

        const char *filename = "logs/basic_async.log";

        for (int i = 0; i < iters; i++)
        {
            auto tp = std::make_shared<details::thread_pool>(queue_size, 1);
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
            auto logger = std::make_shared<async_logger>("async_logger", std::move(file_sink), std::move(tp), async_overflow_policy::block);
            bench_mt(howmany, std::move(logger), threads);
            auto count = count_lines(filename);

            if (count != howmany)
            {
                console->error("Test failed. {} has {:n} lines instead of {:n}", filename, count, howmany);
                exit(1);
            }
            else
            {
                console->info("Line count OK ({:n})\n", count);
            }
        }
    }
    catch (std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        perror("Last error");
        return 1;
    }
    return 0;
}

void thread_fun(std::shared_ptr<spdlog::logger> logger, int howmany)
{
    for (int i = 0; i < howmany; i++)
    {
        logger->info("Hello logger: msg number {}", i);
    }
}

void bench_mt(int howmany, std::shared_ptr<spdlog::logger> logger, int thread_count)
{
    using std::chrono::high_resolution_clock;
    vector<thread> threads;
    auto start = high_resolution_clock::now();

    int msgs_per_thread = howmany / thread_count;
    int msgs_per_thread_mod = howmany % thread_count;
    for (int t = 0; t < thread_count; ++t)
    {
        if (t == 0 && msgs_per_thread_mod)
            threads.push_back(std::thread(thread_fun, logger, msgs_per_thread + msgs_per_thread_mod));
        else
            threads.push_back(std::thread(thread_fun, logger, msgs_per_thread));
    }

    for (auto &t : threads)
    {
        t.join();
    };

    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();
    spdlog::get("console")->info("Elapsed: {} secs\t {:n}/sec", delta_d, int(howmany / delta_d));
}
