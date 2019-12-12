#ifndef SPDLOG_COMPILED_LIB
#error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

// Slightly modified version of fmt lib's format.cc source file.
// Copyright (c) 2012 - 2016, Victor Zverovich
// All rights reserved.

#if !defined(SPDLOG_FMT_EXTERNAL)
#include "spdlog/fmt/bundled/format-inl.h"

FMT_BEGIN_NAMESPACE
template struct FMT_API internal::basic_data<void>;

// Workaround a bug in MSVC2013 that prevents instantiation of format_float.
int (*instantiate_format_float)(double, int, internal::float_specs, internal::buffer<char> &) = internal::format_float;

#ifndef FMT_STATIC_THOUSANDS_SEPARATOR
template FMT_API internal::locale_ref::locale_ref(const std::locale &loc);
template FMT_API std::locale internal::locale_ref::get<std::locale>() const;
#endif

// Explicit instantiations for char.

template FMT_API std::string internal::grouping_impl<char>(locale_ref);
template FMT_API char internal::thousands_sep_impl(locale_ref);
template FMT_API char internal::decimal_point_impl(locale_ref);

template FMT_API void internal::buffer<char>::append(const char *, const char *);

template FMT_API void internal::arg_map<format_context>::init(const basic_format_args<format_context> &args);

template FMT_API std::string internal::vformat<char>(string_view, basic_format_args<format_context>);

template FMT_API format_context::iterator internal::vformat_to(internal::buffer<char> &, string_view, basic_format_args<format_context>);

template FMT_API int internal::snprintf_float(double, int, internal::float_specs, internal::buffer<char> &);
template FMT_API int internal::snprintf_float(long double, int, internal::float_specs, internal::buffer<char> &);
template FMT_API int internal::format_float(double, int, internal::float_specs, internal::buffer<char> &);
template FMT_API int internal::format_float(long double, int, internal::float_specs, internal::buffer<char> &);

// Explicit instantiations for wchar_t.

template FMT_API std::string internal::grouping_impl<wchar_t>(locale_ref);
template FMT_API wchar_t internal::thousands_sep_impl(locale_ref);
template FMT_API wchar_t internal::decimal_point_impl(locale_ref);

template FMT_API void internal::buffer<wchar_t>::append(const wchar_t *, const wchar_t *);

template FMT_API std::wstring internal::vformat<wchar_t>(wstring_view, basic_format_args<wformat_context>);
FMT_END_NAMESPACE

#endif
