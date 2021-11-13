// Slightly modified version of fmt lib's format.cc source file.
// Copyright (c) 2012 - 2016, Victor Zverovich
// All rights reserved.

#ifndef SPDLOG_COMPILED_LIB
#    error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

#if !defined(SPDLOG_FMT_EXTERNAL) && !defined(SPDLOG_USE_STD_FORMAT)
#    include <spdlog/fmt/bundled/format-inl.h>

FMT_BEGIN_NAMESPACE
namespace detail {

template<typename T>
int format_float(char *buf, std::size_t size, const char *format, int precision, T value)
{
#    ifdef FMT_FUZZ
    if (precision > 100000)
        throw std::runtime_error("fuzz mode - avoid large allocation inside snprintf");
#    endif
    // Suppress the warning about nonliteral format string.
    int (*snprintf_ptr)(char *, size_t, const char *, ...) = FMT_SNPRINTF;
    return precision < 0 ? snprintf_ptr(buf, size, format, value) : snprintf_ptr(buf, size, format, precision, value);
}

template FMT_API dragonbox::decimal_fp<float> dragonbox::to_decimal(float x) FMT_NOEXCEPT;
template FMT_API dragonbox::decimal_fp<double> dragonbox::to_decimal(double x) FMT_NOEXCEPT;
} // namespace detail

// Workaround a bug in MSVC2013 that prevents instantiation of format_float.
int (*instantiate_format_float)(double, int, detail::float_specs, detail::buffer<char> &) = detail::format_float;

#    ifndef FMT_STATIC_THOUSANDS_SEPARATOR
template FMT_API detail::locale_ref::locale_ref(const std::locale &loc);
template FMT_API std::locale detail::locale_ref::get<std::locale>() const;
#    endif

// Explicit instantiations for char.

template FMT_API auto detail::thousands_sep_impl(locale_ref) -> thousands_sep_result<char>;
template FMT_API char detail::decimal_point_impl(locale_ref);

template FMT_API void detail::buffer<char>::append(const char *, const char *);

// DEPRECATED!
// There is no correspondent extern template in format.h because of
// incompatibility between clang and gcc (#2377).
template FMT_API void detail::vformat_to(
    detail::buffer<char> &, string_view, basic_format_args<FMT_BUFFER_CONTEXT(char)>, detail::locale_ref);

template FMT_API int detail::snprintf_float(double, int, detail::float_specs, detail::buffer<char> &);
template FMT_API int detail::snprintf_float(long double, int, detail::float_specs, detail::buffer<char> &);
template FMT_API int detail::format_float(double, int, detail::float_specs, detail::buffer<char> &);
template FMT_API int detail::format_float(long double, int, detail::float_specs, detail::buffer<char> &);

// Explicit instantiations for wchar_t.

template FMT_API auto detail::thousands_sep_impl(locale_ref) -> thousands_sep_result<wchar_t>;
template FMT_API wchar_t detail::decimal_point_impl(locale_ref);

template FMT_API void detail::buffer<wchar_t>::append(const wchar_t *, const wchar_t *);

template struct detail::basic_data<void>;

FMT_END_NAMESPACE

#endif // !SPDLOG_FMT_EXTERNAL
