// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/log_msg.h>

namespace spdlog {
namespace details {

// Extend log_msg with internal buffer to store its payload.
// This is needed since log_msg holds string_views that points to stack data.

class SPDLOG_API log_msg_buffer : public log_msg {
    memory_buf_t buffer;
    void update_string_views();

public:
    log_msg_buffer() = default;
    explicit log_msg_buffer(const log_msg &orig_msg);
    log_msg_buffer(const log_msg_buffer &other);
    log_msg_buffer(log_msg_buffer &&other) SPDLOG_NOEXCEPT;
    log_msg_buffer &operator=(const log_msg_buffer &other);
    log_msg_buffer &operator=(log_msg_buffer &&other) SPDLOG_NOEXCEPT;
};

class SPDLOG_API movable_log_msg_buffer : public log_msg_buffer {
public:
    movable_log_msg_buffer() = default;

    explicit movable_log_msg_buffer(const log_msg &orig_msg)
        : log_msg_buffer(orig_msg) {}

    movable_log_msg_buffer(const movable_log_msg_buffer &) = delete;
    movable_log_msg_buffer &operator=(const movable_log_msg_buffer &) = delete;

#if defined(_MSC_VER) && _MSC_VER <= 1800
    movable_log_msg_buffer(movable_log_msg_buffer &&other) SPDLOG_NOEXCEPT
        : log_msg_buffer(std::move(other)) {}

    movable_log_msg_buffer &operator=(movable_log_msg_buffer &&other) SPDLOG_NOEXCEPT {
        log_msg_buffer::operator=(std::move(other));
        return *this;
    }
#else
    movable_log_msg_buffer(movable_log_msg_buffer &&) = default;
    movable_log_msg_buffer &operator=(movable_log_msg_buffer &&) = default;
#endif
};

}  // namespace details
}  // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "log_msg_buffer-inl.h"
#endif
