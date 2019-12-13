//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// spdlog usage example

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main(int, char *[])
{
    try
    {
        spdlog::set_level(spdlog::level::warn);
        spdlog::env::load_levels();

        auto l1 = spdlog::stderr_color_st("l1");
        auto l2 = spdlog::stderr_color_st("l2");

        spdlog::info("Hello default logger");
        l1->debug("Hello l1");
//        l2->trace("Hello l2");
    }
    catch (spdlog::spdlog_ex &ex)
    {
        spdlog::info("spdlog_ex: {}", ex.what());
    }
}
