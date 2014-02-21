// example.cpp : Simple logger example
//
#include <string>
#include <functional>

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

    if(argc !=3) {
        std::cerr << "Usage: " << argv[0] << " qsize, threads" << std::endl;
        return 0;
    }
    int qsize = atoi(argv[1]);
    int threads = atoi(argv[2]);

    using namespace c11log::sinks;
    auto null_sink = std::make_shared<null_sink>();
    auto stdout_sink = std::make_shared<stdout_sink>();
    auto async = std::make_shared<async_sink>(qsize);
    auto fsink = std::make_shared<rotating_file_sink>("example_log", "txt", 1024*1024*50 , 5);

    async->add_sink(fsink);

    auto &logger = c11log::get_logger("async");
    logger.add_sink(async);

    testlog(threads);
}

