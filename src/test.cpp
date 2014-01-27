// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "c11log/logger.h"
#include "c11log/sinks/async_sink.h"
#include "c11log/sinks/file_sinks.h"
#include "c11log/sinks/stdout_sinks.h"

#include "utils.h"



int main(int argc, char* argv[])
{
	int nthreads = argc > 1 ? atoi(argv[1]) : 1;
    auto null_sink = std::make_shared<c11log::sinks::null_sink>();
    auto stdout_sink = std::make_shared<c11log::sinks::stdout_sink>();
    auto async = std::make_shared<c11log::sinks::async_sink>(100);
    //auto fsink = std::make_shared<c11log::sinks::rotating_file_sink>("newlog", "txt", 1024*1024*10 , 2);
    auto fsink = std::make_shared<c11log::sinks::midnight_file_sink>("midnight", "txt");

    async->add_sink(fsink);

    c11log::logger logger("test");
    logger.add_sink(async);
	
    std::atomic<uint32_t> counter { 0 };
    auto counter_ptr = &counter;
    std::cout << "Starting " << nthreads << " threads.." << std::endl;
    for (int i = 0; i < nthreads; i++)
    {
        new std::thread([&logger, counter_ptr]() {
            while (true)
            {
                logger.info() << "Hello from thread " << std::this_thread::get_id() << "\tcounter: " << counter_ptr->load();
                (*counter_ptr)++;
            }

        });
    }
    while (true)
    {
        counter = 0;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Counter = " << utils::format(counter.load()) << std::endl;
    }
    async->shutdown(std::chrono::seconds(10));
    return 0;
}


