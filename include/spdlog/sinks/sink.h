//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/details/log_msg.h"
#include "spdlog/details/pattern_formatter.h"
#include "spdlog/formatter.h"

namespace spdlog {
namespace sinks {
class sink
{
public:
    // default sink ctor with default pattern formatter
    sink()
        : formatter_(std::unique_ptr<spdlog::formatter>(new pattern_formatter("%+")))
    {
    }

    virtual ~sink() = default;

    virtual void log(const details::log_msg &msg) = 0;
    virtual void flush() = 0;

    bool should_log(level::level_enum msg_level) const
    {
        return msg_level >= level_.load(std::memory_order_relaxed);
    }

    void set_level(level::level_enum log_level)
    {
        level_.store(log_level);
    }

    level::level_enum level() const
    {
        return static_cast<spdlog::level::level_enum>(level_.load(std::memory_order_relaxed));
    }

    void set_pattern(const std::string &pattern)
    {
        formatter_ = std::unique_ptr<spdlog::formatter>(new pattern_formatter(pattern));
    }

    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter)
    {
        formatter_ = std::move(sink_formatter);
    }

protected:
    level_t level_{level::trace};
    std::unique_ptr<spdlog::formatter> formatter_;
};

} // namespace sinks
} // namespace spdlog
