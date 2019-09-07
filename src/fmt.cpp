#ifndef SPDLOG_COMPILED_LIB
#error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

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
template FMT_API char internal::decimal_point_impl(locale_ref);
template FMT_API void internal::buffer<char>::append(const char *, const char *);
template FMT_API void internal::arg_map<format_context>::init(const basic_format_args<format_context> &args);
template FMT_API std::string internal::vformat<char>(string_view, basic_format_args<format_context>);
template FMT_API format_context::iterator internal::vformat_to(internal::buffer<char> &, string_view, basic_format_args<format_context>);
template FMT_API char *internal::sprintf_format(double, internal::buffer<char> &, sprintf_specs);
template FMT_API char *internal::sprintf_format(long double, internal::buffer<char> &, sprintf_specs);

// Explicit instantiations for wchar_t.
template FMT_API wchar_t internal::thousands_sep_impl(locale_ref);
template FMT_API wchar_t internal::decimal_point_impl(locale_ref);
template FMT_API void internal::buffer<wchar_t>::append(const wchar_t *, const wchar_t *);
template FMT_API void internal::arg_map<wformat_context>::init(const basic_format_args<wformat_context> &);
template FMT_API std::wstring internal::vformat<wchar_t>(wstring_view, basic_format_args<wformat_context>);
FMT_END_NAMESPACE

#endif
