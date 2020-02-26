// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
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
        if (logger_name.empty())
        {
            default_level_ = lvl;
        }
        else
        {
            levels_[logger_name] = lvl;
        }
    }

    void set_default(level::level_enum lvl)
    {
        default_level_ = lvl;
    }

    level::level_enum get(const std::string &logger_name)
    {
        auto it = levels_.find(logger_name);
        return it != levels_.end() ? it->second : default_level_;
    }

    level::level_enum default_level()
    {
        return default_level_;
    }
};
} // namespace cfg
} // namespace spdlog
