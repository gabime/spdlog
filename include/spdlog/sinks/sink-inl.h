// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/sinks/sink.h>
#endif

#include <spdlog/common.h>

SPDLOG_NAMESPACE_BEGIN

SPDLOG_INLINE bool sinks::sink::should_log(level::level_enum msg_level) const {
    return msg_level >= level_.load(std::memory_order_relaxed);
}

SPDLOG_INLINE void sinks::sink::set_level(level::level_enum log_level) {
    level_.store(log_level, std::memory_order_relaxed);
}

SPDLOG_INLINE level::level_enum sinks::sink::level() const {
    return static_cast<level::level_enum>(level_.load(std::memory_order_relaxed));
}

SPDLOG_NAMESPACE_END
