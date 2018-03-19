//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "log4cplus/logger.h"
#include "log4cplus/fileappender.h"
#include "log4cplus/layout.h"
#include "log4cplus/ndc.h"
#include "log4cplus/helpers/loglog.h"
#include "log4cplus/helpers/property.h"
#include "log4cplus/loggingmacros.h"

using namespace log4cplus;

int main(int argc, char * argv[])
{
    using namespace std::chrono;
    using clock = steady_clock;

    int thread_count = 10;
    if (argc > 1)
        thread_count = std::atoi(argv[1]);

    int howmany = 1000000;

    log4cplus::initialize();
    SharedFileAppenderPtr append(
        new FileAppender(LOG4CPLUS_TEXT("logs/log4cplus-bench-mt.log"), std::ios_base::trunc,
            true, true));
    append->setName(LOG4CPLUS_TEXT("File"));

    log4cplus::tstring pattern = LOG4CPLUS_TEXT("%d{%Y-%m-%d %H:%M:%S.%Q}: %p - %m %n");
    append->setLayout( std::auto_ptr<Layout>(new PatternLayout(pattern)) );
    append->getloc();
    Logger::getRoot().addAppender(SharedAppenderPtr(append.get()));

    Logger root = Logger::getRoot();

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
                LOG4CPLUS_INFO(root, "log4cplus message #" << counter << ": This is some text for your pleasure");
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
    std::cout << "Delta = " << deltaf << " seconds" << std::endl;
    std::cout << "Rate = " << rate << "/sec" << std::endl;

    log4cplus::Logger::shutdown();
    return 0;
}
