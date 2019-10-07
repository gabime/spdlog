// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifdef SPDLOG_USE_EXPORT_HEADER
#include "spdlog_export.h"
#else
#define SPDLOG_EXPORT
#endif

#include <atomic>
#include <utility>
// null, no cost dummy "mutex" and dummy "atomic" int

namespace spdlog {
namespace details {
struct SPDLOG_EXPORT null_mutex
{
    void lock() const {}
    void unlock() const {}
    bool try_lock() const
    {
        return true;
    }
};

struct SPDLOG_EXPORT null_atomic_int
{
    int value;
    null_atomic_int() = default;

    explicit null_atomic_int(int new_value)
        : value(new_value)
    {}

    int load(std::memory_order = std::memory_order_relaxed) const
    {
        return value;
    }

    void store(int new_value, std::memory_order = std::memory_order_relaxed)
    {
        value = new_value;
    }

    int exchange(int new_value, std::memory_order = std::memory_order_relaxed)
    {
        std::swap(new_value, value);
        return new_value; // return value before the call
    }
};

} // namespace details
} // namespace spdlog
