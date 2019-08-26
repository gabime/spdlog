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
void clone_example();

#include "spdlog/spdlog.h"

int main(int, char *[])
{

    // Backtrace support of latest debug/trace messages
    spdlog::enable_backtrace(16);

    spdlog::trace("Backtrace message 1");
    spdlog::debug("Backtrace message 2");
    spdlog::info("Hello");
    spdlog::dump_backtrace();
}