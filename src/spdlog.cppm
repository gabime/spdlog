// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// C++20 named module for spdlog.
//
// This is a *native* module: spdlog's headers are compiled directly inside the
// purview of this module unit (below, after `export module spdlog;`), and every
// declaration marked SPDLOG_EXPORT in those headers (see include/spdlog/namespace.h)
// becomes `export`-ed from here. Declarations are therefore attached to (owned
// by) the named module `spdlog`, not to the global module.
//
// A companion PR implements an alternative *wrapper* module design, where every
// declaration instead stays attached to the global module (via using-declarations
// in the purview) so that `import spdlog;` and `#include <spdlog/...>` can be
// freely mixed in the same program. See that PR's src/spdlog.cppm for details on
// the tradeoffs. This native design cannot offer that: because module-attached
// entities get module ownership (different name mangling on MSVC; strong
// ownership on Clang), the set of entities produced by `import spdlog;` is
// distinct from the set produced by `#include <spdlog/...>`, even though both
// have the same names. Concretely, this means:
//
//   * Do NOT mix `import spdlog;` and `#include <spdlog/...>` in the same
//     program (not even in different translation units): they define two
//     different `spdlog::logger`, two different registries, etc., which is an
//     ODR violation and/or a source of very confusing link and runtime errors
//     (e.g. spdlog::get() in one TU will never find a logger registered from
//     the other). Pick one style per program.
//   * This module cannot link against a separately-built spdlog::spdlog (static
//     or shared) library. It is built in header-only mode instead: every
//     spdlog definition - normally emitted once into libspdlog from src/*.cpp -
//     is instead emitted once into this module unit's own object code, and
//     SPDLOG_COMPILED_LIB is intentionally never defined for this target (see
//     the spdlog_module target in CMakeLists.txt).
//   * Macros cannot cross the module boundary. The logging macros live in
//     <spdlog/macros.h>; include it alongside `import spdlog;` if you use
//     SPDLOG_INFO(..) and friends.
//   * Standard library names are never re-exported. `import spdlog;` gives you
//     spdlog's names only; `std::shared_ptr`, `std::string`, `std::ostream`,
//     ... still come from the usual #includes.
//   * fmt stays attached to the *global* module, not to spdlog (see the "Global
//     module fragment" comment below). The fmt names needed to specialize
//     fmt::formatter<T> are re-exported at the end of this file. Under
//     SPDLOG_USE_STD_FORMAT nothing is re-exported; #include <format> yourself.
//
// NOTE for MSVC users: include your standard headers *before* `import spdlog;`.
// Including one afterwards trips a cascade of C2572 "redefinition of default
// argument" errors inside the MSVC standard library.
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
// Only third-party (fmt), standard library, and platform headers belong here.
// spdlog's own headers are included further below, inside the module purview,
// so their declarations become part of (and, where marked SPDLOG_EXPORT,
// exported from) this named module, instead of being attached to the global
// module.
//
// fmt (or std::format) must stay attached to the *global* module: spdlog's
// bundled-fmt source file (src/bundled_fmtlib_format.cpp, used by the ordinary
// spdlog/spdlog_header_only targets) and any other code linking against fmt
// directly must see the same, globally-attached fmt entities that this module
// uses. If fmt were instead first declared inside our purview, it would become
// owned by module spdlog (different mangling on MSVC, strong ownership on
// Clang) and could no longer be linked against/interchanged with ordinarily
// compiled code. Pre-including it here, then letting spdlog's own headers
// #include it again below (a no-op, since these headers all have include
// guards), keeps fmt global while spdlog's own declarations get attached to
// this module.
// ---------------------------------------------------------------------------

// A config header with no declarations of its own (SPDLOG_USE_STD_FORMAT,
// SPDLOG_FMT_EXTERNAL, ...); safe, and necessary, to pull in here first.
#include <spdlog/tweakme.h>

#if defined(SPDLOG_USE_STD_FORMAT)
#include <format>
#elif defined(SPDLOG_FMT_EXTERNAL) || defined(SPDLOG_FMT_EXTERNAL_HO)
#include <fmt/chrono.h>
#include <fmt/format.h>
#if defined(SPDLOG_WCHAR_FILENAMES) || defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT)
#include <fmt/xchar.h>
#endif
#else
// spdlog/fmt/fmt.h (and spdlog/fmt/chrono.h, spdlog/fmt/xchar.h) define
// FMT_HEADER_ONLY themselves unless SPDLOG_COMPILED_LIB is defined. The module
// is always built in header-only mode (see the big comment above), so define it
// here too, before pulling in the bundled fmt headers directly, to match.
#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#ifndef FMT_USE_WINDOWS_H
#define FMT_USE_WINDOWS_H 0
#endif
#include <spdlog/fmt/bundled/chrono.h>
#include <spdlog/fmt/bundled/format.h>
#if defined(SPDLOG_WCHAR_FILENAMES) || defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT)
#include <spdlog/fmt/bundled/xchar.h>
#endif
#endif

// ---------------------------------------------------------------------------
// Standard library headers used by spdlog's headers and their *-inl.h
// implementation files.
// ---------------------------------------------------------------------------
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <charconv>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// C headers some spdlog headers include by their .h name
#include <stdio.h>
#include <stdlib.h>

#if defined(__has_include)
#if __has_include(<version>)
#include <version>
#endif
#endif

#if defined(__cpp_lib_span) && __cpp_lib_span >= 202002L
#include <span>
#endif

// ---------------------------------------------------------------------------
// Platform headers used by details/os-inl.h and the console/wincolor/msvc/
// win_eventlog sinks.
// ---------------------------------------------------------------------------
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <fileapi.h>   // WriteFile(..)
#include <winbase.h>
#include <wincon.h>
#include <direct.h>    // _mkdir/_wmkdir
#include <io.h>        // _get_osfhandle, _isatty, _fileno
#include <process.h>   // _get_pid
#include <share.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#if defined(__linux__)
#include <sys/syscall.h>  // gettid() on Linux
#elif defined(__FreeBSD__)
#include <pthread_np.h>  // pthread_getthreadid_np
#elif defined(__NetBSD__)
#include <lwp.h>  // _lwp_self
#elif defined(__sun)
#include <thread.h>  // thr_self
#elif defined(_AIX)
#include <pthread.h>  // pthread_getthrds_np
#endif
#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#endif
#endif

export module spdlog;

// SPDLOG_EXPORT expands to nothing for ordinary header use (#include
// <spdlog/...>). Defining it as `export` here turns every declaration marked
// SPDLOG_EXPORT in the headers below into an exported entity of this module.
#define SPDLOG_EXPORT export
// lets the headers adapt the few spots that behave differently inside a module
#define SPDLOG_MODULE_BUILD

// ---------------------------------------------------------------------------
// spdlog, compiled header-only style directly into this module's purview.
//
// Every public header is listed explicitly, even where one already pulls in
// another, for clarity; #pragma once makes the repeats harmless. Each header's
// own `#ifdef SPDLOG_HEADER_ONLY / #include "xxx-inl.h"` footer fires as usual,
// so non-inline definitions end up compiled into this module unit instead of
// into libspdlog.
//
// Compilers warn about #include in a module purview (MSVC C5244, Clang
// -Winclude-angled-in-module-purview) because it is usually a mistake; here it
// is the design. The warnings are silenced with pragmas rather than compiler
// flags so that the silencing also applies when CMake compiles this file into a
// BMI with a consumer's flags, and does not leak into consumers' own modules.
// ---------------------------------------------------------------------------
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(push)
#pragma warning(disable : 5244)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

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
#include <spdlog/details/console_globals.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/details/log_msg_buffer.h>
#include <spdlog/details/null_mutex.h>
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
// wincolor_sink.h needs <windows.h>; stdout_color_sinks.h itself only ever
// aliases one of ansicolor_sink/wincolor_sink depending on _WIN32, so only the
// applicable one needs to be compiled into the module.
#include <spdlog/sinks/ansicolor_sink.h>
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#endif

// ---------------------------------------------------------------------------
// fmt names needed by consumers that specialize fmt::formatter<T>.
//
// fmt lives in the global module fragment above, so its entities stay attached
// to the global module and are merely re-exported here - without this block
// they would be reachable but not nameable through `import spdlog;`. fmt's own
// namespace macros reopen the very namespace the headers use (fmt::v12::...).
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
