// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/details/log_msg.h"
#include "spdlog/fmt/fmt.h"

namespace spdlog {
namespace details {

// extend log_msg with internal buffer to store its payload.
// this is needed since log_msg holds string_views that points to stack data.

class log_msg_buffer : public log_msg
{
    memory_buf_t buffer;
    void update_string_views()
    {
        logger_name = string_view_t{buffer.data(), logger_name.size()};
        payload = string_view_t{logger_name.end(), payload.size()};
    }

public:
    log_msg_buffer() = default;

    explicit log_msg_buffer(const log_msg &orig_msg)
        : log_msg{orig_msg}
    {
        buffer.append(logger_name.begin(), logger_name.end());
        buffer.append(payload.begin(), payload.end());
        update_string_views();
    }

    log_msg_buffer(const log_msg_buffer &other)
        : log_msg{other}
    {
        buffer.append(logger_name.begin(), logger_name.end());
        buffer.append(payload.begin(), payload.end());
        update_string_views();
    }

    log_msg_buffer(log_msg_buffer &&other)
        : log_msg{std::move(other)}
        , buffer{std::move(other.buffer)}
    {
        update_string_views();
    }

    log_msg_buffer &operator=(const log_msg_buffer &other)
    {
        log_msg::operator=(other);
        buffer.append(other.buffer.data(), other.buffer.data() + other.buffer.size());
        update_string_views();
        return *this;
    }

    log_msg_buffer &operator=(log_msg_buffer &&other)
    {
        log_msg::operator=(std::move(other));
        buffer = std::move(other.buffer);
        update_string_views();
        return *this;
    }
};

} // namespace details
} // namespace spdlog
