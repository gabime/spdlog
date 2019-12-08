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
        auto cfg = spdlog::cfg::from_env();
        for (auto &item : cfg)
        {
            spdlog::info("['{}'] level: {} pattern: {}", item.first, spdlog::level::to_string_view(item.second.level), item.second.pattern);
        }
    }
    catch (spdlog::spdlog_ex &ex)
    {
        spdlog::info("spdlog_ex: {}", ex.what());
    }
}
