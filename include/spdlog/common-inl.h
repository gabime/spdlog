// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/common.h>
#endif

#include <algorithm>
#include <iterator>
#include <cctype>

SPDLOG_NAMESPACE_BEGIN
namespace level {

// SPDLOG_INLINE_VAR (rather than `static`): to_string_view/to_short_c_str/from_str below are
// exported as part of the C++20 module interface, so these namespace-scope arrays they reference
// need external linkage - a TU-local (`static`) entity cannot be referenced from an exported
// function's body without becoming an ill-formed "exposure" once this file is compiled inside the
// module's purview.
#if __cplusplus >= 201703L
constexpr
#endif
    SPDLOG_INLINE_VAR string_view_t level_string_views[] SPDLOG_LEVEL_NAMES;

SPDLOG_INLINE_VAR const char *short_level_names[] SPDLOG_SHORT_LEVEL_NAMES;

SPDLOG_INLINE const string_view_t &to_string_view(level::level_enum l) SPDLOG_NOEXCEPT {
    return level_string_views[l];
}

SPDLOG_INLINE const char *to_short_c_str(level::level_enum l) SPDLOG_NOEXCEPT {
    return short_level_names[l];
}

SPDLOG_INLINE level::level_enum from_str(const std::string &name) SPDLOG_NOEXCEPT {
    auto it = std::find_if(std::begin(level_string_views), std::end(level_string_views),
                           [&name](const string_view_t &level_name) {
                               return level_name.size() == name.size() &&
                                      std::equal(name.begin(), name.end(), level_name.begin(),
                                                 [](char a, char b) {
                                                     return std::tolower(static_cast<unsigned char>(a)) ==
                                                            std::tolower(static_cast<unsigned char>(b));
                                                 });
                           });
    if (it != std::end(level_string_views))
        return static_cast<level::level_enum>(std::distance(std::begin(level_string_views), it));

    // check also for "warn" and "err" before giving up..
    auto iequals = [](const std::string &a, const std::string &b) {
        return a.size() == b.size() &&
               std::equal(a.begin(), a.end(), b.begin(), [](char ac, char bc) {
                   return std::tolower(static_cast<unsigned char>(ac)) ==
                          std::tolower(static_cast<unsigned char>(bc));
               });
    };

    if (iequals(name, "warn")) {
        return level::warn;
    }
    if (iequals(name, "err")) {
        return level::err;
    }
    return level::off;
}
}  // namespace level

SPDLOG_INLINE spdlog_ex::spdlog_ex(std::string msg)
    : msg_(std::move(msg)) {}

SPDLOG_INLINE spdlog_ex::spdlog_ex(const std::string &msg, int last_errno) {
#ifdef SPDLOG_USE_STD_FORMAT
    msg_ = std::system_error(std::error_code(last_errno, std::generic_category()), msg).what();
#else
    memory_buf_t outbuf;
    fmt::format_system_error(outbuf, last_errno, msg.c_str());
    msg_ = fmt::to_string(outbuf);
#endif
}

SPDLOG_INLINE const char *spdlog_ex::what() const SPDLOG_NOEXCEPT { return msg_.c_str(); }

SPDLOG_INLINE void throw_spdlog_ex(const std::string &msg, int last_errno) {
    SPDLOG_THROW(spdlog_ex(msg, last_errno));
}

SPDLOG_INLINE void throw_spdlog_ex(std::string msg) { SPDLOG_THROW(spdlog_ex(std::move(msg))); }

SPDLOG_NAMESPACE_END
