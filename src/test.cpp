// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <functional>

#include "c11log/logger.h"
#include "c11log/sinks/async_sink.h"
#include "c11log/sinks/file_sinks.h"
#include "c11log/sinks/stdout_sinks.h"

#include "utils.h"


std::atomic<uint64_t> push_count, pop_count;
std::atomic<bool> active;

using Q = c11log::details::blocking_queue<std::string>;
using std::chrono::seconds;

void pusher(Q* q)
{
	while(active)
	{
		//if(q->push("Hello", seconds(10)))
		q->push("hello");
		++push_count;
	}

}
void popper(Q* q) 
{
	std::string output;
	while(active)
	{
		//if(q->pop(output, seconds(10)))
		q->pop(output);
		++pop_count;
	}
}

void testq(int size, int pushers, int poppers)
{

	active = true;			
	Q q{static_cast<Q::size_type>(size)};		

	for(int i = 0; i < poppers; i++)
		new std::thread(std::bind(popper, &q));
		
	for(int i = 0; i < pushers; i++)
		new std::thread(std::bind(pusher, &q));
	
	

	while(active)
	{
		using std::endl;
		using std::cout;
		using utils::format;
		
		push_count = 0;
		pop_count = 0;
		std::this_thread::sleep_for(seconds(1));
		cout << "Pushes/sec =\t" << format(push_count.load()) << endl;
		cout << "Pops/sec =\t" << format(pop_count.load()) << endl;
		cout << "Total/sec =\t" << format(push_count+pop_count) << endl << endl;
		cout << "Queue size =\t" << format(q.size()) << endl;
		cout << "---------------------------------------------------------------------" << endl;		
	}
		

}
int main(int argc, char* argv[])
{
	if(argc !=4)
	{
		std::cerr << "Usage: " << argv[0] << " qsize, pushers, poppers" << std::endl;
		return 0;
	}
	int qsize = atoi(argv[1]);
	int pushers = atoi(argv[2]);
	int poppers = atoi(argv[3]);
	
	testq(qsize, pushers, poppers);
	
	/*
	using namespace std::chrono;
	
	
	int nthreads = argc > 1 ? atoi(argv[1]) : 1;
	int nlines = argc > 2 ? atoi(argv[2]) : 100000;
	
		
				
    auto null_sink = std::make_shared<c11log::sinks::null_sink>();
    auto stdout_sink = std::make_shared<c11log::sinks::stdout_sink>();
    auto async = std::make_shared<c11log::sinks::async_sink>(1000);
    //auto fsink = std::make_shared<c11log::sinks::rotating_file_sink>("newlog", "txt", 1024*1024*10 , 2);
    //auto fsink = std::make_shared<c11log::sinks::daily_file_sink>("daily", "txt");

	//async->add_sink(fsink);
    async->add_sink(null_sink);

    //console logger
    auto &console = c11log::get_logger("console");
    console.add_sink(stdout_sink);
	//c11log::details::blocking_queue<std::string> q(1000);
	//auto q_ptr = &q;
    std::vector<std::thread*> threads;
    std::cout << "Starting " << nthreads << " threads x " << utils::format(nlines) << " lines each.." << std::endl;
    for (int i = 0; i < nthreads; i++)
    {    	
    	auto logger = std::make_shared<c11log::logger>("test");
	    logger->add_sink(async);
	    
        auto t = new std::thread([logger, nlines, i]() {
			auto &console = c11log::get_logger("console");
		    for(int j = 0 ; j < nlines; ++j)
            {            	
                logger->info() << "Hello from thread #" << i << "\tcounter: " <<  j ;
                if(j % 2000 == 0)
                	console.info() << "Hello from thread " << i << "\tcounter: " <<  j;                                
            }
        });
        threads.push_back(t);
        //std::this_thread::sleep_for(milliseconds(2));
    }
    
  
	auto stime = steady_clock::now();
	int thread_joined = 0;
	for(auto t:threads)
	{
		t->join();
		std::cout << "Joined " << ++thread_joined << " threads" << std::endl;
	}
	
	auto delta = steady_clock::now() - stime;
	auto delta_seconds = duration_cast<milliseconds>(delta).count()/1000.0;	 		

    auto total = nthreads*nlines;
    std::cout << "Total: " << utils::format(total) << " = " << utils::format(total/delta_seconds) << "/sec" << std::endl; 
    
    async->shutdown(seconds(1));
    */
}


