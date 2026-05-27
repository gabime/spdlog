// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "../details/log_msg.h"
#include "../formatter.h"

SPDLOG_NAMESPACE_BEGIN
namespace sinks {
class SPDLOG_API sink {
public:
    virtual ~sink() = default;
    virtual void log(const details::log_msg &msg) = 0;
    virtual void flush() = 0;
    virtual void set_pattern(const std::string &pattern) = 0;
    virtual void set_formatter(std::unique_ptr<formatter> sink_formatter) = 0;

    void set_level(level level) { level_.store(level, std::memory_order_relaxed); }
    level log_level() const { return level_.load(std::memory_order_relaxed); }
    bool should_log(level msg_level) const { return msg_level >= level_.load(std::memory_order_relaxed); }

protected:
    // sink log level - default is all
    atomic_level_t level_{level::trace};
};

}  // namespace sinks
SPDLOG_NAMESPACE_END
