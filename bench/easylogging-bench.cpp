//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <chrono>
#include <iostream>

#include "easylogging++.cc"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

int main(int, char *[])
{
    using namespace std::chrono;
    using clock = steady_clock;

    int howmany = 1000000;

    // Load configuration from file
    el::Configurations conf("easyl.conf");
    el::Loggers::reconfigureLogger("default", conf);

    el::Logger *defaultLogger = el::Loggers::getLogger("default");

    auto start = clock::now();
    for (int i = 0; i < howmany; ++i)
        LOG(INFO) << "easylog message #" << i << ": This is some text for your pleasure";

    duration<float> delta = clock::now() - start;
    float deltaf = delta.count();
    auto rate = howmany / deltaf;

    std::cout << "Total: " << howmany << std::endl;
    std::cout << "Delta = " << deltaf << " seconds" << std::endl;
    std::cout << "Rate = " << rate << "/sec" << std::endl;

    return 0;
}
