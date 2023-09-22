// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <atomic>
#include <utility>

// null, no cost dummy "mutex" and dummy "atomic" log level

namespace spdlog {
enum class log_level; // forward declaration

namespace details {
struct null_mutex
{
    void lock() const {}
    void unlock() const {}
};

struct null_atomic_log_level
{
    spdlog::log_level value;

    explicit null_atomic_log_level(spdlog::log_level new_value)
        : value(new_value)
    {}

    [[nodiscard]] log_level load(std::memory_order = std::memory_order_relaxed) const
    {
        return value;
    }

    void store(log_level new_value, std::memory_order = std::memory_order_relaxed)
    {
        value = new_value;
    }

    log_level exchange(log_level new_value, std::memory_order = std::memory_order_relaxed)
    {
        std::swap(new_value, value);
        return new_value; // return value before the call
    }
};

} // namespace details
} // namespace spdlog
