// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/null_mutex.h>
#include <spdlog/tweakme.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <exception>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <type_traits>

#ifdef SPDLOG_USE_STD_FORMAT
#include <version>
#if __cpp_lib_format >= 202207L
#include <format>
#else
#include <string_view>
#endif
#endif

#ifdef SPDLOG_COMPILED_LIB
#undef SPDLOG_HEADER_ONLY
#if defined(SPDLOG_SHARED_LIB)
#if defined(_WIN32)
#ifdef spdlog_EXPORTS
#define SPDLOG_API __declspec(dllexport)
#else  // !spdlog_EXPORTS
#define SPDLOG_API __declspec(dllimport)
#endif
#else  // !defined(_WIN32)
#define SPDLOG_API __attribute__((visibility("default")))
#endif
#else  // !defined(SPDLOG_SHARED_LIB)
#define SPDLOG_API
#endif
#define SPDLOG_INLINE
#else  // !defined(SPDLOG_COMPILED_LIB)
#define SPDLOG_API
#define SPDLOG_HEADER_ONLY
#define SPDLOG_INLINE inline
#endif  // #ifdef SPDLOG_COMPILED_LIB

#include <spdlog/fmt/fmt.h>

#if !defined(SPDLOG_USE_STD_FORMAT) && \
    FMT_VERSION >= 80000  // backward compatibility with fmt versions older than 8
#define SPDLOG_FMT_RUNTIME(format_string) fmt::runtime(format_string)
#ifdef SPDLOG_MODULE_BUILD
// FMT_STRING's compile-string path calls fmt's parse_format_string through
// argument-dependent lookup, which GCC cannot resolve when the template is
// instantiated in a translation unit that imports the module. The module is
// always C++20, where fmt checks format string literals at compile time anyway.
#define SPDLOG_FMT_STRING(format_string) format_string
#else
#define SPDLOG_FMT_STRING(format_string) FMT_STRING(format_string)
#endif
#if defined(SPDLOG_WCHAR_FILENAMES) || defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT)
#include <spdlog/fmt/xchar.h>
#endif
#else
#define SPDLOG_FMT_RUNTIME(format_string) format_string
#define SPDLOG_FMT_STRING(format_string) format_string
#endif

// visual studio up to 2013 does not support noexcept nor constexpr
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define SPDLOG_NOEXCEPT _NOEXCEPT
#define SPDLOG_CONSTEXPR
#else
#define SPDLOG_NOEXCEPT noexcept
#define SPDLOG_CONSTEXPR constexpr
#endif

// If building with std::format, can just use constexpr, otherwise if building with fmt
// SPDLOG_CONSTEXPR_FUNC needs to be set the same as FMT_CONSTEXPR to avoid situations where
// a constexpr function in spdlog could end up calling a non-constexpr function in fmt
// depending on the compiler
// If fmt determines it can't use constexpr, we should inline the function instead
#ifdef SPDLOG_USE_STD_FORMAT
#define SPDLOG_CONSTEXPR_FUNC constexpr
#else  // Being built with fmt
#if FMT_USE_CONSTEXPR
#define SPDLOG_CONSTEXPR_FUNC FMT_CONSTEXPR
#else
#define SPDLOG_CONSTEXPR_FUNC inline
#endif
#endif

// Namespace-scope constants need external linkage so that templates defined in
// these headers can still reference them when instantiated in a translation unit
// that consumes spdlog through `import spdlog;`. A namespace-scope static or
// constexpr variable has internal linkage, which cannot cross the module
// boundary (referencing it from an exported inline function/template body is an
// ill-formed exposure of a TU-local entity). Falls back to static before
// C++17, where inline variables do not exist - and where there is no module to
// worry about either.
#if defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L
#define SPDLOG_INLINE_VAR inline
#else
#define SPDLOG_INLINE_VAR static
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
#if (defined(_MSC_VER) && (_MSC_VER < 1900)) || defined(__cplusplus_winrt)
#define SPDLOG_NO_TLS 1
#endif
#endif

#include <spdlog/macros.h>

#ifdef SPDLOG_NO_EXCEPTIONS
#define SPDLOG_TRY
#define SPDLOG_THROW(ex)                               \
    do {                                               \
        printf("spdlog fatal error: %s\n", ex.what()); \
        std::abort();                                  \
    } while (0)
#define SPDLOG_CATCH_STD
#else
#define SPDLOG_TRY try
#define SPDLOG_THROW(ex) throw(ex)
#define SPDLOG_CATCH_STD             \
    catch (const std::exception &) { \
    }
#endif

#include "./namespace.h"

SPDLOG_NAMESPACE_BEGIN

SPDLOG_EXPORT class formatter;

namespace sinks {
SPDLOG_EXPORT class sink;
}

#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
SPDLOG_EXPORT using filename_t = std::wstring;
// allow macro expansion to occur in SPDLOG_FILENAME_T
#define SPDLOG_FILENAME_T_INNER(s) L##s
#define SPDLOG_FILENAME_T(s) SPDLOG_FILENAME_T_INNER(s)
#else
SPDLOG_EXPORT using filename_t = std::string;
#define SPDLOG_FILENAME_T(s) s
#endif

SPDLOG_EXPORT using log_clock = std::chrono::system_clock;
SPDLOG_EXPORT using sink_ptr = std::shared_ptr<sinks::sink>;
SPDLOG_EXPORT using sinks_init_list = std::initializer_list<sink_ptr>;
SPDLOG_EXPORT using err_handler = std::function<void(const std::string &err_msg)>;
#ifdef SPDLOG_USE_STD_FORMAT
SPDLOG_EXPORT namespace fmt_lib = std;

SPDLOG_EXPORT using string_view_t = std::string_view;
SPDLOG_EXPORT using memory_buf_t = std::string;

SPDLOG_EXPORT template <typename... Args>
#if __cpp_lib_format >= 202207L
using format_string_t = std::format_string<Args...>;
#else
using format_string_t = std::string_view;
#endif

SPDLOG_EXPORT template <class T, class Char = char>
struct is_convertible_to_basic_format_string
    : std::integral_constant<bool, std::is_convertible<T, std::basic_string_view<Char>>::value> {};

#if defined(SPDLOG_WCHAR_FILENAMES) || defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT)
SPDLOG_EXPORT using wstring_view_t = std::wstring_view;
SPDLOG_EXPORT using wmemory_buf_t = std::wstring;

SPDLOG_EXPORT template <typename... Args>
#if __cpp_lib_format >= 202207L
using wformat_string_t = std::wformat_string<Args...>;
#else
using wformat_string_t = std::wstring_view;
#endif
#endif
#define SPDLOG_BUF_TO_STRING(x) x
#else  // use fmt lib instead of std::format
SPDLOG_EXPORT namespace fmt_lib = fmt;

SPDLOG_EXPORT using string_view_t = fmt::basic_string_view<char>;
SPDLOG_EXPORT using memory_buf_t = fmt::basic_memory_buffer<char, 250>;

SPDLOG_EXPORT template <typename... Args>
using format_string_t = fmt::format_string<Args...>;

SPDLOG_EXPORT template <class T>
using remove_cvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

SPDLOG_EXPORT template <typename Char>
#if FMT_VERSION >= 90101
using fmt_runtime_string = fmt::runtime_format_string<Char>;
#else
using fmt_runtime_string = fmt::basic_runtime<Char>;
#endif

// clang doesn't like SFINAE disabled constructor in std::is_convertible<> so have to repeat the
// condition from basic_format_string here, in addition, fmt::basic_runtime<Char> is only
// convertible to basic_format_string<Char> but not basic_string_view<Char>
SPDLOG_EXPORT template <class T, class Char = char>
struct is_convertible_to_basic_format_string
    : std::integral_constant<bool,
                             std::is_convertible<T, fmt::basic_string_view<Char>>::value ||
                                 std::is_same<remove_cvref_t<T>, fmt_runtime_string<Char>>::value> {
};

#if defined(SPDLOG_WCHAR_FILENAMES) || defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT)
SPDLOG_EXPORT using wstring_view_t = fmt::basic_string_view<wchar_t>;
SPDLOG_EXPORT using wmemory_buf_t = fmt::basic_memory_buffer<wchar_t, 250>;

SPDLOG_EXPORT template <typename... Args>
using wformat_string_t = fmt::wformat_string<Args...>;
#endif
#define SPDLOG_BUF_TO_STRING(x) fmt::to_string(x)
#endif

#ifdef SPDLOG_WCHAR_TO_UTF8_SUPPORT
#ifndef _WIN32
#error SPDLOG_WCHAR_TO_UTF8_SUPPORT only supported on windows
#endif  // _WIN32
#endif  // SPDLOG_WCHAR_TO_UTF8_SUPPORT

SPDLOG_EXPORT template <class T>
struct is_convertible_to_any_format_string
    : std::integral_constant<bool,
                             is_convertible_to_basic_format_string<T, char>::value ||
                                 is_convertible_to_basic_format_string<T, wchar_t>::value> {};

#if defined(SPDLOG_NO_ATOMIC_LEVELS)
SPDLOG_EXPORT using level_t = details::null_atomic_int;
#else
SPDLOG_EXPORT using level_t = std::atomic<int>;
#endif

// Log level enum
namespace level {
SPDLOG_EXPORT enum level_enum : int {
    trace = SPDLOG_LEVEL_TRACE,
    debug = SPDLOG_LEVEL_DEBUG,
    info = SPDLOG_LEVEL_INFO,
    warn = SPDLOG_LEVEL_WARN,
    err = SPDLOG_LEVEL_ERROR,
    critical = SPDLOG_LEVEL_CRITICAL,
    off = SPDLOG_LEVEL_OFF,
    n_levels
};

#define SPDLOG_LEVEL_NAME_TRACE SPDLOG_NAMESPACE::string_view_t("trace", 5)
#define SPDLOG_LEVEL_NAME_DEBUG SPDLOG_NAMESPACE::string_view_t("debug", 5)
#define SPDLOG_LEVEL_NAME_INFO SPDLOG_NAMESPACE::string_view_t("info", 4)
#define SPDLOG_LEVEL_NAME_WARNING SPDLOG_NAMESPACE::string_view_t("warning", 7)
#define SPDLOG_LEVEL_NAME_ERROR SPDLOG_NAMESPACE::string_view_t("error", 5)
#define SPDLOG_LEVEL_NAME_CRITICAL SPDLOG_NAMESPACE::string_view_t("critical", 8)
#define SPDLOG_LEVEL_NAME_OFF SPDLOG_NAMESPACE::string_view_t("off", 3)

#if !defined(SPDLOG_LEVEL_NAMES)
#define SPDLOG_LEVEL_NAMES                                                                  \
    {                                                                                       \
        SPDLOG_LEVEL_NAME_TRACE, SPDLOG_LEVEL_NAME_DEBUG, SPDLOG_LEVEL_NAME_INFO,           \
            SPDLOG_LEVEL_NAME_WARNING, SPDLOG_LEVEL_NAME_ERROR, SPDLOG_LEVEL_NAME_CRITICAL, \
            SPDLOG_LEVEL_NAME_OFF                                                           \
    }
#endif

#if !defined(SPDLOG_SHORT_LEVEL_NAMES)

#define SPDLOG_SHORT_LEVEL_NAMES \
    { "T", "D", "I", "W", "E", "C", "O" }
#endif

SPDLOG_EXPORT SPDLOG_API const string_view_t &to_string_view(level::level_enum l) SPDLOG_NOEXCEPT;
SPDLOG_EXPORT SPDLOG_API const char *to_short_c_str(level::level_enum l) SPDLOG_NOEXCEPT;
SPDLOG_EXPORT SPDLOG_API level::level_enum from_str(const std::string &name) SPDLOG_NOEXCEPT;

}  // namespace level

//
// Color mode used by sinks with color support.
//
SPDLOG_EXPORT enum class color_mode { always, automatic, never };

//
// Pattern time - specific time getting to use for pattern_formatter.
// local time by default
//
SPDLOG_EXPORT enum class pattern_time_type {
    local,  // log localtime
    utc     // log utc
};

//
// Log exception
//
SPDLOG_EXPORT class SPDLOG_API spdlog_ex : public std::exception {
public:
    explicit spdlog_ex(std::string msg);
    spdlog_ex(const std::string &msg, int last_errno);
    const char *what() const SPDLOG_NOEXCEPT override;

private:
    std::string msg_;
};

SPDLOG_EXPORT [[noreturn]] SPDLOG_API void throw_spdlog_ex(const std::string &msg, int last_errno);
SPDLOG_EXPORT [[noreturn]] SPDLOG_API void throw_spdlog_ex(std::string msg);

SPDLOG_EXPORT struct source_loc {
    SPDLOG_CONSTEXPR source_loc() = default;
    SPDLOG_CONSTEXPR source_loc(const char *filename_in, int line_in, const char *funcname_in)
        : filename{filename_in},
          line{line_in},
          funcname{funcname_in} {}

    SPDLOG_CONSTEXPR bool empty() const SPDLOG_NOEXCEPT { return line <= 0; }
    const char *filename{nullptr};
    int line{0};
    const char *funcname{nullptr};
};

SPDLOG_EXPORT struct file_event_handlers {
    file_event_handlers()
        : before_open(nullptr),
          after_open(nullptr),
          before_close(nullptr),
          after_close(nullptr) {}

    std::function<void(const filename_t &filename)> before_open;
    std::function<void(const filename_t &filename, std::FILE *file_stream)> after_open;
    std::function<void(const filename_t &filename, std::FILE *file_stream)> before_close;
    std::function<void(const filename_t &filename)> after_close;
};

namespace details {

// make_unique support for pre c++14
#if __cplusplus >= 201402L  // C++14 and beyond
SPDLOG_EXPORT using std::enable_if_t;
SPDLOG_EXPORT using std::make_unique;
#else
SPDLOG_EXPORT template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

SPDLOG_EXPORT template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&...args) {
    static_assert(!std::is_array<T>::value, "arrays not supported");
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif

// to avoid useless casts (see https://github.com/nlohmann/json/issues/2893#issuecomment-889152324)
template <typename T, typename U, enable_if_t<!std::is_same<T, U>::value, int> = 0>
constexpr T conditional_static_cast(U value) {
    return static_cast<T>(value);
}

template <typename T, typename U, enable_if_t<std::is_same<T, U>::value, int> = 0>
constexpr T conditional_static_cast(U value) {
    return value;
}

}  // namespace details
SPDLOG_NAMESPACE_END

#ifdef SPDLOG_HEADER_ONLY
#include "common-inl.h"
#endif
