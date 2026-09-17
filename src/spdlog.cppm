// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// C++20 named module for spdlog.
//
// This is a *wrapper* module: every spdlog and fmt entity is declared in the
// global module fragment below, so all entities stay attached to the global
// module and are the same entities that `#include <spdlog/spdlog.h>` produces.
// A program may therefore freely mix `import spdlog;` and `#include <spdlog/...>`
// in different translation units and link them together.
//
// Two things a module cannot carry across the boundary:
//   * Macros. The logging macros live in <spdlog/macros.h>; include it
//     alongside `import spdlog;` if you use SPDLOG_INFO(..) and friends.
//   * Standard library names. `import spdlog;` gives you spdlog's names only;
//     `std::shared_ptr`, `std::string`, `std::ostream`, ... still come from
//     the usual #includes.
//
// NOTE for MSVC users: include your standard headers *before* `import spdlog;`.
// Including one afterwards trips a cascade of C2572 "redefinition of default
// argument" errors inside the MSVC standard library.
//
// Known MSVC limitation: spdlog::mdc is exported here, but MSVC cannot emit the
// thread_local inside mdc::get_context() from an imported declaration and fails to
// link with "unresolved external symbol __tlregdtor". Users on MSVC should also
// #include <spdlog/mdc.h>, which is harmless on every other compiler.
//
// Only sinks with no external dependency are exported. qt_sinks, mongo_sink,
// kafka_sink, systemd_sink, syslog_sink, tcp_sink, udp_sink, android_sink and
// loki_sink stay header-only and can be #included next to `import spdlog;`.
//
// Future work: when `import std;` is broadly usable (CMake's
// CMAKE_EXPERIMENTAL_CXX_IMPORT_STD / the CXX_MODULE_STD target property), the
// global module fragment below could be replaced by `import std;` in the
// purview - but that means reworking the headers, not just this file.

module;

// ---------------------------------------------------------------------------
// Global module fragment.
//
// Every public header is listed explicitly, even where one already pulls in
// another: a using-declaration only nominates the overloads visible at its
// point of declaration, so all overload sets must be complete before the
// export block below.
// ---------------------------------------------------------------------------
#include <spdlog/common.h>
#include <spdlog/fwd.h>
#include <spdlog/formatter.h>
#include <spdlog/logger.h>
#include <spdlog/async_logger.h>
#include <spdlog/async.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <spdlog/version.h>
#include <spdlog/cfg/argv.h>
#include <spdlog/cfg/env.h>
#include <spdlog/cfg/helpers.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/details/log_msg_buffer.h>
#include <spdlog/details/registry.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/details/thread_pool.h>

// mdc.h #errors out when thread local storage is unavailable
#ifndef SPDLOG_NO_TLS
    #include <spdlog/mdc.h>
#endif

// same guard example.cpp uses: bin_to_hex needs fmt, or MSVC's std::format
#if !defined(SPDLOG_USE_STD_FORMAT) || defined(_MSC_VER)
    #include <spdlog/fmt/bin_to_hex.h>
#endif

#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/callback_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/dup_filter_sink.h>
#include <spdlog/sinks/hourly_file_sink.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>

#ifdef _WIN32
    #include <spdlog/sinks/msvc_sink.h>
    #include <spdlog/sinks/win_eventlog_sink.h>
    #include <spdlog/sinks/wincolor_sink.h>
#else
    // src/color_sinks.cpp only instantiates the ansicolor templates off Windows,
    // so exporting them on Windows would name something that cannot be linked.
    #include <spdlog/sinks/ansicolor_sink.h>
#endif

export module spdlog;

// ---------------------------------------------------------------------------
// fmt names needed by consumers that specialize fmt::formatter<T>.
//
// fmt's own namespace macros are used so that these are redeclared in the very
// namespace the headers put them in (fmt::v12::... for the bundled fmt); a
// hand-written `export namespace fmt {` would instead hoist them out of fmt's
// inline version namespace.
//
// Nothing is re-exported under SPDLOG_USE_STD_FORMAT: names from namespace std
// are never re-exported, consumers simply #include <format>.
// ---------------------------------------------------------------------------
#if !defined(SPDLOG_USE_STD_FORMAT)

export FMT_BEGIN_NAMESPACE

using ::fmt::basic_format_parse_context;
using ::fmt::basic_memory_buffer;
using ::fmt::basic_string_view;
using ::fmt::format;
using ::fmt::format_context;
using ::fmt::format_error;
using ::fmt::format_parse_context;
using ::fmt::format_to;
using ::fmt::formatter;
using ::fmt::memory_buffer;
using ::fmt::string_view;
using ::fmt::to_string;
using ::fmt::vformat;
using ::fmt::vformat_to;

#if FMT_VERSION >= 80000
using ::fmt::appender;
using ::fmt::format_string;
using ::fmt::runtime;
#endif

#if defined(SPDLOG_WCHAR_FILENAMES) || defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT)
using ::fmt::wformat_context;
using ::fmt::wformat_string;
using ::fmt::wmemory_buffer;
using ::fmt::wstring_view;
#endif

FMT_END_NAMESPACE

#endif  // !SPDLOG_USE_STD_FORMAT

// ---------------------------------------------------------------------------
// spdlog.
//
// Every using-declaration names ::SPDLOG_NAMESPACE::, never ::spdlog::, so that
// the module keeps working under -DSPDLOG_NAMESPACE=... and under a custom
// SPDLOG_NAMESPACE_BEGIN/END that wraps everything in an inline namespace.
// ---------------------------------------------------------------------------
export SPDLOG_NAMESPACE_BEGIN

// --- common.h ---
using ::SPDLOG_NAMESPACE::color_mode;
using ::SPDLOG_NAMESPACE::err_handler;
using ::SPDLOG_NAMESPACE::file_event_handlers;
using ::SPDLOG_NAMESPACE::filename_t;
using ::SPDLOG_NAMESPACE::format_string_t;
using ::SPDLOG_NAMESPACE::is_convertible_to_any_format_string;
using ::SPDLOG_NAMESPACE::is_convertible_to_basic_format_string;
using ::SPDLOG_NAMESPACE::level_t;
using ::SPDLOG_NAMESPACE::log_clock;
using ::SPDLOG_NAMESPACE::memory_buf_t;
using ::SPDLOG_NAMESPACE::pattern_time_type;
using ::SPDLOG_NAMESPACE::sink_ptr;
using ::SPDLOG_NAMESPACE::sinks_init_list;
using ::SPDLOG_NAMESPACE::source_loc;
using ::SPDLOG_NAMESPACE::spdlog_ex;
using ::SPDLOG_NAMESPACE::string_view_t;
using ::SPDLOG_NAMESPACE::throw_spdlog_ex;

#if !defined(SPDLOG_USE_STD_FORMAT)
using ::SPDLOG_NAMESPACE::fmt_runtime_string;
using ::SPDLOG_NAMESPACE::remove_cvref_t;
#endif

#if defined(SPDLOG_WCHAR_FILENAMES) || defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT)
using ::SPDLOG_NAMESPACE::wformat_string_t;
using ::SPDLOG_NAMESPACE::wmemory_buf_t;
using ::SPDLOG_NAMESPACE::wstring_view_t;
#endif

// --- fwd.h / formatter.h / logger.h / async_logger.h ---
using ::SPDLOG_NAMESPACE::async_logger;
using ::SPDLOG_NAMESPACE::async_overflow_policy;
using ::SPDLOG_NAMESPACE::formatter;
using ::SPDLOG_NAMESPACE::logger;
using ::SPDLOG_NAMESPACE::swap;

// --- async.h ---
using ::SPDLOG_NAMESPACE::async_factory;
using ::SPDLOG_NAMESPACE::async_factory_impl;
using ::SPDLOG_NAMESPACE::async_factory_nonblock;
using ::SPDLOG_NAMESPACE::create_async;
using ::SPDLOG_NAMESPACE::create_async_nb;
using ::SPDLOG_NAMESPACE::init_thread_pool;
using ::SPDLOG_NAMESPACE::thread_pool;

// --- pattern_formatter.h ---
using ::SPDLOG_NAMESPACE::custom_flag_formatter;
using ::SPDLOG_NAMESPACE::pattern_formatter;

// --- details/synchronous_factory.h ---
using ::SPDLOG_NAMESPACE::synchronous_factory;

// --- spdlog.h ---
using ::SPDLOG_NAMESPACE::apply_all;
using ::SPDLOG_NAMESPACE::apply_logger_env_levels;
using ::SPDLOG_NAMESPACE::create;
using ::SPDLOG_NAMESPACE::critical;
using ::SPDLOG_NAMESPACE::debug;
using ::SPDLOG_NAMESPACE::default_factory;
using ::SPDLOG_NAMESPACE::default_logger;
using ::SPDLOG_NAMESPACE::default_logger_raw;
using ::SPDLOG_NAMESPACE::disable_backtrace;
using ::SPDLOG_NAMESPACE::drop;
using ::SPDLOG_NAMESPACE::drop_all;
using ::SPDLOG_NAMESPACE::dump_backtrace;
using ::SPDLOG_NAMESPACE::enable_backtrace;
using ::SPDLOG_NAMESPACE::error;
using ::SPDLOG_NAMESPACE::flush_all;
using ::SPDLOG_NAMESPACE::flush_every;
using ::SPDLOG_NAMESPACE::flush_on;
using ::SPDLOG_NAMESPACE::get;
using ::SPDLOG_NAMESPACE::get_level;
using ::SPDLOG_NAMESPACE::info;
using ::SPDLOG_NAMESPACE::initialize_logger;
using ::SPDLOG_NAMESPACE::log;
using ::SPDLOG_NAMESPACE::register_logger;
using ::SPDLOG_NAMESPACE::register_or_replace;
using ::SPDLOG_NAMESPACE::set_automatic_registration;
using ::SPDLOG_NAMESPACE::set_default_logger;
using ::SPDLOG_NAMESPACE::set_error_handler;
using ::SPDLOG_NAMESPACE::set_formatter;
using ::SPDLOG_NAMESPACE::set_level;
using ::SPDLOG_NAMESPACE::set_pattern;
using ::SPDLOG_NAMESPACE::should_log;
using ::SPDLOG_NAMESPACE::shutdown;
using ::SPDLOG_NAMESPACE::trace;
using ::SPDLOG_NAMESPACE::warn;

// --- stopwatch.h ---
using ::SPDLOG_NAMESPACE::stopwatch;

// --- mdc.h ---
#ifndef SPDLOG_NO_TLS
using ::SPDLOG_NAMESPACE::mdc;
#endif

// --- fmt/bin_to_hex.h ---
#if !defined(SPDLOG_USE_STD_FORMAT) || defined(_MSC_VER)
using ::SPDLOG_NAMESPACE::to_hex;
#endif

// --- sinks/callback_sink.h (the callback typedef lives in spdlog, not sinks) ---
using ::SPDLOG_NAMESPACE::custom_log_callback;

// --- sink factory functions ---
using ::SPDLOG_NAMESPACE::basic_logger_mt;
using ::SPDLOG_NAMESPACE::basic_logger_st;
using ::SPDLOG_NAMESPACE::callback_logger_mt;
using ::SPDLOG_NAMESPACE::callback_logger_st;
using ::SPDLOG_NAMESPACE::daily_logger_format_mt;
using ::SPDLOG_NAMESPACE::daily_logger_format_st;
using ::SPDLOG_NAMESPACE::daily_logger_mt;
using ::SPDLOG_NAMESPACE::daily_logger_st;
using ::SPDLOG_NAMESPACE::hourly_logger_mt;
using ::SPDLOG_NAMESPACE::hourly_logger_st;
using ::SPDLOG_NAMESPACE::null_logger_mt;
using ::SPDLOG_NAMESPACE::null_logger_st;
using ::SPDLOG_NAMESPACE::rotating_logger_mt;
using ::SPDLOG_NAMESPACE::rotating_logger_st;
using ::SPDLOG_NAMESPACE::stderr_color_mt;
using ::SPDLOG_NAMESPACE::stderr_color_st;
using ::SPDLOG_NAMESPACE::stderr_logger_mt;
using ::SPDLOG_NAMESPACE::stderr_logger_st;
using ::SPDLOG_NAMESPACE::stdout_color_mt;
using ::SPDLOG_NAMESPACE::stdout_color_st;
using ::SPDLOG_NAMESPACE::stdout_logger_mt;
using ::SPDLOG_NAMESPACE::stdout_logger_st;

// ---------------------------------------------------------------------------
namespace level {
using ::SPDLOG_NAMESPACE::level::critical;
using ::SPDLOG_NAMESPACE::level::debug;
using ::SPDLOG_NAMESPACE::level::err;
using ::SPDLOG_NAMESPACE::level::from_str;
using ::SPDLOG_NAMESPACE::level::info;
using ::SPDLOG_NAMESPACE::level::level_enum;
using ::SPDLOG_NAMESPACE::level::n_levels;
using ::SPDLOG_NAMESPACE::level::off;
using ::SPDLOG_NAMESPACE::level::to_short_c_str;
using ::SPDLOG_NAMESPACE::level::to_string_view;
using ::SPDLOG_NAMESPACE::level::trace;
using ::SPDLOG_NAMESPACE::level::warn;
}  // namespace level

// ---------------------------------------------------------------------------
namespace cfg {
using ::SPDLOG_NAMESPACE::cfg::load_argv_levels;
using ::SPDLOG_NAMESPACE::cfg::load_env_levels;

namespace helpers {
using ::SPDLOG_NAMESPACE::cfg::helpers::load_levels;
}  // namespace helpers
}  // namespace cfg

// ---------------------------------------------------------------------------
// Only the details names that appear in the public API or that users routinely
// need (custom flag formatters, custom sinks). The rest - circular_q,
// file_helper, backtracer, periodic_worker, os::*, fmt_helper::* - stay
// header-only. Note that details::os::default_eol and
// details::default_async_q_size have internal linkage and cannot be exported
// at all.
// ---------------------------------------------------------------------------
namespace details {
using ::SPDLOG_NAMESPACE::details::console_mutex;
using ::SPDLOG_NAMESPACE::details::console_nullmutex;
using ::SPDLOG_NAMESPACE::details::enable_if_t;
using ::SPDLOG_NAMESPACE::details::flag_formatter;
using ::SPDLOG_NAMESPACE::details::log_msg;
using ::SPDLOG_NAMESPACE::details::log_msg_buffer;
using ::SPDLOG_NAMESPACE::details::make_unique;
using ::SPDLOG_NAMESPACE::details::null_atomic_int;
using ::SPDLOG_NAMESPACE::details::null_mutex;
using ::SPDLOG_NAMESPACE::details::padding_info;
using ::SPDLOG_NAMESPACE::details::registry;
using ::SPDLOG_NAMESPACE::details::thread_pool;

#if !defined(SPDLOG_USE_STD_FORMAT) || defined(_MSC_VER)
using ::SPDLOG_NAMESPACE::details::dump_info;
#endif
}  // namespace details

// ---------------------------------------------------------------------------
namespace sinks {
using ::SPDLOG_NAMESPACE::sinks::base_sink;
using ::SPDLOG_NAMESPACE::sinks::sink;

using ::SPDLOG_NAMESPACE::sinks::stderr_sink;
using ::SPDLOG_NAMESPACE::sinks::stderr_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::stderr_sink_st;
using ::SPDLOG_NAMESPACE::sinks::stdout_sink;
using ::SPDLOG_NAMESPACE::sinks::stdout_sink_base;
using ::SPDLOG_NAMESPACE::sinks::stdout_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::stdout_sink_st;

using ::SPDLOG_NAMESPACE::sinks::stderr_color_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::stderr_color_sink_st;
using ::SPDLOG_NAMESPACE::sinks::stdout_color_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::stdout_color_sink_st;

using ::SPDLOG_NAMESPACE::sinks::basic_file_sink;
using ::SPDLOG_NAMESPACE::sinks::basic_file_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::basic_file_sink_st;

using ::SPDLOG_NAMESPACE::sinks::rotating_file_sink;
using ::SPDLOG_NAMESPACE::sinks::rotating_file_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::rotating_file_sink_st;

using ::SPDLOG_NAMESPACE::sinks::daily_file_format_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::daily_file_format_sink_st;
using ::SPDLOG_NAMESPACE::sinks::daily_file_sink;
using ::SPDLOG_NAMESPACE::sinks::daily_file_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::daily_file_sink_st;
using ::SPDLOG_NAMESPACE::sinks::daily_filename_calculator;
using ::SPDLOG_NAMESPACE::sinks::daily_filename_format_calculator;

using ::SPDLOG_NAMESPACE::sinks::hourly_file_sink;
using ::SPDLOG_NAMESPACE::sinks::hourly_file_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::hourly_file_sink_st;
using ::SPDLOG_NAMESPACE::sinks::hourly_filename_calculator;

using ::SPDLOG_NAMESPACE::sinks::null_sink;
using ::SPDLOG_NAMESPACE::sinks::null_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::null_sink_st;

using ::SPDLOG_NAMESPACE::sinks::ostream_sink;
using ::SPDLOG_NAMESPACE::sinks::ostream_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::ostream_sink_st;

using ::SPDLOG_NAMESPACE::sinks::dist_sink;
using ::SPDLOG_NAMESPACE::sinks::dist_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::dist_sink_st;

using ::SPDLOG_NAMESPACE::sinks::dup_filter_sink;
using ::SPDLOG_NAMESPACE::sinks::dup_filter_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::dup_filter_sink_st;

using ::SPDLOG_NAMESPACE::sinks::ringbuffer_sink;
using ::SPDLOG_NAMESPACE::sinks::ringbuffer_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::ringbuffer_sink_st;

using ::SPDLOG_NAMESPACE::sinks::callback_sink;
using ::SPDLOG_NAMESPACE::sinks::callback_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::callback_sink_st;

#ifdef _WIN32
using ::SPDLOG_NAMESPACE::sinks::wincolor_sink;
using ::SPDLOG_NAMESPACE::sinks::wincolor_stderr_sink;
using ::SPDLOG_NAMESPACE::sinks::wincolor_stderr_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::wincolor_stderr_sink_st;
using ::SPDLOG_NAMESPACE::sinks::wincolor_stdout_sink;
using ::SPDLOG_NAMESPACE::sinks::wincolor_stdout_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::wincolor_stdout_sink_st;

using ::SPDLOG_NAMESPACE::sinks::msvc_sink;
using ::SPDLOG_NAMESPACE::sinks::msvc_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::msvc_sink_st;
using ::SPDLOG_NAMESPACE::sinks::windebug_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::windebug_sink_st;

using ::SPDLOG_NAMESPACE::sinks::win_eventlog_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::win_eventlog_sink_st;

namespace win_eventlog {
using ::SPDLOG_NAMESPACE::sinks::win_eventlog::win_eventlog_sink;
}  // namespace win_eventlog
#else
using ::SPDLOG_NAMESPACE::sinks::ansicolor_sink;
using ::SPDLOG_NAMESPACE::sinks::ansicolor_stderr_sink;
using ::SPDLOG_NAMESPACE::sinks::ansicolor_stderr_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::ansicolor_stderr_sink_st;
using ::SPDLOG_NAMESPACE::sinks::ansicolor_stdout_sink;
using ::SPDLOG_NAMESPACE::sinks::ansicolor_stdout_sink_mt;
using ::SPDLOG_NAMESPACE::sinks::ansicolor_stdout_sink_st;
#endif
}  // namespace sinks

SPDLOG_NAMESPACE_END

// ---------------------------------------------------------------------------
// spdlog::fmt_lib is a namespace alias. A using-declaration cannot re-export
// one, so it is redeclared here in the purview.
// ---------------------------------------------------------------------------
export SPDLOG_NAMESPACE_BEGIN
#ifdef SPDLOG_USE_STD_FORMAT
namespace fmt_lib = std;
#else
namespace fmt_lib = fmt;
#endif
SPDLOG_NAMESPACE_END

// ---------------------------------------------------------------------------
// The headers above define fmt::formatter specializations. A specialization is
// named by no using-declaration, so under [module.global.frag] it is not
// guaranteed to survive global-module-fragment discarding. These aliases make
// them decl-reachable from the purview. Not exported - they exist only to be
// referenced.
// ---------------------------------------------------------------------------
namespace spdlog_module_detail {

using stopwatch_formatter = SPDLOG_NAMESPACE::fmt_lib::formatter<SPDLOG_NAMESPACE::stopwatch>;

#if !defined(SPDLOG_USE_STD_FORMAT) || defined(_MSC_VER)
using dump_info_formatter =
    SPDLOG_NAMESPACE::fmt_lib::formatter<SPDLOG_NAMESPACE::details::dump_info<
        std::vector<unsigned char>::const_iterator>>;
#endif

}  // namespace spdlog_module_detail
