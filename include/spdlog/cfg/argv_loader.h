// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <spdlog/cfg/text_loader.h>
#include <spdlog/details/os.h>

//
// Init levels and patterns from env variables SPDLOG_LEVEL
// Inspired from Rust's "env_logger" crate (https://crates.io/crates/env_logger).
// Note - fallback to "info" level on unrecognized levels
//
// Examples:
//
// set global level to debug:
// export SPDLOG_LEVEL=debug
//
// turn off all logging except for logger1:
// export SPDLOG_LEVEL="off,logger1=debug"
//
// turn off all logging except for logger1 and logger2:
// export SPDLOG_LEVEL="off,logger1=debug,logger2=info"

namespace spdlog {
namespace cfg {
// search for SPDLOG_LEVEL= in the args and use it to init the levels
void init_from_argv(int args, char *argv[])
{
    if (args < 2)
    {
        return;
    }
    const std::string spdlog_level_prefix = "SPDLOG_LEVEL=";
    for (int i = 0; i < args; i++)
    {
        std::string arg = argv[i];
        if (arg.find(spdlog_level_prefix) == 0)
        {
            auto cfg_string = arg.substr(spdlog_level_prefix.size());
            auto levels = text_loader::load_levels(cfg_string);
            spdlog::details::registry::instance().set_levels(levels);
            return;
        }
    }
}
} // namespace cfg
} // namespace spdlog
