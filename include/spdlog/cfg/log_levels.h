// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/logger.h>
#include <string>
#include <unordered_map>

namespace spdlog {
namespace cfg {
class log_levels
{
    std::unordered_map<std::string, spdlog::level::level_enum> levels_;
    spdlog::level::level_enum default_level_ = level::info;

public:
    void set(const std::string &logger_name, level::level_enum lvl)
    {
        levels_[logger_name] = lvl;
    }

    void set_default(level::level_enum lvl)
    {
        levels_[""] = lvl;
    }

    // configure log level of given logger if it appears in the config list or if default level is set
    void update_logger_level(spdlog::logger &logger)
    {
        auto &logger_name = logger.name();
        auto it = levels_.find(logger_name);

        // if logger was not configured, check if default log level was configured
        if (it == levels_.end())
        {
            it = levels_.find(""); //
        }

        if (it != levels_.end())
        {
            logger.set_level(it->second);
        }
    }
};
} // namespace cfg
} // namespace spdlog
