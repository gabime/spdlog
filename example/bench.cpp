//
// bench.cpp : spdlog benchmarks
//
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/file_sinks.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/null_sink.h"
#include "utils.h"


using namespace std;
using namespace std::chrono;
using namespace spdlog;
using namespace spdlog::sinks;
using namespace utils;


void bench(int howmany, std::shared_ptr<spdlog::logger> log)
{
		cout << log->name() << ", " << format(howmany) << " iterations.." << endl;
      
        auto start = system_clock::now();
        for (auto i = 0; i < howmany; ++i)
        {
            log->info("Hello logger: msg number ") << i;
        }

        auto delta = system_clock::now() - start;
        auto delta_d = duration_cast<duration<double>> (delta).count();		
        cout << "Delta:" << format(delta_d) << " seconds" << endl;
        cout << "Rate:" << format(howmany / delta_d) << "/sec" << endl << endl;
}


void bench_mt(int howmany, std::shared_ptr<spdlog::logger> log, int thread_count)
{

        cout << log->name() << ", " << format(howmany) << " iterations.." << endl;
        std::atomic<int > msg_counter{0};   
      	vector<thread> threads;
		auto start = system_clock::now();
		for (int t = 0; t < thread_count; ++t)
		{
		    threads.push_back(std::thread([&]() {
				while(msg_counter++ < howmany)
			        log->info("Hello logger: msg number ") << msg_counter;
	
			}));
	    }        
	    
		for(auto &t:threads) 
    	{
    	    t.join();
	    };

        auto delta = system_clock::now() - start;
        auto delta_d = duration_cast<duration<double>> (delta).count();		
        cout << "Delta:" << format(delta_d) << " seconds" << endl;
        cout << "Rate:" << format(howmany / delta_d) << "/sec" << endl << endl;
 }


int main(int argc, char* argv[])
{
	

    try {
    	
		int howmany = argc <= 1 ? 100000 : atoi(argv[1]);
		int threads = argc <= 2 ? 4 : atoi(argv[2]);
		int flush_interval = 100;

		
		cout << "*******************************************************************************\n";	
		cout << "Single threaded benchmarks. flush_interval = " << flush_interval << endl;        		
		cout << "*******************************************************************************\n";	
	
		auto rotating_st = spdlog::rotating_logger_st("rotating_st", "logs/rotating_st", 1024 * 1024 * 5, 5, flush_interval);
		bench(howmany, rotating_st);        				
		
		auto daily_st = spdlog::daily_logger_st("daily_st", "logs/daily_st", flush_interval);
		bench(howmany, daily_st);				
		
		bench(howmany, spdlog::create<null_sink_st>("null_st"));        		
		
		cout << "*******************************************************************************\n";	
		cout << "Multi threaded benchmarks (" << threads << " threads), flush_interval = " << flush_interval << endl;        		
		cout << "*******************************************************************************\n";	
	
		auto rotating_mt = spdlog::rotating_logger_mt("rotating_mt", "logs/rotating_mt", 1024 * 1024 * 5, 5, flush_interval);
		bench_mt(howmany, rotating_mt, threads);        				
		
		auto daily_mt = spdlog::daily_logger_mt("daily_mt", "logs/daily_mt", flush_interval);
		bench_mt(howmany, daily_mt, threads);				
		
		bench_mt(howmany, spdlog::create<null_sink_mt>("null_mt"), threads);
    }
    catch (std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        perror("Last error");
    }
    return 0;
}

