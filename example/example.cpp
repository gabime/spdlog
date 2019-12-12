//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// spdlog usage example

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>

int main(int, char *[])
{
    try
    {
        spdlog::env::init();
        spdlog::info("Hello");
    }
    catch (spdlog::spdlog_ex &ex)
    {
        spdlog::info("spdlog_ex: {}", ex.what());
    }
}
