// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/loaders/log_levels.h>

namespace spdlog {
namespace loaders {
namespace helpers {
//
// Init levels from given string
//
// Examples:
//
// set global level to debug: "debug"
// turn off all logging except for logger1: "off,logger1=debug"
// turn off all logging except for logger1 and logger2: "off,logger1=debug,logger2=info"
//
log_levels extract_levels(const std::string &txt);
} // namespace helpers

} // namespace loaders
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "helpers-inl.h"
#endif // SPDLOG_HEADER_ONLY
