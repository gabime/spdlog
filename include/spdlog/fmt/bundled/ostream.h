// Formatting library for C++ - std::ostream support
//
// Copyright (c) 2012 - present, Victor Zverovich
// All rights reserved.
//
// For the license information refer to format.h.

#ifndef FMT_OSTREAM_H_
#define FMT_OSTREAM_H_

#include "format.h"
#include <ostream>

FMT_BEGIN_NAMESPACE
namespace internal {

template <class Char>
class formatbuf : public std::basic_streambuf<Char> {
 private:
  typedef typename std::basic_streambuf<Char>::int_type int_type;
  typedef typename std::basic_streambuf<Char>::traits_type traits_type;

  basic_buffer<Char> &buffer_;

 public:
  formatbuf(basic_buffer<Char> &buffer) : buffer_(buffer) {}

 protected:
  // The put-area is actually always empty. This makes the implementation
  // simpler and has the advantage that the streambuf and the buffer are always
  // in sync and sputc never writes into uninitialized memory. The obvious
  // disadvantage is that each call to sputc always results in a (virtual) call
  // to overflow. There is no disadvantage here for sputn since this always
  // results in a call to xsputn.

  int_type overflow(int_type ch = traits_type::eof()) FMT_OVERRIDE {
    if (!traits_type::eq_int_type(ch, traits_type::eof()))
      buffer_.push_back(static_cast<Char>(ch));
    return ch;
  }

  std::streamsize xsputn(const Char *s, std::streamsize count) FMT_OVERRIDE {
    buffer_.append(s, s + count);
    return count;
  }
};

template <typename Char>
struct test_stream : std::basic_ostream<Char> {
 private:
  struct null;
  // Hide all operator<< from std::basic_ostream<Char>.
  void operator<<(null);
};

// Checks if T has a user-defined operator<< (e.g. not a member of std::ostream).
template <typename T, typename Char>
class is_streamable {
 private:
  template <typename U>
  static decltype(
    internal::declval<test_stream<Char>&>()
      << internal::declval<U>(), std::true_type()) test(int);

  template <typename>
  static std::false_type test(...);

  typedef decltype(test<T>(0)) result;

 public:
  static const bool value = result::value;
};

// Write the content of buf to os.
template <typename Char>
void write(std::basic_ostream<Char> &os, basic_buffer<Char> &buf) {
  const Char *data = buf.data();
  typedef std::make_unsigned<std::streamsize>::type UnsignedStreamSize;
  UnsignedStreamSize size = buf.size();
  UnsignedStreamSize max_size =
      internal::to_unsigned((std::numeric_limits<std::streamsize>::max)());
  do {
    UnsignedStreamSize n = size <= max_size ? size : max_size;
    os.write(data, static_cast<std::streamsize>(n));
    data += n;
    size -= n;
  } while (size != 0);
}

template <typename Char, typename T>
void format_value(basic_buffer<Char> &buffer, const T &value) {
  internal::formatbuf<Char> format_buf(buffer);
  std::basic_ostream<Char> output(&format_buf);
  output.exceptions(std::ios_base::failbit | std::ios_base::badbit);
  output << value;
  buffer.resize(buffer.size());
}
}  // namespace internal

// Disable conversion to int if T has an overloaded operator<< which is a free
// function (not a member of std::ostream).
template <typename T, typename Char>
struct convert_to_int<T, Char, void> {
  static const bool value =
    convert_to_int<T, Char, int>::value &&
    !internal::is_streamable<T, Char>::value;
};

// Formats an object of type T that has an overloaded ostream operator<<.
template <typename T, typename Char>
struct formatter<T, Char,
    typename std::enable_if<
      internal::is_streamable<T, Char>::value &&
      !internal::format_type<
        typename buffer_context<Char>::type, T>::value>::type>
    : formatter<basic_string_view<Char>, Char> {

  template <typename Context>
  auto format(const T &value, Context &ctx) -> decltype(ctx.out()) {
    basic_memory_buffer<Char> buffer;
    internal::format_value(buffer, value);
    basic_string_view<Char> str(buffer.data(), buffer.size());
    return formatter<basic_string_view<Char>, Char>::format(str, ctx);
  }
};

template <typename Char>
inline void vprint(std::basic_ostream<Char> &os,
                   basic_string_view<Char> format_str,
                   basic_format_args<typename buffer_context<Char>::type> args) {
  basic_memory_buffer<Char> buffer;
  internal::vformat_to(buffer, format_str, args);
  internal::write(os, buffer);
}
/**
  \rst
  Prints formatted data to the stream *os*.

  **Example**::

    fmt::print(cerr, "Don't {}!", "panic");
  \endrst
 */
template <typename S, typename... Args>
inline typename std::enable_if<internal::is_string<S>::value>::type
print(std::basic_ostream<FMT_CHAR(S)> &os, const S &format_str,
      const Args & ... args) {
  internal::checked_args<S, Args...> ca(format_str, args...);
  vprint(os, to_string_view(format_str), *ca);
}
FMT_END_NAMESPACE

#endif  // FMT_OSTREAM_H_
