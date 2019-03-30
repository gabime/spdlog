// Copyright(c) 2015-present Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlite.h"
namespace spdlite
{
//
// spdlite namespace functions - forward the calls to the default_logger.
//
spdlite::logger &default_logger();

template<typename... Args>
inline void trace(const char *fmt, const Args &... args)
{
    default_logger().trace(fmt, args...);
}

template<typename... Args>
inline void debug(const char *fmt, const Args &... args)
{
    default_logger().debug(fmt, args...);
}

template<typename... Args>
inline void info(const char *fmt, const Args &... args)
{
    default_logger().info(fmt, args...);
}

template<typename... Args>
inline void warn(const char *fmt, const Args &... args)
{
    default_logger().warn(fmt, args...);
}

template<typename... Args>
inline void error(const char *fmt, const Args &... args)
{
    default_logger().error(fmt, args...);
}

template<typename... Args>
inline void critical(const char *fmt, const Args &... args)
{
    default_logger().critical(fmt, args...);
}

// string view convertable
template<typename T>
inline void trace(const T &msg)
{
    default_logger().trace(msg);
}

template<typename T>
inline void debug(const T &msg)
{
    default_logger().debug(msg);
}

template<typename T>
inline void info(const T &msg)
{
    default_logger().info(msg);
}

template<typename T>
inline void warn(const T &msg)
{
    default_logger().warn(msg);
}

template<typename T>
inline void error(const T &msg)
{
    default_logger().error(msg);
}

template<typename T>
inline void critical(const T &msg)
{
    default_logger().critical(msg);
}

void log_printf(spdlite::level lvl, const char *format, va_list args);
void trace_printf(const char *format, ...);
void debug_printf(const char *format, ...);
void info_printf(const char *format, ...);
void warn_printf(const char *format, ...);
void error_printf(const char *format, ...);
void critical_printf(const char *format, ...);

}
