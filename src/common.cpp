// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/common.h>

#include <algorithm>
#include <iterator>

namespace spdlog {

spdlog::level level_from_str(const std::string &name) noexcept {
    auto it = std::find(std::begin(level_string_views), std::end(level_string_views), name);
    if (it != std::end(level_string_views))
        return static_cast<level>(std::distance(std::begin(level_string_views), it));

    // check also for "warn" and "err" before giving up..
    if (name == "warn") {
        return spdlog::level::warn;
    }
    if (name == "err") {
        return level::err;
    }
    return level::off;
}

spdlog_ex::spdlog_ex(std::string msg)
    : msg_(std::move(msg)) {}

spdlog_ex::spdlog_ex(const std::string &msg, int last_errno) {
#ifdef SPDLOG_USE_STD_FORMAT
    msg_ = std::system_error(std::error_code(last_errno, std::generic_category()), msg).what();
#else
    memory_buf_t outbuf;
    fmt::format_system_error(outbuf, last_errno, msg.c_str());
    msg_ = fmt::to_string(outbuf);
#endif
}

const char *spdlog_ex::what() const noexcept { return msg_.c_str(); }

void throw_spdlog_ex(const std::string &msg, int last_errno) {
    SPDLOG_THROW(spdlog_ex(msg, last_errno));
}

void throw_spdlog_ex(std::string msg) { SPDLOG_THROW(spdlog_ex(std::move(msg))); }

}  // namespace spdlog
