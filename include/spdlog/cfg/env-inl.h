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
#include <utility>
#include <sstream>
#include <cassert>

namespace spdlog {
namespace env {

using name_val_pair = std::pair<std::string, std::string>;

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

// return (name,value) pair from given "name=value" string.
// return empty string on missing parts
// "key=val" => ("key", "val")
// " key  =  val " => ("key", "val")
// "key=" => ("key", "")
// "val" => ("", "val")
inline name_val_pair extract_kv_(char sep, const std::string &str)
{
    auto n = str.find(sep);
    std::string k, v;
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

// return vector of key/value pairs from sequence of "K1=V1,K2=V2,.."
// "a=AAA,b=BBB,c=CCC,.." => {("a","AAA"),("b","BBB"),("c", "CCC"),...}
SPDLOG_INLINE std::unordered_map<std::string, std::string> extract_key_vals_(const std::string &str)
{
    std::string token;
    std::istringstream token_stream(str);
    std::unordered_map<std::string, std::string> rv;
    while (std::getline(token_stream, token, ','))
    {
        if (token.empty())
        {
            continue;
        }
        auto kv = extract_kv_('=', token);

        // '*' marks all loggers
        if (kv.first.empty())
        {
            kv.first = "*";
        }
        rv.insert(std::move(kv));
    }
    return rv;
}

inline details::registry::logger_cfgs from_env_()
{
    using details::os::getenv;
    details::registry::logger_cfgs configs;

    auto levels = extract_key_vals_(getenv("SPDLOG_LEVEL"));
    auto patterns = extract_key_vals_(getenv("SPDLOG_PATTERN"));

    // merge to single dict. and take into account "*"
    for (auto &name_level : levels)
    {
        auto &logger_name = name_level.first;
        assert(!logger_name.empty());
        auto level_name = to_lower_(name_level.second);
        details::registry::logger_cfg cfg;
        cfg.level_name = level_name;
        if (logger_name == "*")
        {
            configs.default_cfg.level_name = cfg.level_name;
        }
        else
        {
            configs.loggers.emplace(logger_name, cfg);
        }

    }

    for (auto &name_pattern : patterns)
    {
        auto &logger_name = name_pattern.first;
        auto &pattern = name_pattern.second;
        auto it = configs.loggers.find(logger_name);

        if (it != configs.loggers.end())
        {
            it->second.pattern = pattern;
        }

        if (logger_name == "*")
        {
            configs.default_cfg.pattern = pattern;
        }
        else
        {
            details::registry::logger_cfg cfg;
            cfg.pattern = pattern;
            configs.loggers.emplace(logger_name, cfg);
        }
    }

    return configs;
}

SPDLOG_INLINE void init()
{
    spdlog::details::registry::instance().set_configs(from_env_());
}

} // namespace env
} // namespace spdlog
