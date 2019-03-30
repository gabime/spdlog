// Copyright(c) 2015-present Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
//
// enable/disable log calls at compile time according to global level.
//
// define SPDLITE_ACTIVE_LEVEL to one of those (before including lite.h):


#define SPDLITE_LEVEL_TRACE 0
#define SPDLITE_LEVEL_DEBUG 1
#define SPDLITE_LEVEL_INFO 2
#define SPDLITE_LEVEL_WARN 3
#define SPDLITE_LEVEL_ERROR 4
#define SPDLITE_LEVEL_CRITICAL 5
#define SPDLITE_LEVEL_OFF 6

#define SPDLITE_LOGGER_CALL(logger, level, ...) logger.log(level, __VA_ARGS__)

// default level is info
#ifndef SPDLITE_ACTIVE_LEVEL
#define SPDLITE_ACTIVE_LEVEL SPDLITE_LEVEL_INFO
#endif 

static_assert(SPDLITE_ACTIVE_LEVEL >= SPDLITE_LEVEL_TRACE && SPDLITE_ACTIVE_LEVEL <= SPDLITE_LEVEL_OFF, "SPDLITE_ACTIVE_LEVEL");


#if SPDLITE_ACTIVE_LEVEL == SPDLITE_LEVEL_TRACE
#define SPDLITE_LOGGER_TRACE(logger, ...) SPDLITE_LOGGER_CALL(logger, spdlite::level::trace, __VA_ARGS__)
#define SPDLITE_TRACE(...) SPDLITE_LOGGER_TRACE(spdlite::default_logger(), __VA_ARGS__)
#else
#define SPDLITE_LOGGER_TRACE(logger, ...) (void)0
#define SPDLITE_TRACE(...) (void)0
#endif

#if SPDLITE_ACTIVE_LEVEL <= SPDLITE_LEVEL_DEBUG
#define SPDLITE_LOGGER_DEBUG(logger, ...) SPDLITE_LOGGER_CALL(logger, spdlite::level::debug, __VA_ARGS__)
#define SPDLITE_DEBUG(...) SPDLITE_LOGGER_DEBUG(spdlite::default_logger(), __VA_ARGS__)
#else
#define SPDLITE_LOGGER_DEBUG(logger, ...) (void)0
#define SPDLITE_DEBUG(...) (void)0
#endif

#if SPDLITE_ACTIVE_LEVEL <= SPDLITE_LEVEL_INFO
#define SPDLITE_LOGGER_INFO(logger, ...) SPDLITE_LOGGER_CALL(logger, spdlite::level::info, __VA_ARGS__)
#define SPDLITE_INFO(...) SPDLITE_LOGGER_INFO(spdlite::default_logger(), __VA_ARGS__)
#else
#define SPDLITE_LOGGER_INFO(logger, ...) (void)0
#define SPDLITE_INFO(...) (void)0
#endif

#if SPDLITE_ACTIVE_LEVEL <= SPDLITE_LEVEL_WARN
#define SPDLITE_LOGGER_WARN(logger, ...) SPDLITE_LOGGER_CALL(logger, spdlite::level::warn, __VA_ARGS__)
#define SPDLITE_WARN(...) SPDLITE_LOGGER_WARN(spdlite::default_logger(), __VA_ARGS__)
#else
#define SPDLITE_LOGGER_WARN(logger, ...) (void)0
#define SPDLITE_WARN(...) (void)0
#endif

#if SPDLITE_ACTIVE_LEVEL <= SPDLITE_LEVEL_ERROR
#define SPDLITE_LOGGER_ERROR(logger, ...) SPDLITE_LOGGER_CALL(logger, spdlite::level::err, __VA_ARGS__)
#define SPDLITE_ERROR(...) SPDLITE_LOGGER_ERROR(spdlite::default_logger(), __VA_ARGS__)
#else
#define SPDLITE_LOGGER_ERROR(logger, ...) (void)0
#define SPDLITE_ERROR(...) (void)0
#endif

#if SPDLITE_ACTIVE_LEVEL <= SPDLITE_LEVEL_CRITICAL
#define SPDLITE_LOGGER_CRITICAL(logger, ...) SPDLITE_LOGGER_CALL(logger, spdlite::level::critical, __VA_ARGS__)
#define SPDLITE_CRITICAL(...) SPDLITE_LOGGER_CRITICAL(spdlite::default_logger(), __VA_ARGS__)
#else
#define SPDLITE_LOGGER_CRITICAL(logger, ...) (void)0
#define SPDLITE_CRITICAL(...) (void)0
#endif
