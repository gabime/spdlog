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
	using namespace std::chrono;
	int nthreads = argc > 1 ? atoi(argv[1]) : 1;
	int nlines = argc > 2 ? atoi(argv[2]) : 1000000;
	
    auto null_sink = std::make_shared<c11log::sinks::null_sink>();
    auto stdout_sink = std::make_shared<c11log::sinks::stdout_sink>();
    auto async = std::make_shared<c11log::sinks::async_sink>(100);
    //auto fsink = std::make_shared<c11log::sinks::rotating_file_sink>("newlog", "txt", 1024*1024*10 , 2);
    auto fsink = std::make_shared<c11log::sinks::daily_file_sink>("daily", "txt");

    async->add_sink(fsink);
    //async->add_sink(null_sink);

    c11log::logger logger("test");
    logger.add_sink(async);
	    
    std::vector<std::thread*> threads;
    std::cout << "Starting " << nthreads << " threads x " << utils::format(nlines) << " lines each.." << std::endl;
    for (int i = 0; i < nthreads; i++)
    {
        auto t = new std::thread([&logger, nlines]() {

		    for(int i = 0 ; i < nlines; ++i)
            {
                logger.info() << "Hello from thread " << std::this_thread::get_id() << "\tcounter: " <<  i ;
            }           

        });
        threads.push_back(t);
    }
    
  
	auto stime = steady_clock::now();
	for(auto t:threads)
		t->join();
	auto delta = steady_clock::now() - stime;
	auto delta_seconds = duration_cast<milliseconds>(delta).count()/1000.0;	 		

    auto total = nthreads*nlines;
    std::cout << "Total: " << utils::format(total) << " = " << utils::format(total/delta_seconds) << "/sec" << std::endl; 
    
    async->shutdown(seconds(1));

    return 0;
}


