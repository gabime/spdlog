//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <atomic>
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <thread>
#include <vector>

#include "spdlog/spdlog.h"

using namespace std;

int main(int argc, char *argv[])
{
    using namespace std::chrono;
    using clock = steady_clock;

    int thread_count = 10;
    if (argc > 1)
        thread_count = std::atoi(argv[1]);

    int howmany = 1000000;

    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("file_logger", "logs/spdlog-bench-mt.log", false);
    logger->set_pattern("[%Y-%m-%d %T.%F]: %L %t %v");

    std::atomic<int> msg_counter{0};
    std::vector<thread> threads;

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

    return 0;
}
