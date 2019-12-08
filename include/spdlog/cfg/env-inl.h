// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include "spdlog/cfg/env.h"
#endif

#include "spdlog/spdlog.h"
#include "spdlog/details/os.h"
#include "spdlog/details/registry.h"

#include <string>
#include <tuple>
#include <sstream>
#include <cctype>
#include <algorithm>

namespace spdlog {
namespace cfg {
// inplace convert  to lowercase
inline std::string &to_lower_(std::string &str)
{
    std::transform(
        str.begin(), str.end(), str.begin(), [](char ch) { return static_cast<char>((ch >= 'A' && ch <= 'Z') ? ch + ('a' - 'A') : ch); });
    return str;
}

// inplace trim spaces
inline std::string &trim_(std::string &str)
{
    const char *spaces = " \n\r\t";
    str.erase(str.find_last_not_of(spaces) + 1);
    str.erase(0, str.find_first_not_of(spaces));
    return str;
}

using name_val_pair = std::pair<std::string, std::string>;

// return tuple with name, value from "name=value" string. replace with empty string on missing parts
inline name_val_pair extract_kv_(char sep, const std::string &str)
{
    auto n = str.find(sep);
    std::string k;
    std::string v;
    if (n == std::string::npos)
    {
        v = str;
    }
    else
    {
        k = str.substr(0, n);
        v = str.substr(n + 1);
    }
    return std::make_pair(trim_(k), trim_(v));
}

// return vector of name/value pairs from str.
// str format: "a=A,b=B,c=C,d=D,.."
SPDLOG_INLINE std::vector<name_val_pair> extract_name_vals_(const std::string &str)
{
    std::vector<name_val_pair> rv;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, ','))
    {
        rv.push_back(extract_kv_('=', token));
    }
    return rv;
}

inline void load_levels_(cfg_map &configs)
{
    using details::os::getenv;
    std::string levels = getenv("SPDLOG_LEVEL");
    auto name_vals = extract_name_vals_(levels);

    for (auto &nv : name_vals)
    {
        auto logger_name = nv.first.empty() ? "*" : nv.first;
        auto level_lowercase = to_lower_(nv.second);
        auto log_level = level::from_str(level_lowercase);
        // set as info if unknown log level given
        if (log_level == level::off && level_lowercase != "off")
        {
            log_level = spdlog::level::info;
        }
        auto it = configs.find(logger_name);
        if (it != configs.end())
        {
            it->second.level = log_level;
        }
        else
        {
            configs.insert({logger_name, logger_cfg{log_level, "%+"}});
        }
    }
}

SPDLOG_INLINE void load_patterns_(cfg_map &configs)
{
    using details::os::getenv;
    std::string patterns = getenv("SPDLOG_PATTERN");
    auto name_vals = extract_name_vals_(patterns);
    for (auto &nv : name_vals)
    {
        auto logger_name = nv.first.empty() ? "*" : nv.first;
        auto pattern = to_lower_(nv.second);
        auto it = configs.find(logger_name);
        if (it != configs.end())
        {
            it->second.pattern = pattern;
        }
        else
        {
            configs.insert({logger_name, logger_cfg{level::info, pattern}});
        }
    }
}

SPDLOG_INLINE cfg_map from_env()
{
    cfg_map configs;
    load_levels_(configs);
    load_patterns_(configs);
    return configs;
}

} // namespace cfg
} // namespace spdlog
