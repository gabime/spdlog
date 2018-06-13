// Formatting library for C++
//
// Copyright (c) 2012 - 2016, Victor Zverovich
// All rights reserved.
//
// For the license information refer to format.h.

#include "fmt/format-inl.h"

namespace fmt {

template struct internal::basic_data<void>;

// Explicit instantiations for char.

template FMT_API char internal::thousands_sep(locale_provider *lp);

template void basic_fixed_buffer<char>::grow(std::size_t);

template void internal::arg_map<format_context>::init(
    const basic_format_args<format_context> &args);

template FMT_API int internal::char_traits<char>::format_float(
    char *buffer, std::size_t size, const char *format,
    unsigned width, int precision, double value);

template FMT_API int internal::char_traits<char>::format_float(
    char *buffer, std::size_t size, const char *format,
    unsigned width, int precision, long double value);

// Explicit instantiations for wchar_t.

template FMT_API wchar_t internal::thousands_sep(locale_provider *lp);

template void basic_fixed_buffer<wchar_t>::grow(std::size_t);

template void internal::arg_map<wformat_context>::init(const wformat_args &args);

template FMT_API int internal::char_traits<wchar_t>::format_float(
    wchar_t *buffer, std::size_t size, const wchar_t *format,
    unsigned width, int precision, double value);

template FMT_API int internal::char_traits<wchar_t>::format_float(
    wchar_t *buffer, std::size_t size, const wchar_t *format,
    unsigned width, int precision, long double value);
}  // namespace fmt
