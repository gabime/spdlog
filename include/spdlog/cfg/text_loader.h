// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/cfg/cfg.h>
#include <spdlog/common.h>
#include <string>
#include <unordered_map>

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
namespace text_loader {
log_levels load_levels(const std::string &txt);
}

} // namespace cfg
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "text_loader-inl.h"
#endif // SPDLOG_HEADER_ONLY
