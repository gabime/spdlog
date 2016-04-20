//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
#pragma once
#include <type_traits>

#include <spdlog/details/line_logger_fwd.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>

#include <string>
#include <utility>

// Line logger class - aggregates operator<< calls to fast ostream
// and logs upon destruction

inline spdlog::details::line_logger::line_logger(logger* callback_logger, level::level_enum msg_level, bool enabled):
    _callback_logger(callback_logger),
    _log_msg(msg_level),
    _enabled(enabled)
{}

inline spdlog::details::line_logger::line_logger(line_logger&& other) :
    _callback_logger(other._callback_logger),
    _log_msg(std::move(other._log_msg)),
    _enabled(other._enabled)
{
    other.disable();
}

//Log the log message using the callback logger
inline spdlog::details::line_logger::~line_logger()
{
    if (_enabled)
    {
#ifndef SPDLOG_NO_NAME
        _log_msg.logger_name = _callback_logger->name();
#endif
#ifndef SPDLOG_NO_DATETIME
        _log_msg.time = os::now();
#endif

#ifndef SPDLOG_NO_THREAD_ID
        _log_msg.thread_id = os::thread_id();
#endif
        _callback_logger->_log_msg(_log_msg);
    }
}

//
// Support for format string with variadic args
//


inline void spdlog::details::line_logger::write(const char* what)
{
    if (_enabled)
        _log_msg.raw << what;
}

template <typename... Args>
inline void spdlog::details::line_logger::write(const char* fmt, const Args&... args)
{
    if (!_enabled)
        return;
    try
    {
        _log_msg.raw.write(fmt, args...);
    }
    catch (const fmt::FormatError& e)
    {
        throw spdlog_ex(fmt::format("formatting error while processing format string '{}': {}", fmt, e.what()));
    }
}


//
// Support for operator<<
//
inline spdlog::details::line_logger& spdlog::details::line_logger::operator<<(const char* what)
{
    if (_enabled)
        _log_msg.raw << what;
    return *this;
}

inline spdlog::details::line_logger& spdlog::details::line_logger::operator<<(const std::string& what)
{
    if (_enabled)
        _log_msg.raw << what;
    return *this;
}

inline spdlog::details::line_logger& spdlog::details::line_logger::operator<<(int what)
{
    if (_enabled)
        _log_msg.raw << what;
    return *this;
}

inline spdlog::details::line_logger& spdlog::details::line_logger::operator<<(unsigned int what)
{
    if (_enabled)
        _log_msg.raw << what;
    return *this;
}


inline spdlog::details::line_logger& spdlog::details::line_logger::operator<<(long what)
{
    if (_enabled)
        _log_msg.raw << what;
    return *this;
}

inline spdlog::details::line_logger& spdlog::details::line_logger::operator<<(unsigned long what)
{
    if (_enabled)
        _log_msg.raw << what;
    return *this;
}

inline spdlog::details::line_logger& spdlog::details::line_logger::operator<<(long long what)
{
    if (_enabled)
        _log_msg.raw << what;
    return *this;
}

inline spdlog::details::line_logger& spdlog::details::line_logger::operator<<(unsigned long long what)
{
    if (_enabled)
        _log_msg.raw << what;
    return *this;
}

inline spdlog::details::line_logger& spdlog::details::line_logger::operator<<(double what)
{
    if (_enabled)
        _log_msg.raw << what;
    return *this;
}

inline spdlog::details::line_logger& spdlog::details::line_logger::operator<<(long double what)
{
    if (_enabled)
        _log_msg.raw << what;
    return *this;
}

inline spdlog::details::line_logger& spdlog::details::line_logger::operator<<(float what)
{
    if (_enabled)
        _log_msg.raw << what;
    return *this;
}

inline spdlog::details::line_logger& spdlog::details::line_logger::operator<<(char what)
{
    if (_enabled)
        _log_msg.raw << what;
    return *this;
}

//Support user types which implements operator<<
template<typename T>
inline spdlog::details::line_logger& spdlog::details::line_logger::operator<<(const T& what)
{
    if (_enabled)
        _log_msg.raw.write("{}", what);
    return *this;
}


inline void spdlog::details::line_logger::disable()
{
    _enabled = false;
}

inline bool spdlog::details::line_logger::is_enabled() const
{
    return _enabled;
}

