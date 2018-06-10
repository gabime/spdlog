//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#define SPDLOG_VERSION "0.17.0"

#include "tweakme.h"

#include <atomic>
#include <chrono>
#include <exception>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>

#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
#include <codecvt>
#include <locale>
#endif

#include "details/null_mutex.h"

// visual studio upto 2013 does not support noexcept nor constexpr
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define SPDLOG_NOEXCEPT throw()
#define SPDLOG_CONSTEXPR
#else
#define SPDLOG_NOEXCEPT noexcept
#define SPDLOG_CONSTEXPR constexpr
#endif

// final keyword support. On by default. See tweakme.h
#if defined(SPDLOG_NO_FINAL)
#define SPDLOG_FINAL
#else
#define SPDLOG_FINAL final
#endif

#if defined(__GNUC__) || defined(__clang__)
#define SPDLOG_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define SPDLOG_DEPRECATED __declspec(deprecated)
#else
#define SPDLOG_DEPRECATED
#endif

#include "fmt/fmt.h"

namespace spdlog {

class formatter;

namespace sinks {
class sink;
}

using log_clock = std::chrono::system_clock;
using sink_ptr = std::shared_ptr<sinks::sink>;
using sinks_init_list = std::initializer_list<sink_ptr>;
using formatter_ptr = std::shared_ptr<spdlog::formatter>;
#if defined(SPDLOG_NO_ATOMIC_LEVELS)
using level_t = details::null_atomic_int;
#else
using level_t = std::atomic<int>;
#endif

using log_err_handler = std::function<void(const std::string &err_msg)>;

// Log level enum
namespace level {
enum level_enum
{
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    err = 4,
    critical = 5,
    off = 6
};

#if !defined(SPDLOG_LEVEL_NAMES)
#define SPDLOG_LEVEL_NAMES                                                                                                                 \
    {                                                                                                                                      \
        "trace", "debug", "info", "warning", "error", "critical", "off"                                                                    \
    }
#endif
static const char *level_names[] SPDLOG_LEVEL_NAMES;

static const char *short_level_names[]{"T", "D", "I", "W", "E", "C", "O"};

inline const char *to_str(spdlog::level::level_enum l)
{
    return level_names[l];
}

inline const char *to_short_str(spdlog::level::level_enum l)
{
    return short_level_names[l];
}
inline spdlog::level::level_enum from_str(const std::string &name)
{
    static std::unordered_map<std::string, level_enum> name_to_level = // map string->level
        {{level_names[0], level::trace},                               // trace
            {level_names[1], level::debug},                            // debug
            {level_names[2], level::info},                             // info
            {level_names[3], level::warn},                             // warn
            {level_names[4], level::err},                              // err
            {level_names[5], level::critical},                         // critical
            {level_names[6], level::off}};                             // off

    auto lvl_it = name_to_level.find(name);
    return lvl_it != name_to_level.end() ? lvl_it->second : level::off;
}

using level_hasher = std::hash<int>;
} // namespace level

//
// Async overflow policy - block by default.
//
enum class async_overflow_policy
{
    block_retry,    // Block / yield / sleep until message can be enqueued
    discard_log_msg // Discard the message it enqueue fails
};

//
// Pattern time - specific time getting to use for pattern_formatter.
// local time by default
//
enum class pattern_time_type
{
    local, // log localtime
    utc    // log utc
};

//
// Log exception
//
class spdlog_ex : public std::exception
{
public:
    explicit spdlog_ex(std::string msg)
        : _msg(std::move(msg))
    {
    }

    spdlog_ex(const std::string &msg, int last_errno)
    {
        fmt::MemoryWriter writer;
        fmt::format_system_error(writer, last_errno, msg);
        _msg = writer.str();
    }

    const char *what() const SPDLOG_NOEXCEPT override
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

#define SPDLOG_CATCH_AND_HANDLE                                                                                                            \
    catch (const std::exception &ex)                                                                                                       \
    {                                                                                                                                      \
        _err_handler(ex.what());                                                                                                           \
    }                                                                                                                                      \
    catch (...)                                                                                                                            \
    {                                                                                                                                      \
        _err_handler("Unknown exeption in logger");                                                                                        \
    }
} // namespace spdlog
