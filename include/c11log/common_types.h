#pragma once

#include <chrono>

namespace c11log
{

typedef std::chrono::system_clock log_clock;

namespace level
{
typedef enum {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    NONE = 99
} level_enum;
const char* to_str(level_enum l);
}
}

static const char* level_names[] { "Debug", "Info", "Warning", "Error", "Fatal" };
inline const char* c11log::level::to_str(c11log::level::level_enum l)
{
    return level_names[l];
}

