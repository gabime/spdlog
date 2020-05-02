//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// spdlog usage example

#include <cstdio>

void load_levels_example();
void stdout_logger_example();
void basic_example();
void rotating_example();
void daily_example();
void async_example();
void binary_example();
void trace_example();
void multi_sink_example();
void user_defined_example();
void err_handler_example();
void syslog_example();
void custom_flags_example();

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h" // for loading levels from the environment variable

#include "spdlog/sinks/basic_file_sink.h"
void basic_example()
{
    // Create basic file logger (not rotated).

    spdlog::file_event_handlers handlers;
    handlers.after_open = [](spdlog::filename_t, std::FILE* fstream)
    {
        fputs("OPEN!\r\n", fstream);
    };
    auto fsink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("c:\\temp\\test.log", true, handlers);
}

int main(int, char *[])
{
    basic_example();
}