// example.cpp : Simple logger example
//
#include "spdlog/spdlog.h"
#include "spdlog/sinks/file_sinks.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/null_sink.h"
#include "utils.h"


using namespace std;
using namespace std::chrono;
using namespace spdlog;
using namespace utils;


int main_(int argc, char* argv[])
{
    try {

        using namespace spdlog::sinks;
        spdlog::create<daily_file_sink_st>("mylog", "dailylog", "txt");
        const unsigned int howmany = argc <= 1 ? 1500000 : atoi(argv[1]);

        spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e %l : %t");


        auto console = spdlog::create<sinks::stdout_sink_st>("reporter");
        console->info("Starting bench with", howmany, "iterations..");
        console->log() <<  "Streams are also supprted: " << std::hex << 255;
        spdlog::stop();

        //return 0;
        auto bench = spdlog::create<sinks::rotating_file_sink_st>("bench", "myrotating", "txt", 1024 * 1024 * 1, 10, 0);

        //auto bench = spdlog::create<sinks::simple_file_sink_st>("bench", "simplelog.txt", 1);
        //auto bench = spdlog::create<sinks::null_sink_st>("bench");
        auto start = system_clock::now();
        for (unsigned int i = 0; i < howmany; ++i)
        {
            bench->info("Hello logger: msg number") << i;
        }

        auto delta = system_clock::now() - start;
        auto delta_d = duration_cast<duration<double>> (delta).count();

        cout << "Total:" << format(howmany) << endl;
        cout << "Delta:" << format(delta_d) << endl;
        cout << "Rate:" << format(howmany / delta_d) << "/sec\n";

    }
    catch (std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
        perror("Last error");
    }
    return 0;
}

