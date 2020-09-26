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
void stopwatch_example();
void trace_example();
void multi_sink_example();
void user_defined_example();
void err_handler_example();
void syslog_example();
void custom_flags_example();

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h" // for loading levels from the environment variable
#include "spdlog/sinks/stdout_color_sinks.h"

int main(int, char *[])
{
    // Log levels can be loaded from argv/env using "SPDLOG_LEVEL"
    spdlog::cfg::load_env_levels();
    spdlog::info("Default logger");
    auto l1 = spdlog::stdout_color_mt("l1");
    auto l2 = spdlog::stdout_color_mt("l2");

    l1->debug("L1");
    l2->trace("L2");
}