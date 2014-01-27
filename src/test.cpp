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
    auto async = std::make_shared<c11log::sinks::async_sink>(1000);
    //auto fsink = std::make_shared<c11log::sinks::rotating_file_sink>("newlog", "txt", 1024*1024*10 , 2);
    auto fsink = std::make_shared<c11log::sinks::daily_file_sink>("daily", "txt");

    async->add_sink(fsink);

    c11log::logger logger("test");
    logger.add_sink(async);
	
    std::atomic<uint32_t> counter { 0 };
    auto counter_ptr = &counter;    
    std::atomic<bool> active{true};
    auto active_ptr = &active;
    
    std::vector<std::thread*> threads;
    std::cout << "Starting " << nthreads << " threads for 3 seconds.." << std::endl;
    for (int i = 0; i < nthreads; i++)
    {
        auto t = new std::thread([&logger, counter_ptr, active_ptr]() {
            while (*active_ptr)
            {
                logger.info() << "Hello from thread " << std::this_thread::get_id() << "\tcounter: " << counter_ptr->load();
                (*counter_ptr)++;
            }           

        });
        threads.push_back(t);
    }
    
    int seconds = 0;
    while (seconds++ < 3)
    {
        counter = 0;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Counter = " << utils::format(counter.load()) << std::endl;
    }
	active = false;
	for(auto t:threads)
		t->join();
    async->shutdown(std::chrono::seconds(1));

    return 0;
}


