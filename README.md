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
* Very fast - performance is the primary goal (see becnhmarks below).
* Headers only.
* No dependencies.
* Cross platform - Linux / Windows on 32/64 bits.
* **new**! Feature rich [formatting](http://cppformat.readthedocs.org/en/stable/syntax.html) and extra performance boost using the excellent [cppformat](http://cppformat.github.io/) under the cover
* stream call style: ```logger.info("Hello") << " logger";
* mixed calls style ```logger.info("Hello {}", name) << 1 << 2 << 3;
* [Custom](https://github.com/gabime/spdlog/wiki/Custom-formatting) formatting.
* Multi/Single threaded loggers.
* Various log targets:
    * Rotating log files.
    * Daily log files.
    * Console logging.
    * Linux syslog.
    * Easily extendable with custom log targets  (just implement a single function in the [sink](include/spdlog/sinks/sink.h) interface).
* Optional, even faster, async logging .
* Log levels.




## Benchmarks

Below are some [benchmarks](bench) comparing the time needed to log 1,000,000 lines to file under Ubuntu 64 bit, Intel i7-4770 CPU @ 3.40GHz:

|threads|boost log|glog|g2log|spdlog|spdlog <sup>async mode</sup>|
|-------|:-------:|:-----:|------:|------:|------:|
|1|4.779s|1.109s|3.155s|0.947s|1.455s
|10|15.151ss|3.546s|3.500s|1.549s|2.040s|




## Usage Example
```c++
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
        console->info("Creating file {}..", filename);

        auto file_logger = spd::rotating_logger_mt("file_logger", filename, 1024 * 1024 * 5, 3);
        file_logger->info("Log file message number", 1);

        for (int i = 0; i < 100; ++i)
        {
            file_logger->info("{} * {} = {}, i, i, i*i );
        }
        
        
        console->info("Easy padding in numbers like {:08d}", 12);
        console->info("int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
        console->info("Positional args are {1} {0}..", "too", "supported): 
        
        console->info("{:<30}", "left aligned");
        console->info("{:>30}", "right aligned");
        console->info("{:^30}", "centered");
        
        //see cppformat's full docs here:
        //http://cppformat.readthedocs.org/en/stable/syntax.html
        
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
