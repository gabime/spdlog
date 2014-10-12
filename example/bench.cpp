// example.cpp : Simple logger example
//
#include <mutex>
#include "c11log/logger.h"
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


int main2(int argc, char* argv[])
{

    const unsigned int howmany = argc <= 1 ? 1000000 : atoi(argv[1]);

    logger cout_logger("example", std::make_shared<sinks::stderr_sink_mt>());
    cout_logger.info() << "Hello logger";

    auto nullsink = std::make_shared<sinks::null_sink<details::null_mutex>>();




    logger my_logger("my_logger", nullsink);


    auto start = system_clock::now();
    for (unsigned int i = 1; i <= howmany; ++i)
        my_logger.info() << "Hello logger: msg #" << i << 1<<2<<3<<4<<5<<6<<7<<8<<9<<10<<11<<12<<13<<14<<15<<16<<17<<18<<19;
    //my_logger.info("Hello logger: msg #",i,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19);


    auto delta = system_clock::now() - start;
    auto delta_d = duration_cast<duration<double>> (delta).count();

    cout << "Total:" << format(howmany) << endl;
    cout << "Delta:" << format(delta_d) << endl;
    cout << "Rate:" << format(howmany / delta_d) << "/sec" << endl;

    return 0;
}

