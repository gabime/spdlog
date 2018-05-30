# spdlog

Very fast, header only, C++ logging library. [![Build Status](https://travis-ci.org/gabime/spdlog.svg?branch=master)](https://travis-ci.org/gabime/spdlog)&nbsp; [![Build status](https://ci.appveyor.com/api/projects/status/d2jnxclg20vd0o50?svg=true)](https://ci.appveyor.com/project/gabime/spdlog)


## Install
#### Just copy the headers:

* Copy the source [folder](https://github.com/gabime/spdlog/tree/master/include/spdlog) to your build tree and use a C++11 compiler.

#### Or use your favorite package manager:

* Ubuntu: `apt-get install libspdlog-dev`
* Homebrew: `brew install spdlog`
* FreeBSD:  `cd /usr/ports/devel/spdlog/ && make install clean`
* Fedora: `yum install spdlog`
* Gentoo: `emerge dev-libs/spdlog`
* Arch Linux: `yaourt -S spdlog-git`
* vcpkg: `vcpkg install spdlog`
 

## Platforms
 * Linux, FreeBSD, Solaris, AIX
 * Windows (vc 2013+, cygwin)
 * Mac OSX (clang 3.5+)
 * Android

## Features
* Very fast - performance is the primary goal (see [benchmarks](#benchmarks) below).
* Headers only, just copy and use.
* Feature rich [call style](#usage-example) using the excellent [fmt](https://github.com/fmtlib/fmt) library.
* Optional printf syntax support.
* Asynchronous mode (optional)
* [Custom](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting) formatting.
* Conditional Logging
* Multi/Single threaded loggers.
* Various log targets:
    * Rotating log files.
    * Daily log files.
    * Console logging (colors supported).
    * syslog.
    * Windows debugger (```OutputDebugString(..)```)
    * Easily extendable with custom log targets  (just implement a single function in the [sink](include/spdlog/sinks/sink.h) interface).
* Severity based filtering - threshold levels can be modified in runtime as well as in compile time.



## Benchmarks

Below are some [benchmarks](bench) comparing popular log libraries under Ubuntu 64 bit, Intel i7-4770 CPU @ 3.40GHz

#### Synchronous mode
Time needed to log 1,000,000 lines in synchronous mode (in seconds, the best of 3 runs):

|threads|boost log 1.54|glog   |easylogging |spdlog|
|-------|:-------:|:-----:|----------:|------:|
|1|       4.169s  |1.066s |0.975s     |0.392s|
|10|     6.180s   |3.032s |2.857s     |0.773s|
|100|     5.981s  |1.139s |4.512s     |0.587s|


#### Asynchronous mode
Time needed to log 1,000,000 lines in asynchronous mode, i.e. the time it takes to put them in the async queue (in seconds, the best of 3 runs):

|threads|g2log <sup>async logger</sup>   |spdlog <sup>async mode</sup>|
|:-------|:-----:|-------------------------:|
|1|       1.850s |0.39s |
|10|      0.943s  |0.416s|
|100|      0.959s |0.413s|




## Usage Example
```c++

#include "spdlog/spdlog.h"

#include <iostream>
#include <memory>

void async_example();
void syslog_example();
void user_defined_example();
void err_handler_example();

namespace spd = spdlog;
int main(int, char*[])
{
    try
    {
        // Console logger with color
        auto console = spd::stdout_color_mt("console");
        console->info("Welcome to spdlog!");
        console->error("Some error message with arg{}..", 1);
	
        // Formatting examples
        console->warn("Easy padding in numbers like {:08d}", 12);
        console->critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
        console->info("Support for floats {:03.2f}", 1.23456);
        console->info("Positional args are {1} {0}..", "too", "supported");
        console->info("{:<30}", "left aligned");
	
	// Use global registry to retrieve loggers
        spd::get("console")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name) function");
        
        // Create basic file logger (not rotated)
        auto my_logger = spd::basic_logger_mt("basic_logger", "logs/basic.txt");
        my_logger->info("Some log message");

        // Create a file rotating logger with 5mb size max and 3 rotated files
        auto rotating_logger = spd::rotating_logger_mt("some_logger_name", "logs/mylogfile.txt", 1048576 * 5, 3);
        for (int i = 0; i < 10; ++i)
            rotating_logger->info("{} * {} equals {:>10}", i, i, i*i);

        // Create a daily logger - a new file is created every day on 2:30am
        auto daily_logger = spd::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);
        // trigger flush if the log severity is error or higher
        daily_logger->flush_on(spd::level::err);
        daily_logger->info(123.44);

        // Customize msg format for all messages
        spd::set_pattern("[%^+++%$] [%H:%M:%S %z] [thread %t] %v");
        console->info("This an info message with custom format (and custom color range between the '%^' and '%$')");
	console->error("This an error message with custom format (and custom color range between the '%^' and '%$')");

        // Runtime log levels
	spd::set_level(spd::level::info); //Set global log level to info
	console->debug("This message should not be displayed!");
	console->set_level(spd::level::debug); // Set specific logger's log level
	console->debug("This message should be displayed..");

        // Compile time log levels
        // define SPDLOG_DEBUG_ON or SPDLOG_TRACE_ON
        SPDLOG_TRACE(console, "Enabled only #ifdef SPDLOG_TRACE_ON..{} ,{}", 1, 3.23);
        SPDLOG_DEBUG(console, "Enabled only #ifdef SPDLOG_DEBUG_ON.. {} ,{}", 1, 3.23);

        // Asynchronous logging is very fast..
        // Just call spdlog::set_async_mode(q_size) and all created loggers from now on will be asynchronous..
        async_example();

        // syslog example. linux/osx only
        syslog_example();

        // android example. compile with NDK
        android_example();

        // Log user-defined types example
        user_defined_example();

        // Change default log error handler
        err_handler_example();

        // Apply a function on all registered loggers
        spd::apply_all([&](std::shared_ptr<spd::logger> l)
        {
            l->info("End of example.");
        });

        // Release and close all loggers
        spd::drop_all();
    }
    // Exceptions will only be thrown upon failed logger or sink construction (not during logging)
    catch (const spd::spdlog_ex& ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
        return 1;
    }
}

void async_example()
{
    size_t q_size = 4096; 
    spd::set_async_mode(q_size);
    auto async_file = spd::daily_logger_st("async_file_logger", "logs/async_log.txt");
    for (int i = 0; i < 100; ++i)
        async_file->info("Async message #{}", i);
}

//syslog example
void syslog_example()
{
#ifdef SPDLOG_ENABLE_SYSLOG 
    std::string ident = "spdlog-example";
    auto syslog_logger = spd::syslog_logger("syslog", ident, LOG_PID);
    syslog_logger->warn("This is warning that will end up in syslog..");
#endif
}

// user defined types logging by implementing operator<<
struct my_type
{
    int i;
    template<typename OStream>
    friend OStream& operator<<(OStream& os, const my_type &c)
    {
        return os << "[my_type i="<<c.i << "]";
    }
};

#include <spdlog/fmt/ostr.h> // must be included
void user_defined_example()
{
    spd::get("console")->info("user defined type: {}", my_type { 14 });
}

//
//custom error handler
//
void err_handler_example()
{	
	spd::set_error_handler([](const std::string& msg) {
		std::cerr << "my err handler: " << msg << std::endl;
	}); 
	// (or logger->set_error_handler(..) to set for specific logger)
}

```

## Documentation
Documentation can be found in the [wiki](https://github.com/gabime/spdlog/wiki/1.-QuickStart) pages.
