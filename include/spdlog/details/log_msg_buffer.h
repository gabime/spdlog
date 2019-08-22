// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/details/log_msg.h"
#include "spdlog/fmt/bundled/core.h"

namespace spdlog {
namespace details {

// regular log_msgs only holds string_views to stack data - so the cannot be stored for later use.
// this one can, since it contains and owns the payload buffer.
struct log_msg_buffer : log_msg
{
    fmt::basic_memory_buffer<char, 40> loggername_buf;
    fmt::basic_memory_buffer<char, 200> payload_buf;
    log_msg_buffer() = default;

    log_msg_buffer(const log_msg &orig_msg): log_msg(orig_msg)
    {
        update_buffers();
    }

    log_msg_buffer(const log_msg_buffer& other):log_msg(other)
    {
        update_buffers();
    }

    log_msg_buffer(const log_msg_buffer&& other):log_msg(std::move(other))
    {
        update_buffers();
    }

    log_msg_buffer& operator=(log_msg_buffer &other) SPDLOG_NOEXCEPT
    {
        *static_cast<log_msg*>(this) = other;
        update_buffers();
        return *this;
    }

    log_msg_buffer& operator=(log_msg_buffer &&other) SPDLOG_NOEXCEPT
    {
        *static_cast<log_msg*>(this) = std::move(other);
        update_buffers();
        return *this;
    }

    void update_buffers() SPDLOG_NOEXCEPT
    {
        loggername_buf.clear();
        loggername_buf.append(logger_name.data(), logger_name.data() + logger_name.size());
        logger_name = string_view_t{loggername_buf.data(), loggername_buf.size()};

        payload_buf.clear();
        payload_buf.append(payload.data(),payload.data() + payload.size());
        payload = string_view_t{payload_buf.data(), payload_buf.size()};
    }
};

}
}