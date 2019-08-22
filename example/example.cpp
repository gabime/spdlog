//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// spdlog usage example

#include <cstdio>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/backtrace-sink.h"

#include <vector>


using namespace spdlog::details;
using namespace spdlog::sinks;

int main(int, char *[]) {
    auto backtrack_sink = std::make_shared<backtrace_sink_mt >(spdlog::level::debug, 10);
    backtrack_sink ->add_sink(std::make_shared<stdout_color_sink_mt>());
    spdlog::logger l("loggername", backtrack_sink);
    l.set_level(spdlog::level::trace);
    //spdlog::set_backtrace(spdlog::level::warn, 16)
    for(int i = 0; i < 100; i++)
    {
        //l.log(spdlog::source_loc{__FILE__, __LINE__, "main"}, spdlog::level::debug, "Debug message #{}", i);
        SPDLOG_LOGGER_TRACE((&l), "Debug message #{}", i);
    }
    l.warn("This will trigger the log of all prev messages in the queue");
}
