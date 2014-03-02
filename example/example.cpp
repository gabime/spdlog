// example.cpp : Simple logger example
//
#include <string>
#include <functional>
#include <iomanip>
#include "c11log/logger.h"
#include "c11log/sinks/async_sink.h"
#include "c11log/sinks/file_sinks.h"
#include "c11log/sinks/stdout_sinks.h"

#include "utils.h"

std::atomic<uint64_t> log_count;
std::atomic<bool> active;

using std::chrono::seconds;

void logging_thread()
{
    auto &logger = c11log::get_logger("async");
    while(active) {
        logger.info()<<"Hello logger!";
        ++log_count;
    }
}


void testlog(int threads)
{

    active = true;

    for(int i = 0; i < threads; i++)
        new std::thread(std::bind(logging_thread));

    while(active) {
        using std::endl;
        using std::cout;
        using utils::format;

        log_count = 0;
        std::this_thread::sleep_for(seconds(1));
        cout << "Logs/sec =\t" << format(log_count.load()) << endl;
    }
}


int main(int argc, char* argv[])
{

    using namespace std::chrono;
    using namespace c11log;
    using namespace utils;
    using std::cout;
    using std::endl;

    //auto fsink2 = std::make_shared<sinks::rotating_file_sink>("log", "txt", 1024*1024*50 , 5, seconds(1));
    auto &logger2 = c11log::get_logger("logger2");
    //logger2.add_sink(fsink2);
    logger2.add_sink(std::make_shared<sinks::null_sink>());
	logger2.add_sink(std::make_shared<sinks::stdout_sink>());

	info_logger info(&logger2);
	info << "Hello info logger" << "!!";
	return 0;
    auto start = system_clock::now();
    const unsigned int howmany = 10000000;
    for(unsigned int i = 0; i < howmany ; i++)
        logger2.info() << "Hello logger " << i;

    auto delta = system_clock::now() - start;
    auto delta_d = duration_cast<duration<double>> (delta);
    cout << "Total " << format(howmany) << endl;
    cout << "Delta " << format(delta_d.count()) << endl;
    cout << "Rate: " << format(howmany/delta_d.count()) << "/sec" << endl;


    return 0;


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
}

