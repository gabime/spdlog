#pragma once

#include "spdlog.h"
#include <atomic>
#include <limits>
#include <chrono>

#define SPDLOG_LOGGER_CONDITIONAL(logger, condition, level, ...) \
    do {                                                         \
        if (condition) {                                         \
            SPDLOG_LOGGER_CALL(logger, level, __VA_ARGS__);      \
        }                                                        \
    } while (0)

#define SPDLOG_LOGGER_EVERY_N(logger, level, N, ...)                                             \
    do {                                                                                         \
        static std::atomic<unsigned long> spdlog_every_n_counter_{0};                            \
        const long spdlog_every_n_interval_ = static_cast<long>(N);                              \
        const unsigned long spdlog_every_n_count_ =                                              \
            spdlog_every_n_counter_.fetch_add(1, std::memory_order_relaxed);                     \
        if (spdlog_every_n_interval_ <= 1 ||                                                     \
            spdlog_every_n_count_ % static_cast<unsigned long>(spdlog_every_n_interval_) == 0) { \
            SPDLOG_LOGGER_CALL(logger, level, __VA_ARGS__);                                      \
        }                                                                                        \
    } while (0)

#define SPDLOG_LOGGER_THROTTLE(logger, level, interval_sec, ...)                             \
    do {                                                                                     \
        static std::atomic<long long> spdlog_throttle_last_{                                 \
            (std::numeric_limits<long long>::min)()}; /* sentinel: never logged yet */       \
        const double spdlog_throttle_interval_ = static_cast<double>(interval_sec);          \
        bool spdlog_throttle_fire_ = false;                                                  \
        if (spdlog_throttle_interval_ <= 0.0) {                                              \
            spdlog_throttle_fire_ = true;                                                    \
        } else {                                                                             \
            const long long spdlog_throttle_now_ =                                           \
                std::chrono::duration_cast<std::chrono::nanoseconds>(                        \
                    std::chrono::steady_clock::now().time_since_epoch())                     \
                    .count();                                                                \
            const long long spdlog_throttle_gap_ =                                           \
                static_cast<long long>(spdlog_throttle_interval_ * 1e9);                     \
            long long spdlog_throttle_prev_ =                                                \
                spdlog_throttle_last_.load(std::memory_order_relaxed);                       \
            if (spdlog_throttle_prev_ == (std::numeric_limits<long long>::min)() ||          \
                spdlog_throttle_now_ - spdlog_throttle_prev_ >= spdlog_throttle_gap_) {      \
                spdlog_throttle_fire_ = spdlog_throttle_last_.compare_exchange_strong(       \
                    spdlog_throttle_prev_, spdlog_throttle_now_, std::memory_order_relaxed); \
            }                                                                                \
        }                                                                                    \
        if (spdlog_throttle_fire_) {                                                         \
            SPDLOG_LOGGER_CALL(logger, level, __VA_ARGS__);                                  \
        }                                                                                    \
    } while (0)
