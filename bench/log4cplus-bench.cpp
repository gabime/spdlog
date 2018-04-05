//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <chrono>
#include <iostream>
#include <memory>

#include "log4cplus/fileappender.h"
#include "log4cplus/helpers/loglog.h"
#include "log4cplus/helpers/property.h"
#include "log4cplus/layout.h"
#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"
#include "log4cplus/ndc.h"

using namespace log4cplus;

int main(int, char *[])
{
    using namespace std::chrono;
    using clock = steady_clock;

    int howmany = 1000000;

    log4cplus::initialize();
    SharedFileAppenderPtr append(new FileAppender(LOG4CPLUS_TEXT("logs/log4cplus-bench.log"), std::ios_base::trunc, true, true));
    append->setName(LOG4CPLUS_TEXT("File"));

    log4cplus::tstring pattern = LOG4CPLUS_TEXT("%d{%Y-%m-%d %H:%M:%S.%Q}: %p - %m %n");
    append->setLayout(std::auto_ptr<Layout>(new PatternLayout(pattern)));
    append->getloc();
    Logger::getRoot().addAppender(SharedAppenderPtr(append.get()));

    Logger root = Logger::getRoot();

    auto start = clock::now();
    for (int i = 0; i < howmany; ++i)
        LOG4CPLUS_INFO(root, "log4cplus message #" << i << ": This is some text for your pleasure");

    duration<float> delta = clock::now() - start;
    float deltaf = delta.count();
    auto rate = howmany / deltaf;

    std::cout << "Total: " << howmany << std::endl;
    std::cout << "Delta = " << deltaf << " seconds" << std::endl;
    std::cout << "Rate = " << rate << "/sec" << std::endl;

    log4cplus::Logger::shutdown();
    return 0;
}
