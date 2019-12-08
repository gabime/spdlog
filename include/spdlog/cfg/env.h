// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
#include <string>
#include <unordered_map>

// config spdlog from environment variables
namespace spdlog 
{
 namespace cfg
 {
     struct logger_cfg {
         logger_cfg(level::level_enum level, std::string pattern):
            level{level}, pattern(std::move(pattern)) {}

         level::level_enum level;
         std::string pattern;
     };
     using cfg_map = std::unordered_map<std::string, logger_cfg>;

    // Init levels and patterns from env variabls SPDLOG_LEVEL & SPDLOG_PATTERN
    // Examples:
    // export SPDLOG_LEVEL=debug
    // export SPDLOG_LEVEL=logger1=%v,*=[%x] [%l] [%n] %v
    // export SPDLOG_LEVEL=logger1=debug,logger2=info,*=error
    // export SPDLOG_PATTERN=[%x] [%l] [%n] %v
    //
    // Note: will set the level to info if finds unknown level in SPDLOG_LEVEL
     cfg_map from_env();
 }
}

#ifdef SPDLOG_HEADER_ONLY
#include "env-inl.h"
#endif // SPDLOG_HEADER_ONLY
