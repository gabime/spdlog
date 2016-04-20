//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
#pragma once

#include <spdlog/common.h>
#include <spdlog/details/log_msg.h>

#include <string>

// Line logger class - aggregates operator<< calls to fast ostream
// and logs upon destruction

namespace spdlog
{

// Forward declaration
class logger;

namespace details
{
class line_logger
{
public:
    line_logger(logger* callback_logger, level::level_enum msg_level, bool enabled);

    // No copy intended. Only move
    line_logger(const line_logger& other) = delete;
    line_logger& operator=(const line_logger&) = delete;
    line_logger& operator=(line_logger&&) = delete;


    line_logger(line_logger&& other);

    //Log the log message using the callback logger
    ~line_logger();

    //
    // Support for format string with variadic args
    //


    void write(const char* what);

    template <typename... Args>
    void write(const char* fmt, const Args&... args);

    //
    // Support for operator<<
    //
    line_logger& operator<<(const char* what);
    line_logger& operator<<(const std::string& what);
    line_logger& operator<<(int what);
    line_logger& operator<<(unsigned int what);
    line_logger& operator<<(long what);
    line_logger& operator<<(unsigned long what);
    line_logger& operator<<(long long what);
    line_logger& operator<<(unsigned long long what);
    line_logger& operator<<(double what);
    line_logger& operator<<(long double what);
    line_logger& operator<<(float what);
    line_logger& operator<<(char what);
    //Support user types which implements operator<<
    template<typename T>
    line_logger& operator<<(const T& what);

    void disable();
    bool is_enabled() const;

private:
    logger* _callback_logger;
    log_msg _log_msg;
    bool _enabled;
};
} //Namespace details
} // Namespace spdlog

