#pragma once

#if defined(__GNUC__) && __GNUC__ == 12
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"  // Workaround for GCC 12
#endif
#include <catch2/catch_all.hpp>
#if defined(__GNUC__) && __GNUC__ == 12
    #pragma GCC diagnostic pop
#endif

#include <stdlib.h>

#include <chrono>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#include "utils.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#undef SPDLOG_LEVEL_NAMES
#undef SPDLOG_SHORT_LEVEL_NAMES

#include "spdlog/details/fmt_helper.h"
#include "spdlog/pattern_formatter.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

// to_string_view
[[nodiscard]] constexpr spdlog::string_view_t to_string_view(const spdlog::memory_buf_t &buf) noexcept {
    return spdlog::string_view_t{buf.data(), buf.size()};
}
