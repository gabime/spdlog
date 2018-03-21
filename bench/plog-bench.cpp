//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <chrono>
#include <iostream>
#include <memory>

#include "plog/Log.h"

int main(int, char *[])
{
    using namespace std::chrono;
    using clock = steady_clock;

    int howmany = 1000000;

    plog::init(plog::debug, "logs/plog-bench.log");

    auto start = clock::now();
    for (int i = 0; i < howmany; ++i)
        LOG_INFO << "plog message #" << i << ": This is some text for your pleasure";

    duration<float> delta = clock::now() - start;
    float deltaf = delta.count();
    auto rate = howmany / deltaf;

    std::cout << "Total: " << howmany << std::endl;
    std::cout << "Delta = " << deltaf << " seconds" << std::endl;
    std::cout << "Rate = " << rate << "/sec" << std::endl;

    return 0;
}
