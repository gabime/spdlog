// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
#include <string>
#include <unordered_map>

//
// Init levels and patterns from env variables SPDLOG_LEVEL and SPDLOG_PATTERN.
// Inspired from Rust's "env_logger" crate (https://crates.io/crates/env_logger).
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
//
// set global pattern:
// export SPDLOG_PATTERN="[%x] [%l] [%n] %v"
//
// set pattern for logger1:
// export SPDLOG_PATTERN="logger1=%v,*=[%x] [%l] [%n] %v"
//
// set global pattern and different pattern for logger1:
// export SPDLOG_PATTERN="[%x] [%l] [%n] %v,logger1=[%u] %v"

namespace spdlog {
namespace env {
void init();
}
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "env-inl.h"
#endif // SPDLOG_HEADER_ONLY
