// example.cpp : Simple logger example
//
#include <mutex>
#include "c11log/logger.h"
#include "c11log/sinks/async_sink.h"
#include "c11log/sinks/file_sinks.h"
#include "c11log/sinks/stdout_sinks.h"
#include "c11log/sinks/null_sink.h"
#include "utils.h"

using namespace std::chrono;
using namespace c11log;
using namespace utils;


int main(int argc, char* argv[])
{
    const unsigned int howmany = argc <= 1 ? 500000 : atoi(argv[1]);

    //std::string pattern = "%B %d, %Y %H:%M:%S.%e **************[%n:%l] %t";
    std::string pattern = " [%z] %t";
    auto formatter = std::make_shared<details::pattern_formatter>(pattern);

    logger cout_logger("bench", { std::make_shared<sinks::stderr_sink_mt>() });
    cout_logger.formatter(formatter);
    cout_logger.info() << "Hello logger " << 1234;

    auto nullsink = std::make_shared<sinks::null_sink_st>();
    auto rotating = std::make_shared<sinks::rotating_file_sink_mt>("myrotating", "txt", 1024 * 1024 * 5, 5, 100);

    logger my_logger("my_logger", { nullsink }, formatter);

    auto start = system_clock::now();
    for (unsigned int i = 1; i <= howmany; ++i)
        my_logger.info() << "Hello logger: msg #" << i;

    auto delta = system_clock::now() - start;
    auto delta_d = duration_cast<duration<double>> (delta).count();

    cout_logger.info("Total:") << format(howmany);
    cout_logger.info("Delta:") << format(delta_d);
    cout_logger.info("Rate:") << format(howmany / delta_d) << "/sec";

    return 0;
}

