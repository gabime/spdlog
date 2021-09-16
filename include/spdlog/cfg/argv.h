// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <spdlog/cfg/helpers.h>

//
// Init log levels using each argv entry that starts with "SPDLOG_LEVEL="
//
// set all loggers to debug level:
// example.exe "SPDLOG_LEVEL=debug"

// set logger1 to trace level
// example.exe "SPDLOG_LEVEL=logger1=trace"

// turn off all logging except for logger1 and logger2:
// example.exe "SPDLOG_LEVEL=off,logger1=debug,logger2=info"

namespace spdlog {
namespace cfg {

// search for SPDLOG_LEVEL= in the args and use it to init the levels
inline level_map load_argv_levels(int argc, const char **argv)
{
    const std::string spdlog_level_prefix = "SPDLOG_LEVEL=";
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg.find(spdlog_level_prefix) == 0)
        {
            auto levels_string = arg.substr(spdlog_level_prefix.size());
            return helpers::load_levels(levels_string);
        }
    }

    // the "SPDLOG_LEVEL=" prefix was not found in any of the argv items
    return std::unordered_map<std::string, level::level_enum>{};
}

inline level_map load_argv_levels(int argc, char **argv)
{
    return load_argv_levels(argc, const_cast<const char **>(argv));
}

} // namespace cfg
} // namespace spdlog
