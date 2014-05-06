#pragma once

namespace c11log
{
namespace details
{
struct log_msg
{
    log_msg() = default;
    log_msg(level::level_enum l):msg_level(l) {};    
    
    bufpair_t msg_buf;
    log_clock::time_point msg_time;
    std::size_t msg_header_size;
    level::level_enum msg_level;
};
}
}
