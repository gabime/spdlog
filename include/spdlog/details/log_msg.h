// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
#include <string>

#if !defined(_WIN32) && defined(SPDLOG_EXTENDED_STLYING)
#   include <vector>
#   include <array>
#endif


namespace spdlog {
namespace details {

#if !defined(_WIN32) && defined(SPDLOG_EXTENDED_STLYING)
enum class style_type
{
    // reservation for data structures
    null_style,
    // font style
    reset, bold, dark, underline, blink, reverse,
    // font foreground colors
    fg_black, fg_red, fg_green, fg_yellow, fg_blue, fg_magenta, fg_cyan, fg_white, fg_default,
    // font  background colors
    bg_black, bg_red, bg_green, bg_yellow, bg_blue, bg_magenta, bg_cyan, bg_white, bg_default,
};

#if !defined(SPDLOG_ANSI_STLYE_COUNT)
#   define SPDLOG_ANSI_STLYE_COUNT 25
    using styles_array  = std::array<details::style_type, SPDLOG_ANSI_STLYE_COUNT>;
    using style_strings = std::array<std::string, SPDLOG_ANSI_STLYE_COUNT>;
    using style_codes   = std::array<string_view_t, SPDLOG_ANSI_STLYE_COUNT>;
#endif

// styling info.
struct styling_info
{
    styling_info() = default;
    styling_info(details::styles_array styles)
        : styles(styles)
        , is_start(true)
    {}

    bool is_start   = false;
    size_t position = 0;
    details::styles_array styles{};
};
#endif

struct SPDLOG_API log_msg
{
    log_msg() = default;
    log_msg(log_clock::time_point log_time, source_loc loc, string_view_t logger_name, level::level_enum lvl, string_view_t msg);
    log_msg(source_loc loc, string_view_t logger_name, level::level_enum lvl, string_view_t msg);
    log_msg(string_view_t logger_name, level::level_enum lvl, string_view_t msg);
    log_msg(const log_msg &other) = default;
    log_msg &operator=(const log_msg &other) = default;

    string_view_t logger_name;
    level::level_enum level{level::off};
    log_clock::time_point time;
    size_t thread_id{0};

    // wrapping the formatted text with color (updated by pattern_formatter).
    mutable size_t color_range_start{0};
    mutable size_t color_range_end{0};

#if !defined(_WIN32) && defined(SPDLOG_EXTENDED_STLYING)
    // for ansi console styling
    mutable std::vector<styling_info> styling_ranges;
#endif

    source_loc source;
    string_view_t payload;
};
} // namespace details
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#    include "log_msg-inl.h"
#endif
