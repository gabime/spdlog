// Formatting library for C++ - std::locale support
//
// Copyright (c) 2012 - present, Victor Zverovich
// All rights reserved.
//
// For the license information refer to format.h.

#ifndef FMT_LOCALE_H_
#define FMT_LOCALE_H_

#include "format.h"
#include <locale>

FMT_BEGIN_NAMESPACE

namespace internal {
template <typename Char>
typename buffer_context<Char>::type::iterator vformat_to(
    const std::locale &loc, basic_buffer<Char> &buf,
    basic_string_view<Char> format_str,
    basic_format_args<typename buffer_context<Char>::type> args) {
  typedef back_insert_range<basic_buffer<Char> > range;
  return vformat_to<arg_formatter<range>>(
    buf, to_string_view(format_str), args, internal::locale_ref(loc));
}

template <typename Char>
std::basic_string<Char> vformat(
    const std::locale &loc, basic_string_view<Char> format_str,
    basic_format_args<typename buffer_context<Char>::type> args) {
  basic_memory_buffer<Char> buffer;
  internal::vformat_to(loc, buffer, format_str, args);
  return fmt::to_string(buffer);
}
}

template <typename S, typename Char = FMT_CHAR(S)>
inline std::basic_string<Char> vformat(
    const std::locale &loc, const S &format_str,
    basic_format_args<typename buffer_context<Char>::type> args) {
  return internal::vformat(loc, to_string_view(format_str), args);
}

template <typename S, typename... Args>
inline std::basic_string<FMT_CHAR(S)> format(
    const std::locale &loc, const S &format_str, const Args &... args) {
  return internal::vformat(
    loc, to_string_view(format_str),
    *internal::checked_args<S, Args...>(format_str, args...));
}

template <typename String, typename OutputIt, typename... Args>
inline typename std::enable_if<internal::is_output_iterator<OutputIt>::value,
                               OutputIt>::type
    vformat_to(OutputIt out, const std::locale &loc, const String &format_str,
               typename format_args_t<OutputIt, FMT_CHAR(String)>::type args) {
  typedef output_range<OutputIt, FMT_CHAR(String)> range;
  return vformat_to<arg_formatter<range>>(
    range(out), to_string_view(format_str), args, internal::locale_ref(loc));
}

template <typename OutputIt, typename S, typename... Args>
inline typename std::enable_if<
    internal::is_string<S>::value &&
    internal::is_output_iterator<OutputIt>::value, OutputIt>::type
    format_to(OutputIt out, const std::locale &loc, const S &format_str,
              const Args &... args) {
  internal::check_format_string<Args...>(format_str);
  typedef typename format_context_t<OutputIt, FMT_CHAR(S)>::type context;
  format_arg_store<context, Args...> as{args...};
  return vformat_to(out, loc, to_string_view(format_str),
                    basic_format_args<context>(as));
}

FMT_END_NAMESPACE

#endif  // FMT_LOCALE_H_
