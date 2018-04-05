//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <chrono>
#include <iostream>

#include "glog/logging.h"

int main(int, char *argv[])
{
    using namespace std::chrono;
    using clock = steady_clock;

    int howmany = 1000000;

    FLAGS_logtostderr = 0;
    FLAGS_log_dir = "logs";
    google::InitGoogleLogging(argv[0]);
    auto start = clock::now();
    for (int i = 0; i < howmany; ++i)
        LOG(INFO) << "glog message #" << i << ": This is some text for your pleasure";

    duration<float> delta = clock::now() - start;
    float deltaf = delta.count();
    auto rate = howmany / deltaf;

    std::cout << "Total: " << howmany << std::endl;
    std::cout << "Delta = " << deltaf << " seconds" << std::endl;
    std::cout << "Rate = " << rate << "/sec" << std::endl;

    return 0;
}
