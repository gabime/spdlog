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
    CRITICAL,
    FATAL,
    NONE = 99
} level_enum;

static const char* level_names[] { "debug", "info", "warning", "error", "critical", "fatal" };
inline const char* to_str(c11log::level::level_enum l)
{
    return level_names[l];
}
} //level
} //c11log
