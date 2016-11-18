//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//


#pragma once

#include <spdlog/details/log_msg.h>

namespace spdlog
{
namespace sinks
{
class sink
{
public:
    sink()
    {
         _level = level::trace;
    }

    virtual ~sink() {}
    virtual void log(const details::log_msg& msg) = 0;
    virtual void flush() = 0;

    bool should_log(level::level_enum msg_level) const;
    void set_level(level::level_enum log_level);
    level::level_enum level() const;

private:
    level_t _level;

};

inline bool sink::should_log(level::level_enum msg_level) const
{
    return msg_level >= _level.load(std::memory_order_relaxed);
}

inline void sink::set_level(level::level_enum log_level)
{
    _level.store(log_level);
}

inline level::level_enum sink::level() const
{
    return static_cast<spdlog::level::level_enum>(_level.load(std::memory_order_relaxed));
}

}
}

