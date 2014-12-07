# spdlog

Very fast, header only, C++ logging library.

## Install
Just copy the files to your build tree and use a C++11 compiler

## Tested on:
* gcc 4.8.1 and above
* clang 3.5
* visual studio 2013
* mingw with g++ 4.9.x

##Features
* Very fast - performance is the primary goal (see [becnhmarks](#benchmarks) below).
* Headers only.
* No dependencies.
* Cross platform - Linux / Windows on 32/64 bits.
* **new!** Feature rich cppfromat [call style](http://cppformat.readthedocs.org/en/stable/syntax.html) using the excellent [cppformat](http://cppformat.github.io/) library:
* ```logger.info("Hello {} !!", "world");```
* ostream call style ```logger.info() << "Hello << "logger";```
* Mixed cppformat/ostream call style ```logger.info("{} + {} = ", 1, 2) << "?";```
* **new!** Optional, extremly fast asynchronous mode using lockfree queues, pre allocated memory and deffered formatting.
* [Custom](https://github.com/gabime/spdlog/wiki/Custom-formatting) formatting.
* Multi/Single threaded loggers.
* Various log targets:
    * Rotating log files.
    * Daily log files.
    * Console logging.
    * Linux syslog.
    * Easily extendable with custom log targets  (just implement a single function in the [sink](include/spdlog/sinks/sink.h) interface).
* Log levels.



## Benchmarks

Below are some [benchmarks](bench) comparing the time needed to log 1,000,000 lines to file under Ubuntu 64 bit, Intel i7-4770 CPU @ 3.40GHz (the best of 3 runs for each logger):

|threads|boost log|glog|g2log <sup>async mode</sup>|spdlog|spdlog <sup>async mode</sup>|
|-------|:-------:|:-----:|------:|------:|------:|
|1|4.779s|1.109s|3.155s|0.319s|0.212s
|10|15.151ss|3.546s|3.500s|0.641s|0.199s|


## Usage Example
```c++
#include <iostream>
#include "spdlog/spdlog.h"

int main(int, char* [])
{
    namespace spd = spdlog;
    try
    {
        std::string filename = "logs/spdlog_example";
        // Set log level to all loggers to DEBUG and above
        spd::set_level(spd::level::DEBUG);

        //Create console, multithreaded logger
        auto console = spd::stdout_logger_mt("console");
        console->info("Welcome to spdlog!") ;
        console->info("An info message example {}..", 1);
        console->info() << "Streams are supported too  " << 1;

        console->info("Easy padding in numbers like {:08d}", 12);
        console->info("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
        console->info("Support for floats {:03.2f}", 1.23456);
        console->info("Positional args are {1} {0}..", "too", "supported");

        console->info("{:<30}", "left aligned");
        console->info("{:>30}", "right aligned");
        console->info("{:^30}", "centered");
        
        //See cppformat syntax documation here:
        //http://cppformat.readthedocs.org/en/stable/syntax.html
        
       
        //Create a file rotating logger with 5mb size max and 3 rotated files
        auto file_logger = spd::rotating_logger_mt("file_logger", filename, 1024 * 1024 * 5, 3);
        file_logger->info("Log file message number", 1);

        spd::set_pattern("*** [%H:%M:%S %z] [thread %t] %v ***");
        file_logger->info("This is another message with custom format");

        spd::get("console")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name) function");

        SPDLOG_TRACE(file_logger, "This is a trace message (only #ifdef _DEBUG)", 123);

        //
        // Asynchronous logging is easy..
        // Just call spdlog::set_async_mode(max_q_size) and all created loggers from now on will be asynchronous..
        //
        size_t q_size = 1048576; //queue size must be power of 2
        spdlog::set_async_mode(q_size);
        auto async_file= spd::daily_logger_st("async_file_logger", "logs/async_log.txt");
        async_file->info() << "This is async log.." << "Should be very fast!";

        //
        // syslog example
        //
#ifdef __linux__
        auto syslog_logger = spd::syslog_logger("syslog");
        syslog_logger->warn("This is warning that will end up in syslog. This is Linux only!");
#endif
    }
    catch (const spd::spdlog_ex& ex)
    {
        std::cout << "Log failed: " << ex.what() << std::endl;
    }
}

```
