// example.cpp : Simple logger example
//

#include "c11log/logger.h"
#include "c11log/sinks/async_sink.h"
#include "c11log/sinks/file_sinks.h"
#include "c11log/sinks/console_sinks.h"

#include "utils.h"

using std::cout;
using std::endl;
using namespace std::chrono;
using namespace c11log;
using namespace utils;


int main(int argc, char* argv[])
{

    if(argc || argv) {};
    const unsigned int howmany = argc <= 1 ? 1000 :atoi(argv[1]);

    auto fsink = std::make_shared<sinks::rotating_file_sink>("log", "txt", 1024*1024*50 , 5, 0);
    auto null_sink = std::make_shared<sinks::null_sink>();


    logger cout_logger ("cout", {sinks::stdout_sink()});

    logger my_logger ("my_logger", {null_sink});

    auto start = system_clock::now();
    for(unsigned int i = 0; i < howmany ; i++)
        my_logger.info("Hello logger");


    auto delta = system_clock::now() - start;
    auto delta_d = duration_cast<duration<double>> (delta).count();
    cout_logger.info("Total ") << format(howmany);
    cout_logger.info("Delta ") << format(delta_d);
    cout_logger.info("Rate: ") << format(howmany/delta_d) << "/sec";


    return 0;

    /*
    if(argc !=3) {
        std::cerr << "Usage: " << argv[0] << " qsize, threads" << std::endl;
        return 0;
    }
    int qsize = atoi(argv[1]);
    int threads = atoi(argv[2]);


    auto null_sink = std::make_shared<sinks::null_sink>();
    auto stdout_sink = std::make_shared<sinks::stdout_sink>();
    auto async = std::make_shared<sinks::async_sink>(qsize);
    auto fsink = std::make_shared<sinks::rotating_file_sink>("log", "txt", 1024*1024*50 , 5,  std::chrono::milliseconds(1000));

    async->add_sink(fsink);

    //auto &logger = c11log::get_logger("async");
    //logger.add_sink(fsink);



    testlog(threads);
    */
}

