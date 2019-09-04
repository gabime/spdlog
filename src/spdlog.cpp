// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef SPDLOG_COMPILED_LIB
#error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

#include <mutex>

#include "spdlog/details/null_mutex.h"
#include "spdlog/async.h"

#include "spdlog/spdlog-inl.h"
#include "spdlog/common-inl.h"
#include "spdlog/details/backtracer-inl.h"
#include "spdlog/logger-inl.h"
template spdlog::logger::logger(std::string name, sinks_init_list::iterator begin, sinks_init_list::iterator end);

#include "spdlog/async_logger-inl.h"
#include "spdlog/details/log_msg-inl.h"
#include "spdlog/sinks/sink-inl.h"

#include "spdlog/sinks/base_sink-inl.h"
template class spdlog::sinks::base_sink<std::mutex>;
template class spdlog::sinks::base_sink<spdlog::details::null_mutex>;

#include "spdlog/sinks/basic_file_sink-inl.h"
template class spdlog::sinks::basic_file_sink<std::mutex>;
template class spdlog::sinks::basic_file_sink<spdlog::details::null_mutex>;

#include "spdlog/sinks/rotating_file_sink-inl.h"
template class spdlog::sinks::rotating_file_sink<std::mutex>;
template class spdlog::sinks::rotating_file_sink<spdlog::details::null_mutex>;

#include "spdlog/details/registry-inl.h"

#include "spdlog/details/os-inl.h"
#include "spdlog/details/periodic_worker-inl.h"
#include "spdlog/details/file_helper-inl.h"
#include "spdlog/details/pattern_formatter-inl.h"

#include "spdlog/details/thread_pool-inl.h"
template class spdlog::details::mpmc_blocking_queue<spdlog::details::async_msg>;

//
// color sinks
//
#ifdef _WIN32
#include "spdlog/sinks/wincolor_sink-inl.h"
template class spdlog::sinks::wincolor_sink<spdlog::details::console_mutex>;
template class spdlog::sinks::wincolor_sink<spdlog::details::console_nullmutex>;
template class spdlog::sinks::wincolor_stdout_sink<spdlog::details::console_mutex>;
template class spdlog::sinks::wincolor_stdout_sink<spdlog::details::console_nullmutex>;
template class spdlog::sinks::wincolor_stderr_sink<spdlog::details::console_mutex>;
template class spdlog::sinks::wincolor_stderr_sink<spdlog::details::console_nullmutex>;
#else
#include "spdlog/sinks/ansicolor_sink-inl.h"
template class spdlog::sinks::ansicolor_sink<spdlog::details::console_mutex>;
template class spdlog::sinks::ansicolor_sink<spdlog::details::console_nullmutex>;
template class spdlog::sinks::ansicolor_stdout_sink<spdlog::details::console_mutex>;
template class spdlog::sinks::ansicolor_stdout_sink<spdlog::details::console_nullmutex>;
template class spdlog::sinks::ansicolor_stderr_sink<spdlog::details::console_mutex>;
template class spdlog::sinks::ansicolor_stderr_sink<spdlog::details::console_nullmutex>;
#endif

// factory methods for color loggers
#include "spdlog/sinks/stdout_color_sinks-inl.h"
template std::shared_ptr<spdlog::logger> spdlog::stdout_color_mt<spdlog::synchronous_factory>(
    const std::string &logger_name, color_mode mode);
template std::shared_ptr<spdlog::logger> spdlog::stdout_color_st<spdlog::synchronous_factory>(
    const std::string &logger_name, color_mode mode);
template std::shared_ptr<spdlog::logger> spdlog::stderr_color_mt<spdlog::synchronous_factory>(
    const std::string &logger_name, color_mode mode);
template std::shared_ptr<spdlog::logger> spdlog::stderr_color_st<spdlog::synchronous_factory>(
    const std::string &logger_name, color_mode mode);

template std::shared_ptr<spdlog::logger> spdlog::stdout_color_mt<spdlog::async_factory>(const std::string &logger_name, color_mode mode);
template std::shared_ptr<spdlog::logger> spdlog::stdout_color_st<spdlog::async_factory>(const std::string &logger_name, color_mode mode);
template std::shared_ptr<spdlog::logger> spdlog::stderr_color_mt<spdlog::async_factory>(const std::string &logger_name, color_mode mode);
template std::shared_ptr<spdlog::logger> spdlog::stderr_color_st<spdlog::async_factory>(const std::string &logger_name, color_mode mode);

#include "spdlog/sinks/stdout_sinks-inl.h"

template class spdlog::sinks::stdout_sink_base<spdlog::details::console_mutex>;
template class spdlog::sinks::stdout_sink_base<spdlog::details::console_nullmutex>;
template class spdlog::sinks::stdout_sink<spdlog::details::console_mutex>;
template class spdlog::sinks::stdout_sink<spdlog::details::console_nullmutex>;
template class spdlog::sinks::stderr_sink<spdlog::details::console_mutex>;
template class spdlog::sinks::stderr_sink<spdlog::details::console_nullmutex>;

template std::shared_ptr<spdlog::logger> spdlog::stdout_logger_mt<spdlog::synchronous_factory>(const std::string &logger_name);
template std::shared_ptr<spdlog::logger> spdlog::stdout_logger_st<spdlog::synchronous_factory>(const std::string &logger_name);
template std::shared_ptr<spdlog::logger> spdlog::stderr_logger_mt<spdlog::synchronous_factory>(const std::string &logger_name);
template std::shared_ptr<spdlog::logger> spdlog::stderr_logger_st<spdlog::synchronous_factory>(const std::string &logger_name);

template std::shared_ptr<spdlog::logger> spdlog::stdout_logger_mt<spdlog::async_factory>(const std::string &logger_name);
template std::shared_ptr<spdlog::logger> spdlog::stdout_logger_st<spdlog::async_factory>(const std::string &logger_name);
template std::shared_ptr<spdlog::logger> spdlog::stderr_logger_mt<spdlog::async_factory>(const std::string &logger_name);
template std::shared_ptr<spdlog::logger> spdlog::stderr_logger_st<spdlog::async_factory>(const std::string &logger_name);

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
