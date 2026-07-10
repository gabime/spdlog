// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/details/log_msg_buffer.h>
#endif

#include <cstring>
#include <cstddef>

SPDLOG_NAMESPACE_BEGIN
namespace details {

SPDLOG_INLINE log_msg_buffer::log_msg_buffer(const log_msg &orig_msg)
    : log_msg{orig_msg} {
    buffer.append(logger_name.begin(), logger_name.end());
    buffer.append(payload.begin(), payload.end());
    append_source();
    update_string_views();
}

SPDLOG_INLINE log_msg_buffer::log_msg_buffer(const log_msg_buffer &other)
    : log_msg{other} {
    buffer.append(logger_name.begin(), logger_name.end());
    buffer.append(payload.begin(), payload.end());
    append_source();
    update_string_views();
}

SPDLOG_INLINE log_msg_buffer::log_msg_buffer(log_msg_buffer &&other) SPDLOG_NOEXCEPT
    : log_msg{other},
      buffer{std::move(other.buffer)} {
    update_string_views();
}

SPDLOG_INLINE log_msg_buffer &log_msg_buffer::operator=(const log_msg_buffer &other) {
    log_msg::operator=(other);
    buffer.clear();
    buffer.append(other.buffer.data(), other.buffer.data() + other.buffer.size());
    update_string_views();
    return *this;
}

SPDLOG_INLINE log_msg_buffer &log_msg_buffer::operator=(log_msg_buffer &&other) SPDLOG_NOEXCEPT {
    log_msg::operator=(other);
    buffer = std::move(other.buffer);
    update_string_views();
    return *this;
}

// Append the null-terminated source_loc strings to the buffer so that they are
// owned by this log_msg_buffer. Without this, source.filename/source.funcname
// would keep pointing at the caller's memory, which dangles once the message is
// queued (async logging, ringbuffer_sink, ...) and outlives that memory.
SPDLOG_INLINE void log_msg_buffer::append_source() {
    if (source.filename != nullptr) {
        buffer.append(source.filename, source.filename + std::strlen(source.filename) + 1);
    }
    if (source.funcname != nullptr) {
        buffer.append(source.funcname, source.funcname + std::strlen(source.funcname) + 1);
    }
}

SPDLOG_INLINE void log_msg_buffer::update_string_views() {
    logger_name = string_view_t{buffer.data(), logger_name.size()};
    payload = string_view_t{buffer.data() + logger_name.size(), payload.size()};

    // Re-point the source strings into our buffer (in the same order they were
    // appended by append_source). They are stored null-terminated.
    size_t offset = logger_name.size() + payload.size();
    if (source.filename != nullptr) {
        source.filename = buffer.data() + offset;
        offset += std::strlen(source.filename) + 1;
    }
    if (source.funcname != nullptr) {
        source.funcname = buffer.data() + offset;
    }
}

}  // namespace details
SPDLOG_NAMESPACE_END
