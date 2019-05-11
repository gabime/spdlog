// Copyright(c) 2015-present Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#ifndef SPDLOG_STATIC_LIB
#warning spdlog is in header only configuration. please define SPDLOG_STATIC_LIB
#endif

#include <mutex>
#include <chrono>

#include "spdlog/common.h"

#include "spdlog/common-inl.h"

#include "spdlog/details/null_mutex.h"

#include "spdlog/logger.h"
#include "spdlog/logger-inl.h"

#include "spdlog/async_logger.h"
#include "spdlog/async_logger-inl.h"

#include "spdlog/details/log_msg.h"
#include "spdlog/details/log_msg-inl.h"

#include "spdlog/sinks/sink.h"
#include "spdlog/sinks/sink-inl.h"

#include "spdlog/sinks/base_sink.h"
#include "spdlog/sinks/base_sink-inl.h"
template class spdlog::sinks::base_sink<std::mutex>;
template class spdlog::sinks::base_sink<spdlog::details::null_mutex>;

#include "spdlog/details/registry.h"
#include "spdlog/details/registry-inl.h"

#include "spdlog/details/os.h"
#include "spdlog/details/os-inl.h"

#include "spdlog/details/periodic_worker.h"
#include "spdlog/details/periodic_worker-inl.h"

#include "spdlog/details/file_helper.h"
#include "spdlog/details/file_helper-inl.h"

#include "spdlog/details/pattern_formatter.h"
#include "spdlog/details/pattern_formatter-inl.h"

#include "spdlog/details/thread_pool.h"
#include "spdlog/details/thread_pool-inl.h"
template class spdlog::details::mpmc_blocking_queue<spdlog::details::async_msg>;

#include "spdlog/sinks/ansicolor_sink.h"
#include "spdlog/sinks/ansicolor_sink-inl.h"
template class spdlog::sinks::ansicolor_sink<spdlog::details::console_stdout, spdlog::details::console_mutex>;
template class spdlog::sinks::ansicolor_sink<spdlog::details::console_stdout, spdlog::details::console_nullmutex>;
template class spdlog::sinks::ansicolor_sink<spdlog::details::console_stderr, spdlog::details::console_mutex>;
template class spdlog::sinks::ansicolor_sink<spdlog::details::console_stderr, spdlog::details::console_nullmutex>;

// fmt_helper templates
#include "spdlog/details/fmt_helper.h"
template void spdlog::details::fmt_helper::append_string_view(spdlog::string_view_t view, fmt::memory_buffer &dest);
template spdlog::string_view_t spdlog::details::fmt_helper::to_string_view(const fmt::memory_buffer &buf) SPDLOG_NOEXCEPT;


// Slightly modified version of fmt lib's format.cc source file.
// Copyright (c) 2012 - 2016, Victor Zverovich
// All rights reserved.

#if !defined(SPDLOG_FMT_EXTERNAL)
#include "spdlog/fmt/bundled/format-inl.h"

FMT_BEGIN_NAMESPACE
template struct internal::basic_data<void>;
template FMT_API internal::locale_ref::locale_ref(const std::locale &loc);
template FMT_API std::locale internal::locale_ref::get<std::locale>() const;

// Explicit instantiations for char.

template FMT_API char internal::thousands_sep_impl(locale_ref);

template FMT_API void internal::basic_buffer<char>::append(const char *, const char *);

template FMT_API void internal::arg_map<format_context>::init(const basic_format_args<format_context> &args);

template FMT_API int internal::char_traits<char>::format_float(char *, std::size_t, const char *, int, double);

template FMT_API int internal::char_traits<char>::format_float(char *, std::size_t, const char *, int, long double);

template FMT_API std::string internal::vformat<char>(string_view, basic_format_args<format_context>);

template FMT_API format_context::iterator internal::vformat_to(internal::buffer &, string_view, basic_format_args<format_context>);

template FMT_API void internal::sprintf_format(double, internal::buffer &, core_format_specs);
template FMT_API void internal::sprintf_format(long double, internal::buffer &, core_format_specs);

// Explicit instantiations for wchar_t.

template FMT_API wchar_t internal::thousands_sep_impl(locale_ref);

template FMT_API void internal::basic_buffer<wchar_t>::append(const wchar_t *, const wchar_t *);

template FMT_API void internal::arg_map<wformat_context>::init(const basic_format_args<wformat_context> &);

template FMT_API int internal::char_traits<wchar_t>::format_float(wchar_t *, std::size_t, const wchar_t *, int, double);

template FMT_API int internal::char_traits<wchar_t>::format_float(wchar_t *, std::size_t, const wchar_t *, int, long double);

template FMT_API std::wstring internal::vformat<wchar_t>(wstring_view, basic_format_args<wformat_context>);
FMT_END_NAMESPACE

#endif