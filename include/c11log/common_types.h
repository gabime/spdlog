#pragma once

#include <chrono>

namespace c11log
{

typedef std::chrono::system_clock log_clock;
typedef std::pair<const char*, std::size_t> bufpair_t;

namespace level
{
typedef enum
{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    NONE = 99
} level_enum;

static const char* level_names[] { "debug", "info", "warning", "error", "fatal" };
inline const char* to_str(c11log::level::level_enum l)
{
    return level_names[l];
}
}

struct log_msg
{
	bufpair_t msg_buf;
    log_clock::time_point msg_time;
    std::size_t msg_header_size;
    level::level_enum msg_level;
};

}
