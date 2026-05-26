// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/details/async_log_msg.h"

SPDLOG_NAMESPACE_BEGIN
namespace details {

async_log_msg::async_log_msg(const type type)
    : msg_type_{type} {}

// copy logger name and payload to buffer so can be used asynchronously
// note: source location pointers are copied without allocation since they
// are compiler generated const chars* (__FILE__, __LINE__, __FUNCTION__)
// if you pass custom strings to source location, make sure they outlive the async_log_msg
async_log_msg::async_log_msg(const type type, const log_msg &orig_msg)
    : log_msg{orig_msg},
      msg_type_(type) {
    buffer_.append(logger_name);
    buffer_.append(payload);
    update_string_views();
}

async_log_msg::async_log_msg(const async_log_msg &other)
    : log_msg{other},
      msg_type_{other.msg_type_} {
    buffer_.append(logger_name);
    buffer_.append(payload);
    update_string_views();
}

async_log_msg::async_log_msg(async_log_msg &&other) noexcept
    : log_msg{other},
      msg_type_{other.msg_type_},
      buffer_{std::move(other.buffer_)} {
    update_string_views();
}

async_log_msg &async_log_msg::operator=(const async_log_msg &other) {
    if (this == &other) return *this;
    log_msg::operator=(other);
    msg_type_ = other.msg_type_;
    buffer_.clear();
    buffer_.append(other.buffer_.data(), other.buffer_.data() + other.buffer_.size());
    update_string_views();
    return *this;
}

async_log_msg &async_log_msg::operator=(async_log_msg &&other) noexcept {
    if (this == &other) return *this;
    log_msg::operator=(other);
    msg_type_ = other.msg_type_;
    buffer_ = std::move(other.buffer_);
    update_string_views();
    return *this;
}

void async_log_msg::update_string_views() {
    logger_name = string_view_t{buffer_.data(), logger_name.size()};
    payload = string_view_t{buffer_.data() + logger_name.size(), payload.size()};
}

}  // namespace details
SPDLOG_NAMESPACE_END
