// example.cpp : Simple logger example
//
#include "c11log/logger.h"
#include "c11log/sinks/async_sink.h"
#include "c11log/sinks/file_sinks.h"
#include "c11log/sinks/stdout_sinks.h"
#include "c11log/sinks/null_sink.h"
#include "utils.h"
#include "c11log/details/registry.h"

using namespace std::chrono;
using namespace c11log;
using namespace utils;


int main(int argc, char* argv[])
{
    try {
        const unsigned int howmany = argc <= 1 ? 500000 : atoi(argv[1]);

        //c11log::set_format("%t");
        auto console = c11log::create<sinks::stdout_sink_st>("reporter");
        //console->set_format("[%n %l] %t");
        console->set_level(c11log::level::INFO);
        console->info("Starting bench with", howmany, "iterations..");

        //auto bench = c11log::create<sinks::rotating_file_sink_st>("bench", "myrotating", "txt", 1024 * 1024 * 5, 3, 0);
        auto bench = c11log::create<sinks::daily_file_sink_mt>("bench", "sdfsfddaily", "txt", 0);
        //auto bench = c11log::create<sinks::simple_file_sink_st>("bench", "simplelog.txt", 1);
        //auto bench = c11log::create<sinks::null_sink_st>("bench");
        auto start = system_clock::now();
        for (unsigned int i = 1; i <= howmany; ++i)
        {
            c11log::get("bench")->info("Hello logger: msg number", i);
        }

        auto delta = system_clock::now() - start;
        auto delta_d = duration_cast<duration<double>> (delta).count();

        console->info("Total:") << format(howmany);
        console->info("Delta:") << format(delta_d);
        console->info("Rate:") << format(howmany / delta_d) << "/sec";

    }
    catch (std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
    return 0;
}

