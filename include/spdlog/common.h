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
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
#include <codecvt>
#include <locale>
#endif

#include <spdlog/details/null_mutex.h>

//visual studio does not support noexcept yet
#ifndef _MSC_VER
#define SPDLOG_NOEXCEPT noexcept
#else
#define SPDLOG_NOEXCEPT throw()
#endif

namespace spdlog
{

class formatter;

namespace sinks
{
class sink;
}

// Common types across the lib
using log_clock = std::chrono::system_clock;
using sink_ptr = std::shared_ptr < sinks::sink >;
using sinks_init_list = std::initializer_list < sink_ptr >;
using formatter_ptr = std::shared_ptr<spdlog::formatter>;
#if defined(SPDLOG_NO_ATOMIC_LEVELS)
using level_t = details::null_atomic_int;
#else
using level_t = std::atomic_int;
#endif

//Log level enum
namespace level
{
typedef enum
{
    trace = 0,
    debug = 1,
    info = 2,
    notice = 3,
    warn = 4,
    err = 5,
    critical = 6,
    alert = 7,
    emerg = 8,
    off = 9
} level_enum;

static const char* level_names[] { "trace", "debug", "info", "notice", "warning", "error", "critical", "alert", "emerg", "off"};

static const char* short_level_names[] { "T", "D", "I", "N", "W", "E", "C", "A", "M", "O"};

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
class spdlog_ex : public std::exception
{
public:
    spdlog_ex(const std::string& msg) :_msg(msg) {}
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
#define SPDLOG_FILENAME_T(s) L ## s
using filename_t = std::wstring;
inline std::string filename_to_str(const filename_t& filename)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> c;
    return c.to_bytes(filename);
}
#else
#define SPDLOG_FILENAME_T(s) s
using filename_t = std::string;

inline std::string filename_to_str(const filename_t& filename)
{
    return filename;
}
#endif

} //spdlog
