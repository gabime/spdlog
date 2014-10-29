// example.cpp : Simple logger example
//
#include "spitlog/logger.h"
#include "spitlog/sinks/async_sink.h"
#include "spitlog/sinks/file_sinks.h"
#include "spitlog/sinks/stdout_sinks.h"
#include "spitlog/sinks/null_sink.h"
#include "utils.h"
#include "spitlog/details/registry.h"

using namespace std::chrono;
using namespace spitlog;
using namespace utils;


int main(int argc, char* argv[])
{
    try {
        const unsigned int howmany = argc <= 1 ? 1500000 : atoi(argv[1]);

        //spitlog::set_format("%t");
        auto console = spitlog::create<sinks::stdout_sink_st>("reporter");
        //console->set_format("[%n %l] %t");
        console->set_level(spitlog::level::INFO);
        console->info("Starting bench with", howmany, "iterations..");

        auto bench = spitlog::create<sinks::rotating_file_sink_st>("bench", "myrotating", "txt", 1024 * 1024 * 1, 10, 0);

        //auto bench = spitlog::create<sinks::simple_file_sink_st>("bench", "simplelog.txt", 1);
        //auto bench = spitlog::create<sinks::null_sink_st>("bench");
        auto start = system_clock::now();
        for (unsigned int i = 0; i < howmany; ++i)
        {
            bench->info("Hello logger: msg number") << i;
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
        perror("Last error");
    }
    return 0;
}

