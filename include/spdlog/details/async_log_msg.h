// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <cstdint>

#include "./log_msg.h"

SPDLOG_NAMESPACE_BEGIN
namespace details {

// Extend log_msg with internal buffer to store its payload.
// This is needed since log_msg holds string_views that points to stack data.
class SPDLOG_API async_log_msg : public log_msg {
public:
    enum class type : std::uint8_t { log, flush, terminate };
    async_log_msg() = default;
    explicit async_log_msg(type type);
    async_log_msg(type type, const log_msg &orig_msg);
    ~async_log_msg() = default;
    async_log_msg(const async_log_msg &other);
    async_log_msg(async_log_msg &&other) noexcept;
    async_log_msg &operator=(const async_log_msg &other);
    async_log_msg &operator=(async_log_msg &&other) noexcept;
    [[nodiscard]] type message_type() const { return msg_type_; }

private:
    type msg_type_{type::log};
    memory_buf_t buffer_;
    void update_string_views();
};

}  // namespace details
SPDLOG_NAMESPACE_END
