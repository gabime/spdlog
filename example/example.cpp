// example.cpp : Simple logger example
//
#include <string>
#include <functional>
#include "c11log/logger.h"
#include "c11log/sinks/async_sink.h"
#include "c11log/sinks/file_sinks.h"
#include "c11log/sinks/console_sinks.h"
#include "c11log/details/log_msg.h"

#include "utils.h"

using std::cout;
using std::endl;
using namespace std::chrono;
using namespace c11log;
using namespace utils;

int main(int argc, char* argv[])
{

    if(argc || argv) {};

    auto fsink = std::make_shared<sinks::rotating_file_sink>("log", "txt", 1024*1024*50 , 5, 0);
    //auto fsink = std::make_shared<sinks::simple_file_sink>("simplelog", "txt");
    auto null_sink = std::make_shared<sinks::null_sink>();


    logger cout_logger ("cout", {null_sink, sinks::stdout_sink()});

    cout_logger.info() << "Hello cout logger!";

    logger my_logger ("my_logger", {null_sink});

    std::string s(100, '0');
    const unsigned int howmany = 5000000;
    auto start = system_clock::now();
    for(unsigned int i = 0; i < howmany ; i++)
        my_logger.info() << s;
    //my_logger.info() << "Hello logger " << i;;

    //async->shutdown(seconds(3));
    auto delta = system_clock::now() - start;
    auto delta_d = duration_cast<duration<double>> (delta);
    cout << "Total " << format(howmany) << endl;
    cout << "Delta " << format(delta_d.count()) << endl;
    cout << "Rate: " << format(howmany/delta_d.count()) << "/sec" << endl;


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

