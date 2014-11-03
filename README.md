# spdlog

Very fast, header only, C++ logging library.


## Install
Just copy the files to your build tree and use a C++11 compiler


## Tested on:
* gcc 4.8.1 and above
* clang 3.5
* visual studio 2013

##Features
* Very fast - performance is the primary goal
* Headers only
* No dependencies
* Cross platform - Linux / Windows on 32/64 bits
* Mult/Single threaded loggers
* Rotating log files
* Daily log files
* Console logging
* Optional async logging 
* Logging levels
* Custom formatting with user defined patterns

## Benchmarks
Here are some benchmarks  (Ubuntu 64 bits, Intel i7-4770 CPU @ 3.40GHz)
```
*******************************************************************************
Single thread, 250,000 iterations, flush every 1000 lines
*******************************************************************************
rotating_st...	817,860 lines/sec
daily_st...		827,820 lines /sec

*******************************************************************************
4 threads sharing same logger, 250,000 iterations, flush every 1000 lines
*******************************************************************************
rotating_mt...	1,476,013 lines/sec
daily_mt...		1,477,619 lines/sec
```

## Usage Example
```
#include <iostream>
#include "spdlog/spdlog.h"

int main(int, char* [])
{
    namespace spd = spdlog;

    try
    {
        std::string filename = "spdlog_example";
        auto console = spd::stdout_logger_mt("console");
        console->info("Welcome to spdlog!") ;
        console->info() << "Creating file " << filename << "..";

        auto file_logger = spd::rotating_logger_mt("file_logger", filename, 1024 * 1024 * 5, 3);
        file_logger->info("Log file message number", 1);

        for (int i = 0; i < 100; ++i)
        {
            auto square = i*i;
            file_logger->info() << i << '*' << i << '=' << square << " (" << "0x" << std::hex << square << ")";
        }

        // Change log level to all loggers to warning and above
        spd::set_level(spd::level::WARN);
        console->info("This should not be displayed");
        console->warn("This should!");
        spd::set_level(spd::level::INFO);

        // Change format pattern to all loggers
        spd::set_pattern(" **** %Y-%m-%d %H:%M:%S.%e %l **** %v");
        spd::get("console")->info("This is another message with different format");
    }
    catch (const spd::spdlog_ex& ex)
    {
        std::cout << "Log failed: " << ex.what() << std::endl;
    }
    return 0;
}
```
