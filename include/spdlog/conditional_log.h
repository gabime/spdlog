#pragma once

#include <spdlog/spdlog.h>
#include <atomic>
#include <chrono>

SPDLOG_NAMESPACE_BEGIN
namespace details {

// Fires on every n-th call (first call included). n <= 1 fires every call.
struct every_n_state {
    std::atomic<unsigned long long> count{0};
    bool should_log(long long n) noexcept {
        if (n <= 1) {
            return true;
        }
        const auto c = count.fetch_add(1, std::memory_order_relaxed);
        return c % static_cast<unsigned long long>(n) == 0;
    }
};

// Fires at most once per interval (first call always fires).
// Non-positive intervals fire every call. Intervals beyond the clock's range fire once.
struct throttle_state {
    using clock = std::chrono::steady_clock;
    std::atomic<clock::time_point> next{(clock::time_point::min)()};

    bool should_log(std::chrono::milliseconds interval) noexcept {
        if (interval.count() <= 0) {
            return true;
        }
        const auto now = clock::now();
        auto expected = next.load(std::memory_order_relaxed);
        if (now < expected) {
            return false;
        }
        // Saturate at time_point::max() instead of overflowing now + interval.
        const auto headroom =
            std::chrono::duration_cast<std::chrono::milliseconds>((clock::time_point::max)() - now);
        const auto next_tp = interval < headroom
                                 ? now + std::chrono::duration_cast<clock::duration>(interval)
                                 : (clock::time_point::max)();
        return next.compare_exchange_strong(expected, next_tp, std::memory_order_relaxed);
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
// The throttle interval is std::chrono::milliseconds; coarser durations
// (seconds, minutes, ...) convert implicitly. Non-positive intervals fire on
// every call; intervals beyond the clock's range fire only once.
//

#define SPDLOG_LOGGER_EVERY_N(logger, level, n, ...)                         \
    do {                                                                     \
        if (SPDLOG_ACTIVE_LEVEL <= (level) && (logger)->should_log(level)) { \
            static SPDLOG_NAMESPACE::details::every_n_state spdlog_every_n_; \
            if (spdlog_every_n_.should_log(static_cast<long long>(n))) {     \
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
