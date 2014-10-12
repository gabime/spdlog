// example.cpp : Simple logger example
//
#define FFLOG_ENABLE_TRACE

#include <iostream>
#include "c11log/logger.h"
#include "c11log/factory.h"
#include "c11log/sinks/stdout_sinks.h"
#include "c11log/sinks/file_sinks.h"
using namespace std;

int main(int argc, char* argv[])
{

    auto console = c11log::factory::stdout_logger();
    auto file = c11log::factory::simple_file_logger("log.txt");
    auto rotating= c11log::factory::rotating_file_logger("myrotating", "txt", 1024*1024*5, 5, 1);
    auto daily = c11log::factory::daily_file_logger("dailylog", "txt", 1, "daily_logger");


    //console->info() << "This is variadic ", " func, ", 123 << " YES";
    FFLOG_TRACE(console, "This is ", 1);


    file->info("Hello file log");
    rotating->info("Hello rotating log");
    daily->info("Hello daily log");

    //multi sink logger: file + console
    auto sink1= std::make_shared<c11log::sinks::stdout_sink_mt>();
    auto sink2 = std::make_shared<c11log::sinks::daily_file_sink_mt>("rotating", "txt");
    c11log::logger combined("combined", { sink1, sink2 });

}

