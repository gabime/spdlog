// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/common.h"

#include <algorithm>
#include <cctype>
#include <iterator>

SPDLOG_NAMESPACE_BEGIN

namespace {
bool iequals(const std::string &a, const std::string &b) {
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), [](char ac, char bc) {
               return std::tolower(static_cast<unsigned char>(ac)) ==
                      std::tolower(static_cast<unsigned char>(bc));
           });
}
}  // namespace

level level_from_str(const std::string &name) noexcept {
    const auto it =
        std::find_if(std::begin(level_string_views), std::end(level_string_views),
                     [&name](const string_view_t &level_name) {
                         return level_name.size() == name.size() &&
                                std::equal(name.begin(), name.end(), level_name.begin(),
                                           [](char a, char b) {
                                               return std::tolower(static_cast<unsigned char>(a)) ==
                                                      std::tolower(static_cast<unsigned char>(b));
                                           });
                     });
    if (it != std::end(level_string_views)) {
        return static_cast<level>(std::distance(std::begin(level_string_views), it));
    }

    // check also for "warn" and "err" before giving up
    if (iequals(name, "warn")) {
        return level::warn;
    }
    if (iequals(name, "err")) {
        return level::err;
    }
    return level::off;
}

spdlog_ex::spdlog_ex(std::string msg)
    : msg_(std::move(msg)) {}

spdlog_ex::spdlog_ex(const std::string &msg, int last_errno) {
    memory_buf_t outbuf;
    fmt::format_system_error(outbuf, last_errno, msg.c_str());
    msg_ = fmt::to_string(outbuf);
}

const char *spdlog_ex::what() const noexcept { return msg_.c_str(); }

void throw_spdlog_ex(const std::string &msg, int last_errno) { throw(spdlog_ex(msg, last_errno)); }

void throw_spdlog_ex(std::string msg) { throw(spdlog_ex(std::move(msg))); }

SPDLOG_NAMESPACE_END
