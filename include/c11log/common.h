#pragma once

namespace c11log
{

using log_clock = std::chrono::system_clock;
namespace level
{
typedef enum
{
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL,
    FATAL,
    NONE = 99
} level_enum;

static const char* level_names[] { "trace", "debug", "info", "warning", "error", "critical", "fatal" };
inline const char* to_str(c11log::level::level_enum l)
{
    return level_names[l];
}
} //level
} //c11log
