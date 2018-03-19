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

#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/Priority.hh"
#include "log4cpp/PatternLayout.hh"

int main(int argc, char * argv[])
{
    using namespace std::chrono;
    using clock = steady_clock;

    int thread_count = 10;
    if (argc > 1)
        thread_count = std::atoi(argv[1]);

    int howmany = 1000000;

    log4cpp::Appender *appender  = new log4cpp::FileAppender("default", "logs/log4cpp-bench-mt.log");
    log4cpp::PatternLayout *layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S.%l}: %p - %m %n");
    appender->setLayout(layout);

    log4cpp::Category& root = log4cpp::Category::getRoot();
    root.addAppender(appender);
    root.setPriority(log4cpp::Priority::INFO);

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
                root << log4cpp::Priority::INFO << "log4cpp message #" << counter << ": This is some text for your pleasure";
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

    root.shutdown();
    return 0;
}
