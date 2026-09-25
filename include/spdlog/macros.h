// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// spdlog logging macros.
//
// This header contains no declarations and includes no spdlog, fmt or standard
// library headers, so that users of the C++20 named module can write
//
//     #include <spdlog/macros.h>
//     import spdlog;
//
// without pulling spdlog's headers into their translation unit. It is included
// by <spdlog/common.h>, so users of the headers get it exactly as before.

#ifndef SPDLOG_MACROS_H
#define SPDLOG_MACROS_H

#pragma once

#include <spdlog/namespace.h>
#include <spdlog/tweakme.h>

#ifndef SPDLOG_FUNCTION
#define SPDLOG_FUNCTION static_cast<const char *>(__FUNCTION__)
#endif

#define SPDLOG_LEVEL_TRACE 0
#define SPDLOG_LEVEL_DEBUG 1
#define SPDLOG_LEVEL_INFO 2
#define SPDLOG_LEVEL_WARN 3
#define SPDLOG_LEVEL_ERROR 4
#define SPDLOG_LEVEL_CRITICAL 5
#define SPDLOG_LEVEL_OFF 6

#if !defined(SPDLOG_ACTIVE_LEVEL)
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif

//
// enable/disable log calls at compile time according to global level.
//
// define SPDLOG_ACTIVE_LEVEL to one of those (before including spdlog.h):
// SPDLOG_LEVEL_TRACE,
// SPDLOG_LEVEL_DEBUG,
// SPDLOG_LEVEL_INFO,
// SPDLOG_LEVEL_WARN,
// SPDLOG_LEVEL_ERROR,
// SPDLOG_LEVEL_CRITICAL,
// SPDLOG_LEVEL_OFF
//

#ifndef SPDLOG_NO_SOURCE_LOC
#define SPDLOG_LOGGER_CALL(logger, level, ...) \
    (logger)->log(SPDLOG_NAMESPACE::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, level, __VA_ARGS__)
#else
#define SPDLOG_LOGGER_CALL(logger, level, ...) \
    (logger)->log(SPDLOG_NAMESPACE::source_loc{}, level, __VA_ARGS__)
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE
#define SPDLOG_LOGGER_TRACE(logger, ...) \
    SPDLOG_LOGGER_CALL(logger, SPDLOG_NAMESPACE::level::trace, __VA_ARGS__)
#define SPDLOG_TRACE(...) SPDLOG_LOGGER_TRACE(SPDLOG_NAMESPACE::default_logger_raw(), __VA_ARGS__)
#else
#define SPDLOG_LOGGER_TRACE(logger, ...) (void)0
#define SPDLOG_TRACE(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
#define SPDLOG_LOGGER_DEBUG(logger, ...) \
    SPDLOG_LOGGER_CALL(logger, SPDLOG_NAMESPACE::level::debug, __VA_ARGS__)
#define SPDLOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(SPDLOG_NAMESPACE::default_logger_raw(), __VA_ARGS__)
#else
#define SPDLOG_LOGGER_DEBUG(logger, ...) (void)0
#define SPDLOG_DEBUG(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_INFO
#define SPDLOG_LOGGER_INFO(logger, ...) SPDLOG_LOGGER_CALL(logger, SPDLOG_NAMESPACE::level::info, __VA_ARGS__)
#define SPDLOG_INFO(...) SPDLOG_LOGGER_INFO(SPDLOG_NAMESPACE::default_logger_raw(), __VA_ARGS__)
#else
#define SPDLOG_LOGGER_INFO(logger, ...) (void)0
#define SPDLOG_INFO(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_WARN
#define SPDLOG_LOGGER_WARN(logger, ...) SPDLOG_LOGGER_CALL(logger, SPDLOG_NAMESPACE::level::warn, __VA_ARGS__)
#define SPDLOG_WARN(...) SPDLOG_LOGGER_WARN(SPDLOG_NAMESPACE::default_logger_raw(), __VA_ARGS__)
#else
#define SPDLOG_LOGGER_WARN(logger, ...) (void)0
#define SPDLOG_WARN(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_ERROR
#define SPDLOG_LOGGER_ERROR(logger, ...) SPDLOG_LOGGER_CALL(logger, SPDLOG_NAMESPACE::level::err, __VA_ARGS__)
#define SPDLOG_ERROR(...) SPDLOG_LOGGER_ERROR(SPDLOG_NAMESPACE::default_logger_raw(), __VA_ARGS__)
#else
#define SPDLOG_LOGGER_ERROR(logger, ...) (void)0
#define SPDLOG_ERROR(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_CRITICAL
#define SPDLOG_LOGGER_CRITICAL(logger, ...) \
    SPDLOG_LOGGER_CALL(logger, SPDLOG_NAMESPACE::level::critical, __VA_ARGS__)
#define SPDLOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(SPDLOG_NAMESPACE::default_logger_raw(), __VA_ARGS__)
#else
#define SPDLOG_LOGGER_CRITICAL(logger, ...) (void)0
#define SPDLOG_CRITICAL(...) (void)0
#endif

#endif  // SPDLOG_MACROS_H
