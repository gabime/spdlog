// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/sinks/sink.h>

#include <spdlog/common.h>

bool spdlog::sinks::sink::should_log(spdlog::level msg_level) const {
    return msg_level >= level_.load(std::memory_order_relaxed);
}

void spdlog::sinks::sink::set_level(level level) { level_.store(level, std::memory_order_relaxed); }

spdlog::level spdlog::sinks::sink::log_level() const {
    return static_cast<spdlog::level>(level_.load(std::memory_order_relaxed));
}
