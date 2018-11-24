//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/tweakme.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

#if defined(SPDLOG_WCHAR_FILENAMES) || defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT)
#include <codecvt>
#include <locale>
#endif

#include "spdlog/details/null_mutex.h"

// visual studio upto 2013 does not support noexcept nor constexpr
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define SPDLOG_NOEXCEPT throw()
#define SPDLOG_CONSTEXPR
#else
#define SPDLOG_NOEXCEPT noexcept
#define SPDLOG_CONSTEXPR constexpr
#endif

#if defined(__GNUC__) || defined(__clang__)
#define SPDLOG_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define SPDLOG_DEPRECATED __declspec(deprecated)
#else
#define SPDLOG_DEPRECATED
#endif

// disable thread local on msvc 2013
#ifndef SPDLOG_NO_TLS
#if (defined(_MSC_VER) && (_MSC_VER < 1900))
#define SPDLOG_NO_TLS 1
#endif
#endif

#include "spdlog/fmt/fmt.h"

namespace spdlog {

class formatter;

namespace sinks {
class sink;
}

using log_clock = std::chrono::system_clock;
using sink_ptr = std::shared_ptr<sinks::sink>;
using sinks_init_list = std::initializer_list<sink_ptr>;
using log_err_handler = std::function<void(const std::string &err_msg)>;

// string_view type - either std::string_view or fmt::string_view (pre c++17)
#if defined(FMT_USE_STD_STRING_VIEW)
using string_view_t = std::string_view;
#else
using string_view_t = fmt::string_view;
#endif

#if defined(SPDLOG_NO_ATOMIC_LEVELS)
using level_t = details::null_atomic_int;
#else
using level_t = std::atomic<int>;
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

// Log level enum
namespace level {
enum level_enum
{
    trace = SPDLOG_LEVEL_TRACE,
    debug = SPDLOG_LEVEL_DEBUG,
    info = SPDLOG_LEVEL_INFO,
    warn = SPDLOG_LEVEL_WARN,
    err = SPDLOG_LEVEL_ERROR,
    critical = SPDLOG_LEVEL_CRITICAL,
    off = SPDLOG_LEVEL_OFF,
};

#if !defined(SPDLOG_LEVEL_NAMES)
#define SPDLOG_LEVEL_NAMES                                                                                                                 \
    {                                                                                                                                      \
        "trace", "debug", "info", "warning", "error", "critical", "off"                                                                    \
    }
#endif

static string_view_t level_string_views[] SPDLOG_LEVEL_NAMES;
static const char *short_level_names[]{"T", "D", "I", "W", "E", "C", "O"};

inline string_view_t &to_string_view(spdlog::level::level_enum l) SPDLOG_NOEXCEPT
{
    return level_string_views[l];
}

inline const char *to_short_c_str(spdlog::level::level_enum l) SPDLOG_NOEXCEPT
{
    return short_level_names[l];
}

inline spdlog::level::level_enum from_str(const std::string &name) SPDLOG_NOEXCEPT
{
    int level = 0;
    for (const auto &level_str : level_string_views)
    {
        if (level_str == name)
        {
            return static_cast<level::level_enum>(level);
        }
        level++;
    }
    return level::off;
}

using level_hasher = std::hash<int>;
} // namespace level

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
        : msg_(std::move(msg))
    {
    }

    spdlog_ex(const std::string &msg, int last_errno)
    {
        fmt::memory_buffer outbuf;
        fmt::format_system_error(outbuf, last_errno, msg);
        msg_ = fmt::to_string(outbuf);
    }

    const char *what() const SPDLOG_NOEXCEPT override
    {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

//
// wchar support for windows file names (SPDLOG_WCHAR_FILENAMES must be defined)
//
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
using filename_t = std::wstring;
#else
using filename_t = std::string;
#endif

struct source_loc
{
    SPDLOG_CONSTEXPR source_loc()
        : filename{""}
        , line{0}
    {
    }
    SPDLOG_CONSTEXPR source_loc(const char *filename, int line)
        : filename(filename)
        , line(line)
    {
    }

    SPDLOG_CONSTEXPR bool empty() const SPDLOG_NOEXCEPT
    {
        return line == 0;
    }
    const char *filename;
    uint32_t line;
};

namespace details {
// make_unique support for pre c++14

#if __cplusplus >= 201402L // C++14 and beyond
using std::make_unique;
#else
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args)
{
    static_assert(!std::is_array<T>::value, "arrays not supported");
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif
} // namespace details
} // namespace spdlog
