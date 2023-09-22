// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/tweakme.h>
#include <spdlog/details/null_mutex.h>

#include <atomic>
#include <chrono>
#include <initializer_list>
#include <memory>
#include <exception>
#include <string>
#include <type_traits>
#include <functional>
#include <cstdio>
#include <cstdint>
#include <array>

#if __has_include(<version>)
#    include <version>
#endif

#if __cpp_lib_source_location >= 201907
#    include <source_location>
#    define SPDLOG_STD_SOURCE_LOCATION
#elif __has_include(<experimental/source_location>)
#    include <experimental/source_location>
#    define SPDLOG_EXPERIMENTAL_SOURCE_LOCATION
#endif

#ifdef SPDLOG_USE_STD_FORMAT
#    if __cpp_lib_format >= 202207L
#        include <format>
#    else
#        include <string_view>
#    endif
#endif

#ifdef SPDLOG_COMPILED_LIB
#    undef SPDLOG_HEADER_ONLY
#    if defined(SPDLOG_SHARED_LIB)
#        if defined(_WIN32)
#            ifdef spdlog_EXPORTS
#                define SPDLOG_API __declspec(dllexport)
#            else // !spdlog_EXPORTS
#                define SPDLOG_API __declspec(dllimport)
#            endif
#        else // !defined(_WIN32)
#            define SPDLOG_API __attribute__((visibility("default")))
#        endif
#    else // !defined(SPDLOG_SHARED_LIB)
#        define SPDLOG_API
#    endif
#    define SPDLOG_INLINE
#else // !defined(SPDLOG_COMPILED_LIB)
#    define SPDLOG_API
#    define SPDLOG_HEADER_ONLY
#    define SPDLOG_INLINE inline
#endif // #ifdef SPDLOG_COMPILED_LIB

#include <spdlog/fmt/fmt.h>

#if !defined(SPDLOG_USE_STD_FORMAT) && FMT_VERSION >= 80000 // backward compatibility with fmt versions older than 8
#    define SPDLOG_FMT_RUNTIME(format_string) fmt::runtime(format_string)
#    if defined(SPDLOG_WCHAR_FILENAMES)
#        include <spdlog/fmt/xchar.h>
#    endif
#else
#    define SPDLOG_FMT_RUNTIME(format_string) format_string
#endif

#ifndef SPDLOG_FUNCTION
#    define SPDLOG_FUNCTION static_cast<const char *>(__FUNCTION__)
#endif

#ifdef SPDLOG_NO_EXCEPTIONS
#    define SPDLOG_TRY
#    define SPDLOG_THROW(ex)                                                                                                               \
        do                                                                                                                                 \
        {                                                                                                                                  \
            printf("spdlog fatal error: %s\n", ex.what());                                                                                 \
            std::abort();                                                                                                                  \
        } while (0)
#    define SPDLOG_CATCH_STD
#else
#    define SPDLOG_TRY try
#    define SPDLOG_THROW(ex) throw(ex)
#    define SPDLOG_CATCH_STD                                                                                                               \
        catch (const std::exception &)                                                                                                     \
        {}
#endif

namespace spdlog {

class formatter;

namespace sinks {
class sink;
}

#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
using filename_t = std::wstring;
// allow macro expansion to occur in SPDLOG_FILENAME_T
#    define SPDLOG_FILENAME_T_INNER(s) L##s
#    define SPDLOG_FILENAME_T(s) SPDLOG_FILENAME_T_INNER(s)
#else
using filename_t = std::string;
#    define SPDLOG_FILENAME_T(s) s
#endif

using log_clock = std::chrono::system_clock;
using sink_ptr = std::shared_ptr<sinks::sink>;
using sinks_init_list = std::initializer_list<sink_ptr>;
using err_handler = std::function<void(const std::string &err_msg)>;

#ifdef SPDLOG_USE_STD_FORMAT
namespace fmt_lib = std;
using string_view_t = std::string_view;
using memory_buf_t = std::string;
using wstring_view_t = std::wstring_view;
using wmemory_buf_t = std::wstring;

template<typename... Args>
#    if __cpp_lib_format >= 202207L
using format_string_t = std::format_string<Args...>;
#    else
using format_string_t = std::string_view;
#    endif

#    define SPDLOG_BUF_TO_STRING(x) x
#else // use fmt lib instead of std::format
namespace fmt_lib = fmt;

using string_view_t = fmt::basic_string_view<char>;
using memory_buf_t = fmt::basic_memory_buffer<char, 250>;
template<typename... Args>
using format_string_t = fmt::format_string<Args...>;
using wstring_view_t = fmt::basic_string_view<wchar_t>;
using wmemory_buf_t = fmt::basic_memory_buffer<wchar_t, 250>;
#    define SPDLOG_BUF_TO_STRING(x) fmt::to_string(x)
#endif // SPDLOG_USE_STD_FORMAT


#define SPDLOG_LEVEL_TRACE 0
#define SPDLOG_LEVEL_DEBUG 1
#define SPDLOG_LEVEL_INFO 2
#define SPDLOG_LEVEL_WARN 3
#define SPDLOG_LEVEL_ERROR 4
#define SPDLOG_LEVEL_CRITICAL 5
#define SPDLOG_LEVEL_OFF 6

#if !defined(SPDLOG_ACTIVE_LEVEL)
#    define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif

// Is convertable to string_view_t ?
template<typename T>
using is_convertible_to_sv = std::enable_if_t<std::is_convertible_v<T, string_view_t>>;

// Log level enum
enum class log_level
{
    trace = SPDLOG_LEVEL_TRACE,
    debug = SPDLOG_LEVEL_DEBUG,
    info = SPDLOG_LEVEL_INFO,
    warn = SPDLOG_LEVEL_WARN,
    err = SPDLOG_LEVEL_ERROR,
    critical = SPDLOG_LEVEL_CRITICAL,
    off = SPDLOG_LEVEL_OFF,
    n_levels
};

#if defined(SPDLOG_NO_ATOMIC_LEVELS)
    using atomic_level_t = details::null_atomic<log_level>;
#else
    using atomic_level_t = std::atomic<log_level>;
#endif

#if !defined(SPDLOG_LEVEL_NAMES)
#define SPDLOG_LEVEL_NAMES { "trace", "debug", "info", "warning", "error", "critical", "off"}
#endif

#if !defined(SPDLOG_SHORT_LEVEL_NAMES)
#define SPDLOG_SHORT_LEVEL_NAMES {"T", "D", "I", "W", "E", "C", "O"}
#endif

constexpr size_t to_size_t(log_level level) noexcept
{
    return static_cast<size_t>(level);
}
constexpr auto levels_count = to_size_t(log_level::n_levels);
constexpr std::array<string_view_t, levels_count> level_string_views SPDLOG_LEVEL_NAMES;
constexpr std::array<const char *, levels_count> short_level_names SPDLOG_SHORT_LEVEL_NAMES;

constexpr string_view_t to_string_view(spdlog::log_level lvl) noexcept
{
    return level_string_views.at(to_size_t(lvl));
}

constexpr const char *to_short_c_str(spdlog::log_level lvl) noexcept
{
    return short_level_names.at(to_size_t(lvl));
}

SPDLOG_API spdlog::log_level level_from_str(const std::string &name) noexcept;



//
// Color mode used by sinks with color support.
//
enum class color_mode
{
    always,
    automatic,
    never
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
class SPDLOG_API spdlog_ex : public std::exception
{
public:
    explicit spdlog_ex(std::string msg);
    spdlog_ex(const std::string &msg, int last_errno);
    const char *what() const noexcept override;

private:
    std::string msg_;
};

[[noreturn]] SPDLOG_API void throw_spdlog_ex(const std::string &msg, int last_errno);
[[noreturn]] SPDLOG_API void throw_spdlog_ex(std::string msg);

struct source_loc
{
    constexpr source_loc() = default;
    constexpr source_loc(const char *filename_in, std::uint_least32_t line_in, const char *funcname_in)
        : filename{filename_in}
        , line{line_in}
        , funcname{funcname_in}
    {}

#ifdef SPDLOG_STD_SOURCE_LOCATION
    static constexpr source_loc current(const std::source_location source_location = std::source_location::current())
    {
        return source_loc{source_location.file_name(), source_location.line(), source_location.function_name()};
    }
#elif defined(SPDLOG_EXPERIMENTAL_SOURCE_LOCATION)
    static constexpr source_loc current(
        const std::experimental::source_location source_location = std::experimental::source_location::current())
    {
        return source_loc{source_location.file_name(), source_location.line(), source_location.function_name()};
    }
#else // no source location support
    static constexpr source_loc current()
    {
        return source_loc{};
    }
#endif

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return line == 0;
    }
    const char *filename{nullptr};
    std::uint_least32_t line{0};
    const char *funcname{nullptr};
};

// trick to capture format string and caller's source location with variadic template.
// see logger::info() etc. to understand how it's used.
struct loc_with_fmt
{
    source_loc loc;
    string_view_t fmt_string;
    template<typename S, typename = is_convertible_to_sv<S>>
    constexpr loc_with_fmt(S fmt_str, source_loc loc = source_loc::current()) noexcept
        : loc(loc)
        , fmt_string(fmt_str)
    {}

#ifndef SPDLOG_USE_STD_FORMAT
    constexpr loc_with_fmt(fmt::runtime_format_string<char> fmt_str, source_loc loc = source_loc::current()) noexcept
        : loc(loc)
        , fmt_string(fmt_str.str)
    {}
#endif
};

struct file_event_handlers
{
    file_event_handlers()
        : before_open(nullptr)
        , after_open(nullptr)
        , before_close(nullptr)
        , after_close(nullptr)
    {}

    std::function<void(const filename_t &filename)> before_open;
    std::function<void(const filename_t &filename, std::FILE *file_stream)> after_open;
    std::function<void(const filename_t &filename, std::FILE *file_stream)> before_close;
    std::function<void(const filename_t &filename)> after_close;
};

namespace details {

// to_string_view

constexpr spdlog::string_view_t to_string_view(const memory_buf_t &buf) noexcept
{
    return spdlog::string_view_t{buf.data(), buf.size()};
}

constexpr spdlog::string_view_t to_string_view(spdlog::string_view_t str) noexcept
{
    return str;
}

#if defined(SPDLOG_WCHAR_FILENAMES)
constexpr spdlog::wstring_view_t to_string_view(const wmemory_buf_t &buf) noexcept
{
    return spdlog::wstring_view_t{buf.data(), buf.size()};
}

constexpr spdlog::wstring_view_t to_string_view(spdlog::wstring_view_t str) noexcept
{
    return str;
}
#endif

// convert format_string<...> to string_view depending on format lib versions
#if defined(SPDLOG_USE_STD_FORMAT)
#    if __cpp_lib_format >= 202207L // std::format and __cpp_lib_format >= 202207L
template<typename T, typename... Args>
constexpr std::basic_string_view<T> to_string_view(std::basic_format_string<T, Args...> fmt) noexcept
{
    return fmt.get();
}
#    else // std::format and __cpp_lib_format < 202207L
template<typename T, typename... Args>
constexpr std::basic_string_view<T> to_string_view(std::basic_format_string<T, Args...> fmt) noexcept
{
    return fmt;
}
#    endif
#else // {fmt} version
template<typename T, typename... Args>
constexpr fmt::basic_string_view<T> to_string_view(fmt::basic_format_string<T, Args...> fmt) noexcept
{
    return fmt;
}
#endif

} // namespace details
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#    include "common-inl.h"
#endif
