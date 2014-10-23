// example.cpp : Simple logger example
//
#include <mutex>
#include "c11log/logger.h"
#include "c11log/factory.h"
#include "c11log/sinks/async_sink.h"
#include "c11log/sinks/file_sinks.h"
#include "c11log/sinks/stdout_sinks.h"
#include "c11log/sinks/null_sink.h"
#include "utils.h"

using namespace std::chrono;
using namespace c11log;
using namespace utils;

std::ostringstream  f1(int i)
{

    std::ostringstream oss;
    oss << "Hello oss " << i;
    return oss;
}

details::fast_oss f2(int i)
{

    details::fast_oss oss;
    oss << "Hello oss " << i;
    return oss;
}

int main(int argc, char* argv[])
{
    const unsigned int howmany = argc <= 1 ? 600000 : atoi(argv[1]);

    const std::string pattern = "%+";
    auto formatter = std::make_shared<details::pattern_formatter>(pattern);
    //logger cout_logger("bench", { std::make_shared<sinks::stderr_sink_mt>() }, pattern);
    //logger::default_formatter(formatter);
    logger cout_logger("bench", { std::make_shared<sinks::stderr_sink_mt>() });
    logger::default_formatter(formatter);
    cout_logger.info() << "Hello logger " << 12.4 << 5 << ',' << 6 << 7 <<8<<9<<10;
    auto nullsink = std::make_shared<sinks::null_sink_st>();
    auto rotating = std::make_shared<sinks::rotating_file_sink_st>("myrotating", "txt", 1024 * 1024 * 5, 20, 100);

    logger my_logger("my_logger", { nullsink });



    auto start = system_clock::now();
    for (unsigned int i = 1; i <= howmany; ++i)
    {
        my_logger.info() << "Hello logger: msg #" << i;
    }

    auto delta = system_clock::now() - start;
    auto delta_d = duration_cast<duration<double>> (delta).count();

    cout_logger.info("Total:") << format(howmany);
    cout_logger.info("Delta:") << format(delta_d);
    cout_logger.info("Rate:") << format(howmany / delta_d) << "/sec";

    return 0;
}

