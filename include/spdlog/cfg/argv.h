// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <spdlog/cfg/helpers.h>
#include <spdlog/details/os.h>

//
// Init levels from argv SPDLOG_LEVEL
// Example: my_program.exe "SPDLOG_LEVEL=trace"

namespace spdlog {
namespace cfg {
namespace argv {
// search for SPDLOG_LEVEL= in the args and use it to init the levels
void load_levels(int args, char *argv[])
{
    const std::string spdlog_level_prefix = "SPDLOG_LEVEL=";
    for (int i = 1; i < args; i++)
    {
        std::string arg = argv[i];
        if (arg.find(spdlog_level_prefix) == 0)
        {
            auto levels_string = arg.substr(spdlog_level_prefix.size());
            auto levels = helpers::extract_levels(levels_string);
            details::registry::instance().update_levels(std::move(levels));
        }
    }
}
} // namespace argv
} // namespace cfg
} // namespace spdlog
