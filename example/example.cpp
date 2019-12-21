//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// spdlog usage example

#include <cstdio>

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

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/loaders/env.h>
#include <spdlog/loaders/argv.h>

int main(int args, char *argv[])
{

    //spdlog::loaders::load_env();
    spdlog::loaders::load_argv(args, argv);
    spdlog::info("HELLO INFO");

    auto l1 = spdlog::stderr_color_st("l1");
    l1->trace("L1 TRACE");
}