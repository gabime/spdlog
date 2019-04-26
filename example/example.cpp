//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
//
// spdlog usage example
//
//


#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"


int main(int, char *[])
{
    using spdlog::sinks::stderr_color_sink_mt;
    auto logger = spdlog::create_async<spdlog::sinks::stderr_color_sink_mt>("async");
    logger->info("HEllo xsdsdfs {}", 123);
    //spdlog::error("HEllo err {}", "HG FS");
}
