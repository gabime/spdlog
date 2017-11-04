//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/details/pattern_formatter_impl.h"

#include <string>

namespace spdlog
{
namespace mdc
{
inline void set_context(std::string key, std::string value)
{
#if ENABLE_MDC_FORMATTER
    spdlog::details::MDC_formatter::mdc_map[std::move(key)] = std::move(value);
#endif
}

inline void remove_context(const std::string& key)
{
#if ENABLE_MDC_FORMATTER
    auto &mdc_map = spdlog::details::MDC_formatter::mdc_map;
    auto value_it = mdc_map.find(key);
    if (value_it != mdc_map.end())
        mdc_map.erase(value_it);
#endif
}
}
}

