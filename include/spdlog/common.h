//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <string>
#include <initializer_list>
#include <chrono>
#include <memory>
#include <atomic>
#include <exception>
#include<functional>

#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
#include <codecvt>
#include <locale>
#endif

#include <spdlog/details/null_mutex.h>

//visual studio upto 2013 does not support noexcept nor constexpr
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define SPDLOG_NOEXCEPT throw()
#define SPDLOG_CONSTEXPR
#else
#define SPDLOG_NOEXCEPT noexcept
#define SPDLOG_CONSTEXPR constexpr
#endif

#if defined(__GNUC__)  || defined(__clang__)
#define SPDLOG_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define SPDLOG_DEPRECATED __declspec(deprecated)
#else
#define SPDLOG_DEPRECATED
#endif


#include <spdlog/fmt/fmt.h>

namespace spdlog
{

class formatter;

namespace sinks
{
class sink;
}

using log_clock = std::chrono::system_clock;
using sink_ptr = std::shared_ptr < sinks::sink >;
using sinks_init_list = std::initializer_list < sink_ptr >;
using formatter_ptr = std::shared_ptr<spdlog::formatter>;
#if defined(SPDLOG_NO_ATOMIC_LEVELS)
using level_t = details::null_atomic_int;
#else
using level_t = std::atomic<int>;
#endif

using log_err_handler = std::function<void(const std::string &err_msg)>;

//Log level enum
namespace level
{
typedef enum
{
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    err = 4,
    critical = 5,
    off = 6
} level_enum;

static const char* level_names[] { "trace", "debug", "info",  "warning", "error", "critical", "off" };

static const char* short_level_names[] { "T", "D", "I", "W", "E", "C", "O" };

inline const char* to_str(spdlog::level::level_enum l)
{
    return level_names[l];
}

inline const char* to_short_str(spdlog::level::level_enum l)
{
    return short_level_names[l];
}
} //level


//
// Async overflow policy - block by default.
//
enum class async_overflow_policy
{
    block_retry, // Block / yield / sleep until message can be enqueued
    discard_log_msg // Discard the message it enqueue fails
};


//
// Log exception
//
namespace details
{
namespace os
{
std::string errno_str(int err_num);
}
}
class spdlog_ex: public std::exception
{
public:
    spdlog_ex(const std::string& msg):_msg(msg)
    {}
    spdlog_ex(const std::string& msg, int last_errno)
    {
        _msg = msg + ": " + details::os::errno_str(last_errno);
    }
    const char* what() const SPDLOG_NOEXCEPT override
    {
        return _msg.c_str();
    }
private:
    std::string _msg;

};

//
// wchar support for windows file names (SPDLOG_WCHAR_FILENAMES must be defined)
//
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
using filename_t = std::wstring;
#else
using filename_t = std::string;
#endif


} //spdlog
