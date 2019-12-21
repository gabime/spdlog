// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/cfg/log_levels.h>
#include <spdlog/common.h>
#include <string>
#include <unordered_map>

//
// Init levels from given string
//

namespace spdlog {
namespace cfg {
namespace helpers {
log_levels extract_levels(const std::string &txt);
}

} // namespace cfg
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "helpers-inl.h"
#endif // SPDLOG_HEADER_ONLY
