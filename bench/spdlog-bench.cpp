//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <chrono>
#include <iostream>

#include "spdlog/spdlog.h"

int main(int, char *[])
{
    using namespace std::chrono;
    using clock = steady_clock;

    int howmany = 1000000;

    auto logger = spdlog::create<spdlog::sinks::simple_file_sink_st>("file_logger", "logs/spdlog-bench.log", false);
    logger->set_pattern("[%Y-%b-%d %T.%f]: %v");

    auto start = clock::now();
    for (int i = 0; i < howmany; ++i)
        logger->info("spdlog message #{} : This is some text for your pleasure", i);

    duration<float> delta = clock::now() - start;
    float deltaf = delta.count();
    auto rate = howmany / deltaf;

    std::cout << "Total: " << howmany << std::endl;
    std::cout << "Delta = " << deltaf << " seconds" << std::endl;
    std::cout << "Rate = " << rate << "/sec" << std::endl;

    return 0;
}
