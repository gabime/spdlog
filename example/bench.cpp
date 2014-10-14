// example.cpp : Simple logger example
//
#include <mutex>
#include "c11log/logger.h"
#include "c11log/pattern_formatter.h"
#include "c11log/sinks/async_sink.h"
#include "c11log/sinks/file_sinks.h"
#include "c11log/sinks/stdout_sinks.h"
#include "c11log/sinks/null_sink.h"
#include "utils.h"

using std::cout;
using std::endl;
using namespace std::chrono;
using namespace c11log;
using namespace utils;


int main(int argc, char* argv[])
{
    const unsigned int howmany = argc <= 1 ? 500000 : atoi(argv[1]);

    std::string pattern = "%Y:%m:%d %H:%M:%S.%e [%n:%l] %t";
    auto formatter1 = std::unique_ptr<formatter>(new pattern_formatter(pattern));

    logger cout_logger("bench",  std::make_shared<sinks::stderr_sink_mt>() , std::move(formatter1));
    cout_logger.info() << "Hello logger";

    auto nullsink = std::make_shared<sinks::null_sink<details::null_mutex>>();
    auto rotating = std::make_shared<sinks::rotating_file_sink_mt>("myrotating", "txt", 1024 * 1024 * 5, 5, 100);


    auto formatter2 = std::unique_ptr<formatter>(new pattern_formatter(pattern));
    logger my_logger("my_logger", rotating, std::move(formatter2));

    auto start = system_clock::now();
    for (unsigned int i = 1; i <= howmany; ++i)
        my_logger.info() << "Hello logger: msg #" << i;



    auto delta = system_clock::now() - start;
    auto delta_d = duration_cast<duration<double>> (delta).count();

    cout << "Total:" << format(howmany) << endl;
    cout << "Delta:" << format(delta_d) << endl;
    cout << "Rate:" << format(howmany / delta_d) << "/sec" << endl;

    return 0;
}

