#pragma once

namespace c11log
{
namespace details
{
struct log_msg
{
    log_msg() = default;
    log_msg(level::level_enum l):msg_level(l) {};
    log_msg(const log_msg& other)
    {
        msg_buf = other.msg_buf;
        msg_time = other.msg_time;
        msg_header_size = other.msg_header_size;
        msg_level = other.msg_level;
    }

    bufpair_t msg_buf;
    log_clock::time_point msg_time;
    std::size_t msg_header_size;
    level::level_enum msg_level;
};
}
}
