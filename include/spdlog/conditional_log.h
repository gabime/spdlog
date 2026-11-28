#pragma once

#include "spdlog.h"
#include <atomic>
#include <chrono>
#include <cmath>

SPDLOG_NAMESPACE_BEGIN
namespace details {

// Fires on every N-th call (first call included). N <= 1 fires every call.
struct every_n_state {
    std::atomic<unsigned long> count{0};
    bool should_log(unsigned long n) noexcept {
        return n == 0 || count++ % n == 0;
    }
};

// Fires at most once per interval (first call always fires).
// Non-positive intervals fire every call.
struct throttle_state {
    using clock = std::chrono::steady_clock;
    std::atomic<clock::time_point> next{clock::time_point{}};

    template <typename Rep, typename Period>
    bool should_log(const std::chrono::duration<Rep, Period>& interval) noexcept {
        using namespace std::chrono;
        auto now = clock::now();
        auto expected = next.load();

        // Non-positive intervals fire every call.
        if (interval.count() <= 0) {
            return now >= expected && next.compare_exchange_strong(expected, now);
        }

        // Guard against overflow: convert to double seconds and check range
        // before casting to nanoseconds, which is UB for out-of-range values
        // (e.g. infinity, NaN, or intervals beyond ~292 years).
        double interval_sec = duration<double>(interval).count();
        double max_sec = duration<double>(clock::duration::max()).count();
        if (!std::isfinite(interval_sec) || interval_sec > max_sec) {
            return now >= expected && next.compare_exchange_strong(expected, now);
        }

        auto interval_ns = duration_cast<clock::duration>(interval);
        return now >= expected && next.compare_exchange_strong(expected, now + interval_ns);
    }
};

}  // namespace details
SPDLOG_NAMESPACE_END

//
// Conditional log macros.
//
// Each macro checks SPDLOG_ACTIVE_LEVEL (compile-time) and
// logger->should_log(level) (runtime) before touching the helper state, so a
// message filtered by either level never consumes a counter/clock slot.
// The interval is a std::chrono::duration; non-positive, non-finite, or
// out-of-range intervals (beyond the nanosecond range, ~292 years) are
// treated like a zero interval: the call fires on every invocation, with no
// undefined behavior from the conversion.
//

#define SPDLOG_LOGGER_EVERY_N(logger, level, n, ...)                         \
    do {                                                                     \
        if (SPDLOG_ACTIVE_LEVEL <= (level) && (logger)->should_log(level)) { \
            static SPDLOG_NAMESPACE::details::every_n_state spdlog_every_n_; \
            if (spdlog_every_n_.should_log(static_cast<unsigned long>(n))) { \
                SPDLOG_LOGGER_CALL(logger, level, __VA_ARGS__);              \
            }                                                                \
        }                                                                    \
    } while (0)

#define SPDLOG_LOGGER_THROTTLE(logger, level, interval, ...)                   \
    do {                                                                       \
        if (SPDLOG_ACTIVE_LEVEL <= (level) && (logger)->should_log(level)) {   \
            static SPDLOG_NAMESPACE::details::throttle_state spdlog_throttle_; \
            if (spdlog_throttle_.should_log(interval)) {                       \
                SPDLOG_LOGGER_CALL(logger, level, __VA_ARGS__);                \
            }                                                                  \
        }                                                                      \
    } while (0)

//
// Per-level every_n macros. Compiled out completely when the level is above
// SPDLOG_ACTIVE_LEVEL.
//

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE
#define SPDLOG_LOGGER_TRACE_EVERY_N(logger, n, ...) \
    SPDLOG_LOGGER_EVERY_N(logger, SPDLOG_NAMESPACE::level::trace, n, __VA_ARGS__)
#define SPDLOG_LOGGER_TRACE_THROTTLE(logger, interval, ...) \
    SPDLOG_LOGGER_THROTTLE(logger, SPDLOG_NAMESPACE::level::trace, interval, __VA_ARGS__)
#define SPDLOG_TRACE_EVERY_N(n, ...) \
    SPDLOG_LOGGER_TRACE_EVERY_N(SPDLOG_NAMESPACE::default_logger_raw(), n, __VA_ARGS__)
#define SPDLOG_TRACE_THROTTLE(interval, ...) \
    SPDLOG_LOGGER_TRACE_THROTTLE(SPDLOG_NAMESPACE::default_logger_raw(), interval, __VA_ARGS__)
#else
#define SPDLOG_LOGGER_TRACE_EVERY_N(logger, n, ...) (void)0
#define SPDLOG_LOGGER_TRACE_THROTTLE(logger, interval, ...) (void)0
#define SPDLOG_TRACE_EVERY_N(n, ...) (void)0
#define SPDLOG_TRACE_THROTTLE(interval, ...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
#define SPDLOG_LOGGER_DEBUG_EVERY_N(logger, n, ...) \
    SPDLOG_LOGGER_EVERY_N(logger, SPDLOG_NAMESPACE::level::debug, n, __VA_ARGS__)
#define SPDLOG_LOGGER_DEBUG_THROTTLE(logger, interval, ...) \
    SPDLOG_LOGGER_THROTTLE(logger, SPDLOG_NAMESPACE::level::debug, interval, __VA_ARGS__)
#define SPDLOG_DEBUG_EVERY_N(n, ...) \
    SPDLOG_LOGGER_DEBUG_EVERY_N(SPDLOG_NAMESPACE::default_logger_raw(), n, __VA_ARGS__)
#define SPDLOG_DEBUG_THROTTLE(interval, ...) \
    SPDLOG_LOGGER_DEBUG_THROTTLE(SPDLOG_NAMESPACE::default_logger_raw(), interval, __VA_ARGS__)
#else
#define SPDLOG_LOGGER_DEBUG_EVERY_N(logger, n, ...) (void)0
#define SPDLOG_LOGGER_DEBUG_THROTTLE(logger, interval, ...) (void)0
#define SPDLOG_DEBUG_EVERY_N(n, ...) (void)0
#define SPDLOG_DEBUG_THROTTLE(interval, ...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_INFO
#define SPDLOG_LOGGER_INFO_EVERY_N(logger, n, ...) \
    SPDLOG_LOGGER_EVERY_N(logger, SPDLOG_NAMESPACE::level::info, n, __VA_ARGS__)
#define SPDLOG_LOGGER_INFO_THROTTLE(logger, interval, ...) \
    SPDLOG_LOGGER_THROTTLE(logger, SPDLOG_NAMESPACE::level::info, interval, __VA_ARGS__)
#define SPDLOG_INFO_EVERY_N(n, ...) \
    SPDLOG_LOGGER_INFO_EVERY_N(SPDLOG_NAMESPACE::default_logger_raw(), n, __VA_ARGS__)
#define SPDLOG_INFO_THROTTLE(interval, ...) \
    SPDLOG_LOGGER_INFO_THROTTLE(SPDLOG_NAMESPACE::default_logger_raw(), interval, __VA_ARGS__)
#else
#define SPDLOG_LOGGER_INFO_EVERY_N(logger, n, ...) (void)0
#define SPDLOG_LOGGER_INFO_THROTTLE(logger, interval, ...) (void)0
#define SPDLOG_INFO_EVERY_N(n, ...) (void)0
#define SPDLOG_INFO_THROTTLE(interval, ...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_WARN
#define SPDLOG_LOGGER_WARN_EVERY_N(logger, n, ...) \
    SPDLOG_LOGGER_EVERY_N(logger, SPDLOG_NAMESPACE::level::warn, n, __VA_ARGS__)
#define SPDLOG_LOGGER_WARN_THROTTLE(logger, interval, ...) \
    SPDLOG_LOGGER_THROTTLE(logger, SPDLOG_NAMESPACE::level::warn, interval, __VA_ARGS__)
#define SPDLOG_WARN_EVERY_N(n, ...) \
    SPDLOG_LOGGER_WARN_EVERY_N(SPDLOG_NAMESPACE::default_logger_raw(), n, __VA_ARGS__)
#define SPDLOG_WARN_THROTTLE(interval, ...) \
    SPDLOG_LOGGER_WARN_THROTTLE(SPDLOG_NAMESPACE::default_logger_raw(), interval, __VA_ARGS__)
#else
#define SPDLOG_LOGGER_WARN_EVERY_N(logger, n, ...) (void)0
#define SPDLOG_LOGGER_WARN_THROTTLE(logger, interval, ...) (void)0
#define SPDLOG_WARN_EVERY_N(n, ...) (void)0
#define SPDLOG_WARN_THROTTLE(interval, ...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_ERROR
#define SPDLOG_LOGGER_ERROR_EVERY_N(logger, n, ...) \
    SPDLOG_LOGGER_EVERY_N(logger, SPDLOG_NAMESPACE::level::err, n, __VA_ARGS__)
#define SPDLOG_LOGGER_ERROR_THROTTLE(logger, interval, ...) \
    SPDLOG_LOGGER_THROTTLE(logger, SPDLOG_NAMESPACE::level::err, interval, __VA_ARGS__)
#define SPDLOG_ERROR_EVERY_N(n, ...) \
    SPDLOG_LOGGER_ERROR_EVERY_N(SPDLOG_NAMESPACE::default_logger_raw(), n, __VA_ARGS__)
#define SPDLOG_ERROR_THROTTLE(interval, ...) \
    SPDLOG_LOGGER_ERROR_THROTTLE(SPDLOG_NAMESPACE::default_logger_raw(), interval, __VA_ARGS__)
#else
#define SPDLOG_LOGGER_ERROR_EVERY_N(logger, n, ...) (void)0
#define SPDLOG_LOGGER_ERROR_THROTTLE(logger, interval, ...) (void)0
#define SPDLOG_ERROR_EVERY_N(n, ...) (void)0
#define SPDLOG_ERROR_THROTTLE(interval, ...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_CRITICAL
#define SPDLOG_LOGGER_CRITICAL_EVERY_N(logger, n, ...) \
    SPDLOG_LOGGER_EVERY_N(logger, SPDLOG_NAMESPACE::level::critical, n, __VA_ARGS__)
#define SPDLOG_LOGGER_CRITICAL_THROTTLE(logger, interval, ...) \
    SPDLOG_LOGGER_THROTTLE(logger, SPDLOG_NAMESPACE::level::critical, interval, __VA_ARGS__)
#define SPDLOG_CRITICAL_EVERY_N(n, ...) \
    SPDLOG_LOGGER_CRITICAL_EVERY_N(SPDLOG_NAMESPACE::default_logger_raw(), n, __VA_ARGS__)
#define SPDLOG_CRITICAL_THROTTLE(interval, ...) \
    SPDLOG_LOGGER_CRITICAL_THROTTLE(SPDLOG_NAMESPACE::default_logger_raw(), interval, __VA_ARGS__)
#else
#define SPDLOG_LOGGER_CRITICAL_EVERY_N(logger, n, ...) (void)0
#define SPDLOG_LOGGER_CRITICAL_THROTTLE(logger, interval, ...) (void)0
#define SPDLOG_CRITICAL_EVERY_N(n, ...) (void)0
#define SPDLOG_CRITICAL_THROTTLE(interval, ...) (void)0
#endif
