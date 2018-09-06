#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/spdlog.h"
#include <iostream>
#include <memory>

int main(int, char *[])
{
    bool enable_debug = true;
    try
    {
        // This other example use a single logger with multiple sinks.
        // This means that the same log_msg is forwarded to multiple sinks;
        // Each sink can have it's own log level and a message will be logged.
        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("./log_regular_file.txt"));
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("./log_debug_file.txt"));

        spdlog::logger console_multisink("multisink", sinks.begin(), sinks.end());
        console_multisink.set_level(spdlog::level::warn);

        sinks[0]->set_level(spdlog::level::trace); // console. Allow everything.  Default value
        sinks[1]->set_level(spdlog::level::trace); //  regular file. Allow everything.  Default value
        sinks[2]->set_level(spdlog::level::off);   //  regular file. Ignore everything.

        console_multisink.warn("warn: will print only on console and regular file");

        if (enable_debug)
        {
            console_multisink.set_level(spdlog::level::debug); // level of the logger
            sinks[1]->set_level(spdlog::level::debug);         // regular file
            sinks[2]->set_level(spdlog::level::debug);         // debug file
        }
        console_multisink.debug("Debug: you should see this on console and both files");

        // Release and close all loggers
        spdlog::drop_all();
    }
    // Exceptions will only be thrown upon failed logger or sink construction (not during logging)
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
        return 1;
    }
}
