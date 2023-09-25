// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <atomic>
#include <utility>

// null, no cost dummy "mutex" and dummy "atomic" log level

namespace spdlog {
namespace details {
struct null_mutex {
    void lock() const {}
    void unlock() const {}
};

template <typename T>
struct null_atomic {
    T value;

    null_atomic() = default;

    explicit constexpr null_atomic(T new_value)
        : value(new_value) {}

    [[nodiscard]] T load(std::memory_order = std::memory_order_seq_cst) const { return value; }

    void store(T new_value, std::memory_order = std::memory_order_seq_cst) { value = new_value; }

    T exchange(T new_value, std::memory_order = std::memory_order_seq_cst) {
        std::swap(new_value, value);
        return new_value;  // return value before the call
    }
};

}  // namespace details
}  // namespace spdlog
