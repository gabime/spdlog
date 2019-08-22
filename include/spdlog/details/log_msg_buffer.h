// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/details/log_msg.h"
#include "spdlog/fmt/bundled/core.h"

namespace spdlog {
namespace details {

// extend log_msg with internal buffer to store its payload.
// this is needed since log_msg holds string_views that points to stack data.

struct log_msg_buffer : log_msg
{
    fmt::basic_memory_buffer<char, 200> buffer;
    log_msg_buffer() = default;

    explicit log_msg_buffer(const log_msg &orig_msg)
        : log_msg(orig_msg)
    {
        buffer.append(logger_name.begin(), logger_name.end());
        logger_name = string_view_t{buffer.data(), buffer.size()};

        buffer.append(payload.begin(), payload.end());
        payload = string_view_t{logger_name.end(), payload.size()};
    }

    log_msg_buffer(log_msg_buffer &&other) = default;
    log_msg_buffer &operator=(log_msg_buffer &&other) = default;
};

} // namespace details
} // namespace spdlog