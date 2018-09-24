//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
//
// spdlog usage example
//
//

#include <iostream>

void stdout_example();
void basic_example();
void rotating_example();
void daily_example();
void async_example();
void binary_example();
void multi_sink_example();
void user_defined_example();
void err_handler_example();
void syslog_example();
void clone_example();

#include "spdlog/spdlog.h"

int main(int, char *[])
{

    try
    {
        // console logging example
        stdout_example();

        // various file loggers
        basic_example();
        rotating_example();
        daily_example();

        clone_example();

        // async logging using a backing thread pool
        async_example();

        // log binary data
        binary_example();

        // a logger can have multiple targets with different formats
        multi_sink_example();

        // user defined types logging by implementing operator<<
        user_defined_example();

        // custom  error handler
        err_handler_example();

        // flush all *registered* loggers using a worker thread every 3 seconds.
        // note: registered loggers *must* be thread safe for this to work correctly!
        spdlog::flush_every(std::chrono::seconds(3));

        // apply some function on all registered loggers
        spdlog::apply_all([&](std::shared_ptr<spdlog::logger> l) { l->info("End of example."); });

        // release any threads created by spdlog, and drop all loggers in the registry.
        spdlog::shutdown();
    }
    // Exceptions will only be thrown upon failed logger or sink construction (not during logging)
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
        return 1;
    }
}

#include "spdlog/sinks/stdout_color_sinks.h"
// or #include "spdlog/sinks/stdout_sinks.h" if no colors needed
void stdout_example()
{
    // create color multi threaded logger
    auto console = spdlog::stdout_color_mt("console");
    console->info("Welcome to spdlog version {}.{}.{} !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
    console->error("Some error message with arg: {}", 1);

    auto err_logger = spdlog::stderr_color_mt("stderr");
    err_logger->error("Some error message");

    // Formatting examples
    console->warn("Easy padding in numbers like {:08d}", 12);
    console->critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    console->info("Support for floats {:03.2f}", 1.23456);
    console->info("Positional args are {1} {0}..", "too", "supported");
    console->info("{:<30}", "left aligned");

    spdlog::get("console")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name)");

    // Runtime log levels
    spdlog::set_level(spdlog::level::info); // Set global log level to info
    console->debug("This message should not be displayed!");
    console->set_level(spdlog::level::trace); // Set specific logger's log level
    console->debug("This message should be displayed..");

    // Customize msg format for all loggers
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    console->info("This an info message with custom format");
    spdlog::set_pattern("%+"); // back to default format

    // Compile time log levels
    // define SPDLOG_DEBUG_ON or SPDLOG_TRACE_ON
    SPDLOG_TRACE(console, "Enabled only #ifdef SPDLOG_TRACE_ON..{} ,{}", 1, 3.23);
    SPDLOG_DEBUG(console, "Enabled only #ifdef SPDLOG_DEBUG_ON.. {} ,{}", 1, 3.23);
}

#include "spdlog/sinks/basic_file_sink.h"
void basic_example()
{
    // Create basic file logger (not rotated)
    auto my_logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");
}

#include "spdlog/sinks/rotating_file_sink.h"
void rotating_example()
{
    // Create a file rotating logger with 5mb size max and 3 rotated files
    auto rotating_logger = spdlog::rotating_logger_mt("some_logger_name", "logs/rotating.txt", 1048576 * 5, 3);
}

#include "spdlog/sinks/daily_file_sink.h"
void daily_example()
{
    // Create a daily logger - a new file is created every day on 2:30am
    auto daily_logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);
}

// clone a logger and give it new name.
// Useful for creating component/subsystem loggers from some "root" logger
void clone_example()
{
    auto network_logger = spdlog::get("console")->clone("network");
    network_logger->info("Logging network stuff..");
}

#include "spdlog/async.h"
void async_example()
{
    // default thread pool settings can be modified *before* creating the async logger:
    // spdlog::init_thread_pool(32768, 1); // queue with max 32k items 1 backing thread.
    auto async_file = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/async_log.txt");
    // alternatively:
    // auto async_file = spdlog::create_async<spdlog::sinks::basic_file_sink_mt>("async_file_logger", "logs/async_log.txt");

    for (int i = 1; i < 101; ++i)
    {
        async_file->info("Async message #{}", i);
    }
}

// log binary data as hex.
// many types of std::container<char> types can be used.
// ranges are supported too.
// format flags:
// {:X} - print in uppercase.
// {:s} - don't separate each byte with space.
// {:p} - don't print the position on each line start.
// {:n} - don't split the output to lines.

#include "spdlog/fmt/bin_to_hex.h"

void binary_example()
{
    auto console = spdlog::get("console");
    std::array<char, 80> buf;
    console->info("Binary example: {}", spdlog::to_hex(buf));
    console->info("Another binary example:{:n}", spdlog::to_hex(std::begin(buf), std::begin(buf) + 10));
    // more examples:
    // logger->info("uppercase: {:X}", spdlog::to_hex(buf));
    // logger->info("uppercase, no delimiters: {:Xs}", spdlog::to_hex(buf));
    // logger->info("uppercase, no delimiters, no position info: {:Xsp}", spdlog::to_hex(buf));
}

// create logger with 2 targets with different log levels and formats
// the console will show only warnings or errors, while the file will log all

void multi_sink_example()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::warn);
    console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multisink.txt", true);
    file_sink->set_level(spdlog::level::trace);

    spdlog::logger logger("multi_sink", {console_sink, file_sink});
    logger.set_level(spdlog::level::debug);
    logger.warn("this should appear in both console and file");
    logger.info("this message should not appear in the console, only in the file");
}
// user defined types logging by implementing operator<<
#include "spdlog/fmt/ostr.h" // must be included
struct my_type
{
    int i;
    template<typename OStream>
    friend OStream &operator<<(OStream &os, const my_type &c)
    {
        return os << "[my_type i=" << c.i << "]";
    }
};

void user_defined_example()
{
    spdlog::get("console")->info("user defined type: {}", my_type{14});
}

//
// custom error handler
//
void err_handler_example()
{
    // can be set globally or per logger(logger->set_error_handler(..))
    spdlog::set_error_handler([](const std::string &msg) { spdlog::get("console")->error("*** ERROR HANDLER EXAMPLE ***: {}", msg); });
    spdlog::get("console")->info("some invalid message to trigger an error {}{}{}{}", 3);
}

// syslog example (linux/osx/freebsd)
#ifndef _WIN32
#include "spdlog/sinks/syslog_sink.h"
void syslog_example()
{
    std::string ident = "spdlog-example";
    auto syslog_logger = spdlog::syslog_logger_mt("syslog", ident, LOG_PID);
    syslog_logger->warn("This is warning that will end up in syslog.");
}
#endif

// Android example
#if defined(__ANDROID__)
#include "spdlog/sinks/android_sink.h"
void android_example()
{
    std::string tag = "spdlog-android";
    auto android_logger = spdlog::android_logger_mt("android", tag);
    android_logger->critical("Use \"adb shell logcat\" to view this message.");
}

#endif
