// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/sinks/sink.h>
#include <spdlog/common.h>

bool spdlog::sinks::sink::should_log(spdlog::level::level_enum msg_level) const
{
    return msg_level >= level_.load(std::memory_order_relaxed);
}

void spdlog::sinks::sink::set_level(level::level_enum log_level)
{
    level_.store(log_level, std::memory_order_relaxed);
}

spdlog::level::level_enum spdlog::sinks::sink::level() const
{
    return static_cast<spdlog::level::level_enum>(level_.load(std::memory_order_relaxed));
}
