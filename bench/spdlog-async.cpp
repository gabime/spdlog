//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

using namespace std;

int main(int argc, char *argv[])
{
    using namespace std::chrono;
    using clock = steady_clock;

    int thread_count = 10;
    if (argc > 1)
        thread_count = std::atoi(argv[1]);

    int howmany = 1000000;
    spdlog::init_thread_pool(howmany, 1);

    auto logger = spdlog::create_async_logger<spdlog::sinks::basic_file_sink_mt>("file_logger", "logs/spdlog-bench-async.log", false);
    logger->set_pattern("[%Y-%m-%d %T.%F]: %L %t %v");

    std::cout << "To stop, press <Enter>" << std::endl;
    std::atomic<bool> run{true};
    std::thread stoper(std::thread([&run]() {
        std::cin.get();
        run = false;
    }));

    while (run)
    {
        std::atomic<int> msg_counter{0};
        std::vector<std::thread> threads;

        auto start = clock::now();
        for (int t = 0; t < thread_count; ++t)
        {
            threads.push_back(std::thread([&]() {
                while (true)
                {
                    int counter = ++msg_counter;
                    if (counter > howmany)
                        break;
                    logger->info("spdlog message #{}: This is some text for your pleasure", counter);
                }
            }));
        }

        for (auto &t : threads)
        {
            t.join();
        }

        duration<float> delta = clock::now() - start;
        float deltaf = delta.count();
        auto rate = howmany / deltaf;

        std::cout << "Total: " << howmany << std::endl;
        std::cout << "Threads: " << thread_count << std::endl;
        std::cout << "Delta = " << std::fixed << deltaf << " seconds" << std::endl;
        std::cout << "Rate = " << std::fixed << rate << "/sec" << std::endl;
    } // while

    stoper.join();

    return 0;
}
