// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>

#include "./namespace.h"
#include "./source_loc.h"
#include "fmt/base.h"
#include "fmt/xchar.h"

// Define SPDLOG_API according to current build settings
#ifndef SPDLOG_SHARED_LIB
    #define SPDLOG_API
#elif defined(_WIN32)
    #ifdef spdlog_EXPORTS
        #define SPDLOG_API __declspec(dllexport) // Export symbols when building the library
    #else
        #define SPDLOG_API __declspec(dllimport) // Import symbols when using the library
    #endif
#elif (defined(__GNUC__) || defined(__clang__))
    #define SPDLOG_API __attribute__((visibility("default"))) // Export symbols for shared libraries
#else
    #define SPDLOG_API
#endif
// End of SPDLOG_API definition

#define SPDLOG_FMT_RUNTIME(format_string) fmt::runtime(format_string)
#define SPDLOG_FMT_STRING(format_string) FMT_STRING(format_string)

#ifndef SPDLOG_FUNCTION
    #define SPDLOG_FUNCTION static_cast<const char *>(__FUNCTION__)
#endif

SPDLOG_NAMESPACE_BEGIN

class formatter;

namespace sinks {
class sink;
}

using log_clock = std::chrono::system_clock;
using sink_ptr = std::shared_ptr<sinks::sink>;
using sinks_init_list = std::initializer_list<sink_ptr>;
using err_handler = std::function<void(const std::string &err_msg)>;
using string_view_t = std::basic_string_view<char>;
using wstring_view_t = std::basic_string_view<wchar_t>;

namespace fmt_lib = fmt;
using memory_buf_t = fmt::basic_memory_buffer<char, 250>;
using wmemory_buf_t = fmt::basic_memory_buffer<wchar_t, 250>;

template <typename... Args>
using format_string_t = fmt::format_string<Args...>;

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
enum class level : std::uint8_t {
    trace = SPDLOG_LEVEL_TRACE,
    debug = SPDLOG_LEVEL_DEBUG,
    info = SPDLOG_LEVEL_INFO,
    warn = SPDLOG_LEVEL_WARN,
    err = SPDLOG_LEVEL_ERROR,
    critical = SPDLOG_LEVEL_CRITICAL,
    off = SPDLOG_LEVEL_OFF,
    n_levels = SPDLOG_LEVEL_OFF + 1
};

using atomic_level_t = std::atomic<level>;

[[nodiscard]] constexpr size_t level_to_number(level lvl) noexcept { return static_cast<size_t>(lvl); }

constexpr auto levels_count = level_to_number(level::n_levels);
constexpr std::array<std::string_view, levels_count> level_string_views{"trace", "debug",    "info", "warning",
                                                                        "error", "critical", "off"};
constexpr std::array<std::string_view, levels_count> short_level_names{"T", "D", "I", "W", "E", "C", "O"};

[[nodiscard]] constexpr std::string_view to_string_view(level lvl) noexcept {
    return level_string_views.at(level_to_number(lvl));
}

[[nodiscard]] constexpr std::string_view to_short_string_view(level lvl) noexcept {
    return short_level_names.at(level_to_number(lvl));
}

[[nodiscard]] SPDLOG_API level level_from_str(const std::string &name) noexcept;

//
// Color mode used by sinks with color support.
//
enum class color_mode { always, automatic, never };

//
// Pattern time - specific time getting to use for pattern_formatter.
// local time by default
//
enum class pattern_time_type {
    local,  // log localtime
    utc     // log utc
};

//
// Log exception
//
class SPDLOG_API spdlog_ex : public std::exception {
public:
    explicit spdlog_ex(std::string msg);

    spdlog_ex(const std::string &msg, int last_errno);

    [[nodiscard]] const char *what() const noexcept override;

private:
    std::string msg_;
};

[[noreturn]] SPDLOG_API void throw_spdlog_ex(const std::string &msg, int last_errno);
[[noreturn]] SPDLOG_API void throw_spdlog_ex(std::string msg);

SPDLOG_NAMESPACE_END
