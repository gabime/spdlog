//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <chrono>
#include <iostream>
#include <memory>

#include "log4cpp/Appender.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/Category.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/PatternLayout.hh"
#include "log4cpp/Priority.hh"

int main(int, char *[])
{
    using namespace std::chrono;
    using clock = steady_clock;

    int howmany = 1000000;

    log4cpp::Appender *appender = new log4cpp::FileAppender("default", "logs/log4cpp-bench.log");
    log4cpp::PatternLayout *layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S.%l}: %p - %m %n");
    appender->setLayout(layout);

    log4cpp::Category &root = log4cpp::Category::getRoot();
    root.addAppender(appender);
    root.setPriority(log4cpp::Priority::INFO);

    auto start = clock::now();
    for (int i = 0; i < howmany; ++i)
        root << log4cpp::Priority::INFO << "log4cpp message #" << i << ": This is some text for your pleasure";

    duration<float> delta = clock::now() - start;
    float deltaf = delta.count();
    auto rate = howmany / deltaf;

    std::cout << "Total: " << howmany << std::endl;
    std::cout << "Delta = " << deltaf << " seconds" << std::endl;
    std::cout << "Rate = " << rate << "/sec" << std::endl;

    root.shutdown();
    return 0;
}
