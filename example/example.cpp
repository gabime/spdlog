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
        spdlog::cfg::env::init();
        spdlog::info("Hello");

        //        auto cfg = spdlog::cfg::from_env();
        //        for (auto &item : cfg)
        //        {
        //            spdlog::info("logger: '{}' level: '{}' pattern: '{}'", item.first, item.second.level_name, item.second.pattern);
        //        }
    }
    catch (spdlog::spdlog_ex &ex)
    {
        spdlog::info("spdlog_ex: {}", ex.what());
    }
}
