// Formatting library for C++ - experimental range support
//
// Copyright (c) 2012 - present, Victor Zverovich
// All rights reserved.
//
// For the license information refer to format.h.
//
// Copyright (c) 2018 - present, Remotion (Igor Schulz)
// All Rights Reserved
// {fmt} support for ranges, containers and types tuple interface.

#ifndef FMT_RANGES_H_
#define FMT_RANGES_H_

#include <initializer_list>
#include <type_traits>

#include "format.h"

FMT_BEGIN_NAMESPACE

template <typename Char, typename Enable = void> struct formatting_range {
#ifdef FMT_DEPRECATED_BRACED_RANGES
  Char prefix = '{';
  Char postfix = '}';
#else
  Char prefix = '[';
  Char postfix = ']';
#endif

  template <typename ParseContext>
  FMT_CONSTEXPR auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
    return ctx.begin();
  }
};

template <typename Char, typename Enable = void> struct formatting_tuple {
  Char prefix = '(';
  Char postfix = ')';

  template <typename ParseContext>
  FMT_CONSTEXPR auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
    return ctx.begin();
  }
};

namespace detail {

template <typename RangeT, typename OutputIterator>
OutputIterator copy(const RangeT& range, OutputIterator out) {
  for (auto it = range.begin(), end = range.end(); it != end; ++it)
    *out++ = *it;
  return out;
}

template <typename OutputIterator>
OutputIterator copy(const char* str, OutputIterator out) {
  while (*str) *out++ = *str++;
  return out;
}

template <typename OutputIterator>
OutputIterator copy(char ch, OutputIterator out) {
  *out++ = ch;
  return out;
}

template <typename OutputIterator>
OutputIterator copy(wchar_t ch, OutputIterator out) {
  *out++ = ch;
  return out;
}

/// Return true value if T has std::string interface, like std::string_view.
template <typename T> class is_std_string_like {
  template <typename U>
  static auto check(U* p)
      -> decltype((void)p->find('a'), p->length(), (void)p->data(), int());
  template <typename> static void check(...);

 public:
  static FMT_CONSTEXPR_DECL const bool value =
      is_string<T>::value || !std::is_void<decltype(check<T>(nullptr))>::value;
};

template <typename Char>
struct is_std_string_like<fmt::basic_string_view<Char>> : std::true_type {};

template <typename... Ts> struct conditional_helper {};

template <typename T, typename _ = void> struct is_range_ : std::false_type {};

#if !FMT_MSC_VER || FMT_MSC_VER > 1800

#  define FMT_DECLTYPE_RETURN(val)  \
    ->decltype(val) { return val; } \
    static_assert(                  \
        true, "")  // This makes it so that a semicolon is required after the
                   // macro, which helps clang-format handle the formatting.

// C array overload
template <typename T, std::size_t N>
auto range_begin(const T (&arr)[N]) -> const T* {
  return arr;
}
template <typename T, std::size_t N>
auto range_end(const T (&arr)[N]) -> const T* {
  return arr + N;
}

template <typename T, typename Enable = void>
struct has_member_fn_begin_end_t : std::false_type {};

template <typename T>
struct has_member_fn_begin_end_t<T, void_t<decltype(std::declval<T>().begin()),
                                           decltype(std::declval<T>().end())>>
    : std::true_type {};

// Member function overload
template <typename T>
auto range_begin(T&& rng) FMT_DECLTYPE_RETURN(static_cast<T&&>(rng).begin());
template <typename T>
auto range_end(T&& rng) FMT_DECLTYPE_RETURN(static_cast<T&&>(rng).end());

// ADL overload. Only participates in overload resolution if member functions
// are not found.
template <typename T>
auto range_begin(T&& rng)
    -> enable_if_t<!has_member_fn_begin_end_t<T&&>::value,
                   decltype(begin(static_cast<T&&>(rng)))> {
  return begin(static_cast<T&&>(rng));
}
template <typename T>
auto range_end(T&& rng) -> enable_if_t<!has_member_fn_begin_end_t<T&&>::value,
                                       decltype(end(static_cast<T&&>(rng)))> {
  return end(static_cast<T&&>(rng));
}

template <typename T, typename Enable = void>
struct has_const_begin_end : std::false_type {};
template <typename T, typename Enable = void>
struct has_mutable_begin_end : std::false_type {};

template <typename T>
struct has_const_begin_end<
    T, void_t<decltype(detail::range_begin(
                  std::declval<const remove_cvref_t<T>&>())),
              decltype(detail::range_begin(
                  std::declval<const remove_cvref_t<T>&>()))>>
    : std::true_type {};

template <typename T>
struct has_mutable_begin_end<
    T, void_t<decltype(detail::range_begin(std::declval<T>())),
              decltype(detail::range_begin(std::declval<T>())),
              enable_if_t<std::is_copy_constructible<T>::value>>>
    : std::true_type {};

template <typename T>
struct is_range_<T, void>
    : std::integral_constant<bool, (has_const_begin_end<T>::value ||
                                    has_mutable_begin_end<T>::value)> {};

template <typename T, typename Enable = void> struct range_to_view;
template <typename T>
struct range_to_view<T, enable_if_t<has_const_begin_end<T>::value>> {
  struct view_t {
    const T* m_range_ptr;

    auto begin() const FMT_DECLTYPE_RETURN(detail::range_begin(*m_range_ptr));
    auto end() const FMT_DECLTYPE_RETURN(detail::range_end(*m_range_ptr));
  };
  static auto view(const T& range) -> view_t { return {&range}; }
};

template <typename T>
struct range_to_view<T, enable_if_t<!has_const_begin_end<T>::value &&
                                    has_mutable_begin_end<T>::value>> {
  struct view_t {
    T m_range_copy;

    auto begin() FMT_DECLTYPE_RETURN(detail::range_begin(m_range_copy));
    auto end() FMT_DECLTYPE_RETURN(detail::range_end(m_range_copy));
  };
  static auto view(const T& range) -> view_t { return {range}; }
};
#  undef FMT_DECLTYPE_RETURN
#endif

/// tuple_size and tuple_element check.
template <typename T> class is_tuple_like_ {
  template <typename U>
  static auto check(U* p) -> decltype(std::tuple_size<U>::value, int());
  template <typename> static void check(...);

 public:
  static FMT_CONSTEXPR_DECL const bool value =
      !std::is_void<decltype(check<T>(nullptr))>::value;
};

// Check for integer_sequence
#if defined(__cpp_lib_integer_sequence) || FMT_MSC_VER >= 1900
template <typename T, T... N>
using integer_sequence = std::integer_sequence<T, N...>;
template <size_t... N> using index_sequence = std::index_sequence<N...>;
template <size_t N> using make_index_sequence = std::make_index_sequence<N>;
#else
template <typename T, T... N> struct integer_sequence {
  using value_type = T;

  static FMT_CONSTEXPR size_t size() { return sizeof...(N); }
};

template <size_t... N> using index_sequence = integer_sequence<size_t, N...>;

template <typename T, size_t N, T... Ns>
struct make_integer_sequence : make_integer_sequence<T, N - 1, N - 1, Ns...> {};
template <typename T, T... Ns>
struct make_integer_sequence<T, 0, Ns...> : integer_sequence<T, Ns...> {};

template <size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;
#endif

template <class Tuple, class F, size_t... Is>
void for_each(index_sequence<Is...>, Tuple&& tup, F&& f) FMT_NOEXCEPT {
  using std::get;
  // using free function get<I>(T) now.
  const int _[] = {0, ((void)f(get<Is>(tup)), 0)...};
  (void)_;  // blocks warnings
}

template <class T>
FMT_CONSTEXPR make_index_sequence<std::tuple_size<T>::value> get_indexes(
    T const&) {
  return {};
}

template <class Tuple, class F> void for_each(Tuple&& tup, F&& f) {
  const auto indexes = get_indexes(tup);
  for_each(indexes, std::forward<Tuple>(tup), std::forward<F>(f));
}

template <typename Range>
using value_type =
    remove_cvref_t<decltype(*detail::range_begin(std::declval<Range>()))>;

template <typename OutputIt> OutputIt write_delimiter(OutputIt out) {
  *out++ = ',';
  *out++ = ' ';
  return out;
}

template <
    typename Char, typename OutputIt, typename Arg,
    FMT_ENABLE_IF(is_std_string_like<typename std::decay<Arg>::type>::value)>
OutputIt write_range_entry(OutputIt out, const Arg& v) {
  *out++ = '"';
  out = write<Char>(out, v);
  *out++ = '"';
  return out;
}

template <typename Char, typename OutputIt, typename Arg,
          FMT_ENABLE_IF(std::is_same<Arg, Char>::value)>
OutputIt write_range_entry(OutputIt out, const Arg v) {
  *out++ = '\'';
  *out++ = v;
  *out++ = '\'';
  return out;
}

template <
    typename Char, typename OutputIt, typename Arg,
    FMT_ENABLE_IF(!is_std_string_like<typename std::decay<Arg>::type>::value &&
                  !std::is_same<Arg, Char>::value)>
OutputIt write_range_entry(OutputIt out, const Arg& v) {
  return write<Char>(out, v);
}

}  // namespace detail

template <typename T> struct is_tuple_like {
  static FMT_CONSTEXPR_DECL const bool value =
      detail::is_tuple_like_<T>::value && !detail::is_range_<T>::value;
};

template <typename TupleT, typename Char>
struct formatter<TupleT, Char, enable_if_t<fmt::is_tuple_like<TupleT>::value>> {
 private:
  // C++11 generic lambda for format()
  template <typename FormatContext> struct format_each {
    template <typename T> void operator()(const T& v) {
      if (i > 0) out = detail::write_delimiter(out);
      out = detail::write_range_entry<Char>(out, v);
      ++i;
    }
    formatting_tuple<Char>& formatting;
    size_t& i;
    typename std::add_lvalue_reference<
        decltype(std::declval<FormatContext>().out())>::type out;
  };

 public:
  formatting_tuple<Char> formatting;

  template <typename ParseContext>
  FMT_CONSTEXPR auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
    return formatting.parse(ctx);
  }

  template <typename FormatContext = format_context>
  auto format(const TupleT& values, FormatContext& ctx) -> decltype(ctx.out()) {
    auto out = ctx.out();
    size_t i = 0;

    detail::copy(formatting.prefix, out);
    detail::for_each(values, format_each<FormatContext>{formatting, i, out});
    detail::copy(formatting.postfix, out);

    return ctx.out();
  }
};

template <typename T, typename Char> struct is_range {
  static FMT_CONSTEXPR_DECL const bool value =
      detail::is_range_<T>::value && !detail::is_std_string_like<T>::value &&
      !std::is_convertible<T, std::basic_string<Char>>::value &&
      !std::is_constructible<detail::std_string_view<Char>, T>::value;
};

template <typename T, typename Char>
struct formatter<
    T, Char,
    enable_if_t<
        fmt::is_range<T, Char>::value
// Workaround a bug in MSVC 2017 and earlier.
#if !FMT_MSC_VER || FMT_MSC_VER >= 1927
        && (has_formatter<detail::value_type<T>, format_context>::value ||
            detail::has_fallback_formatter<detail::value_type<T>, Char>::value)
#endif
        >> {
  formatting_range<Char> formatting;

  template <typename ParseContext>
  FMT_CONSTEXPR auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
    return formatting.parse(ctx);
  }

  template <typename FormatContext>
  typename FormatContext::iterator format(const T& values, FormatContext& ctx) {
    auto out = detail::copy(formatting.prefix, ctx.out());
    size_t i = 0;
    auto view = detail::range_to_view<T>::view(values);
    auto it = view.begin();
    auto end = view.end();
    for (; it != end; ++it) {
      if (i > 0) out = detail::write_delimiter(out);
      out = detail::write_range_entry<Char>(out, *it);
      ++i;
    }
    return detail::copy(formatting.postfix, out);
  }
};

template <typename Char, typename... T> struct tuple_join_view : detail::view {
  const std::tuple<T...>& tuple;
  basic_string_view<Char> sep;

  tuple_join_view(const std::tuple<T...>& t, basic_string_view<Char> s)
      : tuple(t), sep{s} {}
};

template <typename Char, typename... T>
using tuple_arg_join = tuple_join_view<Char, T...>;

template <typename Char, typename... T>
struct formatter<tuple_join_view<Char, T...>, Char> {
  template <typename ParseContext>
  FMT_CONSTEXPR auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const tuple_join_view<Char, T...>& value, FormatContext& ctx) ->
      typename FormatContext::iterator {
    return format(value, ctx, detail::make_index_sequence<sizeof...(T)>{});
  }

 private:
  template <typename FormatContext, size_t... N>
  auto format(const tuple_join_view<Char, T...>& value, FormatContext& ctx,
              detail::index_sequence<N...>) ->
      typename FormatContext::iterator {
    using std::get;
    return format_args(value, ctx, get<N>(value.tuple)...);
  }

  template <typename FormatContext>
  auto format_args(const tuple_join_view<Char, T...>&, FormatContext& ctx) ->
      typename FormatContext::iterator {
    // NOTE: for compilers that support C++17, this empty function instantiation
    // can be replaced with a constexpr branch in the variadic overload.
    return ctx.out();
  }

  template <typename FormatContext, typename Arg, typename... Args>
  auto format_args(const tuple_join_view<Char, T...>& value, FormatContext& ctx,
                   const Arg& arg, const Args&... args) ->
      typename FormatContext::iterator {
    using base = formatter<typename std::decay<Arg>::type, Char>;
    auto out = base().format(arg, ctx);
    if (sizeof...(Args) > 0) {
      out = std::copy(value.sep.begin(), value.sep.end(), out);
      ctx.advance_to(out);
      return format_args(value, ctx, args...);
    }
    return out;
  }
};

FMT_MODULE_EXPORT_BEGIN

/**
  \rst
  Returns an object that formats `tuple` with elements separated by `sep`.

  **Example**::

    std::tuple<int, char> t = {1, 'a'};
    fmt::print("{}", fmt::join(t, ", "));
    // Output: "1, a"
  \endrst
 */
template <typename... T>
FMT_CONSTEXPR auto join(const std::tuple<T...>& tuple, string_view sep)
    -> tuple_join_view<char, T...> {
  return {tuple, sep};
}

template <typename... T>
FMT_CONSTEXPR auto join(const std::tuple<T...>& tuple,
                        basic_string_view<wchar_t> sep)
    -> tuple_join_view<wchar_t, T...> {
  return {tuple, sep};
}

/**
  \rst
  Returns an object that formats `initializer_list` with elements separated by
  `sep`.

  **Example**::

    fmt::print("{}", fmt::join({1, 2, 3}, ", "));
    // Output: "1, 2, 3"
  \endrst
 */
template <typename T>
auto join(std::initializer_list<T> list, string_view sep)
    -> join_view<const T*, const T*> {
  return join(std::begin(list), std::end(list), sep);
}

FMT_MODULE_EXPORT_END
FMT_END_NAMESPACE

#endif  // FMT_RANGES_H_
