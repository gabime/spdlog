// Formatting library for C++
//
// Copyright (c) 2012 - 2016, Victor Zverovich
// All rights reserved.
//
// For the license information refer to format.h.

#ifndef FMT_PRINTF_H_
#define FMT_PRINTF_H_

#include <algorithm>  // std::fill_n
#include <limits>     // std::numeric_limits

#include "ostream.h"

FMT_BEGIN_NAMESPACE
namespace internal {

// Checks if a value fits in int - used to avoid warnings about comparing
// signed and unsigned integers.
template <bool IsSigned>
struct int_checker {
  template <typename T>
  static bool fits_in_int(T value) {
    unsigned max = std::numeric_limits<int>::max();
    return value <= max;
  }
  static bool fits_in_int(bool) { return true; }
};

template <>
struct int_checker<true> {
  template <typename T>
  static bool fits_in_int(T value) {
    return value >= std::numeric_limits<int>::min() &&
           value <= std::numeric_limits<int>::max();
  }
  static bool fits_in_int(int) { return true; }
};

class printf_precision_handler: public function<int> {
 public:
  template <typename T>
  typename std::enable_if<std::is_integral<T>::value, int>::type
      operator()(T value) {
    if (!int_checker<std::numeric_limits<T>::is_signed>::fits_in_int(value))
      FMT_THROW(format_error("number is too big"));
    return static_cast<int>(value);
  }

  template <typename T>
  typename std::enable_if<!std::is_integral<T>::value, int>::type
      operator()(T) {
    FMT_THROW(format_error("precision is not integer"));
    return 0;
  }
};

// An argument visitor that returns true iff arg is a zero integer.
class is_zero_int: public function<bool> {
 public:
  template <typename T>
  typename std::enable_if<std::is_integral<T>::value, bool>::type
      operator()(T value) { return value == 0; }

  template <typename T>
  typename std::enable_if<!std::is_integral<T>::value, bool>::type
      operator()(T) { return false; }
};

template <typename T>
struct make_unsigned_or_bool : std::make_unsigned<T> {};

template <>
struct make_unsigned_or_bool<bool> {
  typedef bool type;
};

template <typename T, typename Context>
class arg_converter: public function<void> {
 private:
  typedef typename Context::char_type Char;

  basic_format_arg<Context> &arg_;
  typename Context::char_type type_;

 public:
  arg_converter(basic_format_arg<Context> &arg, Char type)
    : arg_(arg), type_(type) {}

  void operator()(bool value) {
    if (type_ != 's')
      operator()<bool>(value);
  }

  template <typename U>
  typename std::enable_if<std::is_integral<U>::value>::type
      operator()(U value) {
    bool is_signed = type_ == 'd' || type_ == 'i';
    typedef typename std::conditional<
        std::is_same<T, void>::value, U, T>::type TargetType;
    if (const_check(sizeof(TargetType) <= sizeof(int))) {
      // Extra casts are used to silence warnings.
      if (is_signed) {
        arg_ = internal::make_arg<Context>(
          static_cast<int>(static_cast<TargetType>(value)));
      } else {
        typedef typename make_unsigned_or_bool<TargetType>::type Unsigned;
        arg_ = internal::make_arg<Context>(
          static_cast<unsigned>(static_cast<Unsigned>(value)));
      }
    } else {
      if (is_signed) {
        // glibc's printf doesn't sign extend arguments of smaller types:
        //   std::printf("%lld", -42);  // prints "4294967254"
        // but we don't have to do the same because it's a UB.
        arg_ = internal::make_arg<Context>(static_cast<long long>(value));
      } else {
        arg_ = internal::make_arg<Context>(
          static_cast<typename make_unsigned_or_bool<U>::type>(value));
      }
    }
  }

  template <typename U>
  typename std::enable_if<!std::is_integral<U>::value>::type operator()(U) {
    // No coversion needed for non-integral types.
  }
};

// Converts an integer argument to T for printf, if T is an integral type.
// If T is void, the argument is converted to corresponding signed or unsigned
// type depending on the type specifier: 'd' and 'i' - signed, other -
// unsigned).
template <typename T, typename Context, typename Char>
void convert_arg(basic_format_arg<Context> &arg, Char type) {
  visit(arg_converter<T, Context>(arg, type), arg);
}

// Converts an integer argument to char for printf.
template <typename Context>
class char_converter: public function<void> {
 private:
  basic_format_arg<Context> &arg_;

  FMT_DISALLOW_COPY_AND_ASSIGN(char_converter);

 public:
  explicit char_converter(basic_format_arg<Context> &arg) : arg_(arg) {}

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value>::type
      operator()(T value) {
    typedef typename Context::char_type Char;
    arg_ = internal::make_arg<Context>(static_cast<Char>(value));
  }

  template <typename T>
  typename std::enable_if<!std::is_integral<T>::value>::type operator()(T) {
    // No coversion needed for non-integral types.
  }
};

// Checks if an argument is a valid printf width specifier and sets
// left alignment if it is negative.
template <typename Char>
class printf_width_handler: public function<unsigned> {
 private:
  typedef basic_format_specs<Char> format_specs;

  format_specs &spec_;

  FMT_DISALLOW_COPY_AND_ASSIGN(printf_width_handler);

 public:
  explicit printf_width_handler(format_specs &spec) : spec_(spec) {}

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value, unsigned>::type
      operator()(T value) {
    typedef typename internal::int_traits<T>::main_type UnsignedType;
    UnsignedType width = static_cast<UnsignedType>(value);
    if (internal::is_negative(value)) {
      spec_.align_ = ALIGN_LEFT;
      width = 0 - width;
    }
    unsigned int_max = std::numeric_limits<int>::max();
    if (width > int_max)
      FMT_THROW(format_error("number is too big"));
    return static_cast<unsigned>(width);
  }

  template <typename T>
  typename std::enable_if<!std::is_integral<T>::value, unsigned>::type
      operator()(T) {
    FMT_THROW(format_error("width is not integer"));
    return 0;
  }
};
}  // namespace internal

template <typename Range>
class printf_arg_formatter;

template <
    typename OutputIt, typename Char,
    typename ArgFormatter =
      printf_arg_formatter<back_insert_range<internal::basic_buffer<Char>>>>
class basic_printf_context;

/**
  \rst
  The ``printf`` argument formatter.
  \endrst
 */
template <typename Range>
class printf_arg_formatter:
  public internal::function<
    typename internal::arg_formatter_base<Range>::iterator>,
  public internal::arg_formatter_base<Range> {
 private:
  typedef typename Range::value_type char_type;
  typedef decltype(internal::declval<Range>().begin()) iterator;
  typedef internal::arg_formatter_base<Range> base;
  typedef basic_printf_context<iterator, char_type> context_type;

  context_type &context_;

  void write_null_pointer(char) {
    this->spec().type_ = 0;
    this->write("(nil)");
  }

  void write_null_pointer(wchar_t) {
    this->spec().type_ = 0;
    this->write(L"(nil)");
  }

 public:
  typedef typename base::format_specs format_specs;

  /**
    \rst
    Constructs an argument formatter object.
    *buffer* is a reference to the output buffer and *spec* contains format
    specifier information for standard argument types.
    \endrst
   */
  printf_arg_formatter(internal::basic_buffer<char_type> &buffer,
                       format_specs &spec, context_type &ctx)
    : base(back_insert_range<internal::basic_buffer<char_type>>(buffer), spec),
      context_(ctx) {}

  using base::operator();

  /** Formats an argument of type ``bool``. */
  iterator operator()(bool value) {
    format_specs &fmt_spec = this->spec();
    if (fmt_spec.type_ != 's')
      return (*this)(value ? 1 : 0);
    fmt_spec.type_ = 0;
    this->write(value);
    return this->out();
  }

  /** Formats a character. */
  iterator operator()(char_type value) {
    format_specs &fmt_spec = this->spec();
    if (fmt_spec.type_ && fmt_spec.type_ != 'c')
      return (*this)(static_cast<int>(value));
    fmt_spec.flags_ = 0;
    fmt_spec.align_ = ALIGN_RIGHT;
    return base::operator()(value);
  }

  /** Formats a null-terminated C string. */
  iterator operator()(const char *value) {
    if (value)
      base::operator()(value);
    else if (this->spec().type_ == 'p')
      write_null_pointer(char_type());
    else
      this->write("(null)");
    return this->out();
  }

  /** Formats a null-terminated wide C string. */
  iterator operator()(const wchar_t *value) {
    if (value)
      base::operator()(value);
    else if (this->spec().type_ == 'p')
      write_null_pointer(char_type());
    else
      this->write(L"(null)");
    return this->out();
  }

  /** Formats a pointer. */
  iterator operator()(const void *value) {
    if (value)
      return base::operator()(value);
    this->spec().type_ = 0;
    write_null_pointer(char_type());
    return this->out();
  }

  /** Formats an argument of a custom (user-defined) type. */
  iterator operator()(typename basic_format_arg<context_type>::handle handle) {
    handle.format(context_);
    return this->out();
  }
};

template <typename T>
struct printf_formatter {
  template <typename ParseContext>
  auto parse(ParseContext &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const T &value, FormatContext &ctx) -> decltype(ctx.out()) {
    internal::format_value(internal::get_container(ctx.out()), value);
    return ctx.out();
  }
};

/** This template formats data and writes the output to a writer. */
template <typename OutputIt, typename Char, typename ArgFormatter>
class basic_printf_context :
  private internal::context_base<
    OutputIt, basic_printf_context<OutputIt, Char, ArgFormatter>, Char> {
 public:
  /** The character type for the output. */
  typedef Char char_type;

  template <typename T>
  struct formatter_type { typedef printf_formatter<T> type; };

 private:
  typedef internal::context_base<OutputIt, basic_printf_context, Char> base;
  typedef typename base::format_arg format_arg;
  typedef basic_format_specs<char_type> format_specs;
  typedef internal::null_terminating_iterator<char_type> iterator;

  void parse_flags(format_specs &spec, iterator &it);

  // Returns the argument with specified index or, if arg_index is equal
  // to the maximum unsigned value, the next argument.
  format_arg get_arg(
      iterator it,
      unsigned arg_index = (std::numeric_limits<unsigned>::max)());

  // Parses argument index, flags and width and returns the argument index.
  unsigned parse_header(iterator &it, format_specs &spec);

 public:
  /**
   \rst
   Constructs a ``printf_context`` object. References to the arguments and
   the writer are stored in the context object so make sure they have
   appropriate lifetimes.
   \endrst
   */
  basic_printf_context(OutputIt out, basic_string_view<char_type> format_str,
                       basic_format_args<basic_printf_context> args)
    : base(out, format_str, args) {}

  using base::parse_context;
  using base::out;
  using base::advance_to;

  /** Formats stored arguments and writes the output to the range. */
  void format();
};

template <typename OutputIt, typename Char, typename AF>
void basic_printf_context<OutputIt, Char, AF>::parse_flags(
    format_specs &spec, iterator &it) {
  for (;;) {
    switch (*it++) {
      case '-':
        spec.align_ = ALIGN_LEFT;
        break;
      case '+':
        spec.flags_ |= SIGN_FLAG | PLUS_FLAG;
        break;
      case '0':
        spec.fill_ = '0';
        break;
      case ' ':
        spec.flags_ |= SIGN_FLAG;
        break;
      case '#':
        spec.flags_ |= HASH_FLAG;
        break;
      default:
        --it;
        return;
    }
  }
}

template <typename OutputIt, typename Char, typename AF>
typename basic_printf_context<OutputIt, Char, AF>::format_arg
  basic_printf_context<OutputIt, Char, AF>::get_arg(
    iterator it, unsigned arg_index) {
  (void)it;
  if (arg_index == std::numeric_limits<unsigned>::max())
    return this->do_get_arg(this->parse_context().next_arg_id());
  return base::get_arg(arg_index - 1);
}

template <typename OutputIt, typename Char, typename AF>
unsigned basic_printf_context<OutputIt, Char, AF>::parse_header(
  iterator &it, format_specs &spec) {
  unsigned arg_index = std::numeric_limits<unsigned>::max();
  char_type c = *it;
  if (c >= '0' && c <= '9') {
    // Parse an argument index (if followed by '$') or a width possibly
    // preceded with '0' flag(s).
    internal::error_handler eh;
    unsigned value = parse_nonnegative_int(it, eh);
    if (*it == '$') {  // value is an argument index
      ++it;
      arg_index = value;
    } else {
      if (c == '0')
        spec.fill_ = '0';
      if (value != 0) {
        // Nonzero value means that we parsed width and don't need to
        // parse it or flags again, so return now.
        spec.width_ = value;
        return arg_index;
      }
    }
  }
  parse_flags(spec, it);
  // Parse width.
  if (*it >= '0' && *it <= '9') {
    internal::error_handler eh;
    spec.width_ = parse_nonnegative_int(it, eh);
  } else if (*it == '*') {
    ++it;
    spec.width_ =
        visit(internal::printf_width_handler<char_type>(spec), get_arg(it));
  }
  return arg_index;
}

template <typename OutputIt, typename Char, typename AF>
void basic_printf_context<OutputIt, Char, AF>::format() {
  auto &buffer = internal::get_container(this->out());
  auto start = iterator(this->parse_context());
  auto it = start;
  using internal::pointer_from;
  while (*it) {
    char_type c = *it++;
    if (c != '%') continue;
    if (*it == c) {
      buffer.append(pointer_from(start), pointer_from(it));
      start = ++it;
      continue;
    }
    buffer.append(pointer_from(start), pointer_from(it) - 1);

    format_specs spec;
    spec.align_ = ALIGN_RIGHT;

    // Parse argument index, flags and width.
    unsigned arg_index = parse_header(it, spec);

    // Parse precision.
    if (*it == '.') {
      ++it;
      if ('0' <= *it && *it <= '9') {
        internal::error_handler eh;
        spec.precision_ = static_cast<int>(parse_nonnegative_int(it, eh));
      } else if (*it == '*') {
        ++it;
        spec.precision_ =
            visit(internal::printf_precision_handler(), get_arg(it));
      } else {
        spec.precision_ = 0;
      }
    }

    format_arg arg = get_arg(it, arg_index);
    if (spec.flag(HASH_FLAG) && visit(internal::is_zero_int(), arg))
      spec.flags_ &= ~internal::to_unsigned<int>(HASH_FLAG);
    if (spec.fill_ == '0') {
      if (arg.is_arithmetic())
        spec.align_ = ALIGN_NUMERIC;
      else
        spec.fill_ = ' ';  // Ignore '0' flag for non-numeric types.
    }

    // Parse length and convert the argument to the required type.
    using internal::convert_arg;
    switch (*it++) {
    case 'h':
      if (*it == 'h')
        convert_arg<signed char>(arg, *++it);
      else
        convert_arg<short>(arg, *it);
      break;
    case 'l':
      if (*it == 'l')
        convert_arg<long long>(arg, *++it);
      else
        convert_arg<long>(arg, *it);
      break;
    case 'j':
      convert_arg<intmax_t>(arg, *it);
      break;
    case 'z':
      convert_arg<std::size_t>(arg, *it);
      break;
    case 't':
      convert_arg<std::ptrdiff_t>(arg, *it);
      break;
    case 'L':
      // printf produces garbage when 'L' is omitted for long double, no
      // need to do the same.
      break;
    default:
      --it;
      convert_arg<void>(arg, *it);
    }

    // Parse type.
    if (!*it)
      FMT_THROW(format_error("invalid format string"));
    spec.type_ = static_cast<char>(*it++);
    if (arg.is_integral()) {
      // Normalize type.
      switch (spec.type_) {
      case 'i': case 'u':
        spec.type_ = 'd';
        break;
      case 'c':
        // TODO: handle wchar_t better?
        visit(internal::char_converter<basic_printf_context>(arg), arg);
        break;
      }
    }

    start = it;

    // Format argument.
    visit(AF(buffer, spec, *this), arg);
  }
  buffer.append(pointer_from(start), pointer_from(it));
}

template <typename Char, typename Context>
void printf(internal::basic_buffer<Char> &buf, basic_string_view<Char> format,
            basic_format_args<Context> args) {
  Context(std::back_inserter(buf), format, args).format();
}

template <typename Buffer>
struct printf_context {
  typedef basic_printf_context<
    std::back_insert_iterator<Buffer>, typename Buffer::value_type> type;
};

template <typename ...Args>
inline format_arg_store<printf_context<internal::buffer>::type, Args...>
    make_printf_args(const Args & ... args) {
  return format_arg_store<printf_context<internal::buffer>::type, Args...>(
      args...);
}
typedef basic_format_args<printf_context<internal::buffer>::type> printf_args;
typedef basic_format_args<printf_context<internal::wbuffer>::type> wprintf_args;

inline std::string vsprintf(string_view format, printf_args args) {
  memory_buffer buffer;
  printf(buffer, format, args);
  return to_string(buffer);
}

/**
  \rst
  Formats arguments and returns the result as a string.

  **Example**::

    std::string message = fmt::sprintf("The answer is %d", 42);
  \endrst
*/
template <typename... Args>
inline std::string sprintf(string_view format_str, const Args & ... args) {
  return vsprintf(format_str,
    make_format_args<typename printf_context<internal::buffer>::type>(args...));
}

inline std::wstring vsprintf(wstring_view format, wprintf_args args) {
  wmemory_buffer buffer;
  printf(buffer, format, args);
  return to_string(buffer);
}

template <typename... Args>
inline std::wstring sprintf(wstring_view format_str, const Args & ... args) {
  return vsprintf(format_str,
    make_format_args<typename printf_context<internal::wbuffer>::type>(args...));
}

template <typename Char>
inline int vfprintf(std::FILE *f, basic_string_view<Char> format,
                    basic_format_args<typename printf_context<
                      internal::basic_buffer<Char>>::type> args) {
  basic_memory_buffer<Char> buffer;
  printf(buffer, format, args);
  std::size_t size = buffer.size();
  return std::fwrite(
    buffer.data(), sizeof(Char), size, f) < size ? -1 : static_cast<int>(size);
}

/**
  \rst
  Prints formatted data to the file *f*.

  **Example**::

    fmt::fprintf(stderr, "Don't %s!", "panic");
  \endrst
 */
template <typename... Args>
inline int fprintf(std::FILE *f, string_view format_str, const Args & ... args) {
  auto vargs = make_format_args<
    typename printf_context<internal::buffer>::type>(args...);
  return vfprintf<char>(f, format_str, vargs);
}

template <typename... Args>
inline int fprintf(std::FILE *f, wstring_view format_str,
                   const Args & ... args) {
  return vfprintf(f, format_str,
    make_format_args<typename printf_context<internal::wbuffer>::type>(args...));
}

inline int vprintf(string_view format, printf_args args) {
  return vfprintf(stdout, format, args);
}

inline int vprintf(wstring_view format, wprintf_args args) {
  return vfprintf(stdout, format, args);
}

/**
  \rst
  Prints formatted data to ``stdout``.

  **Example**::

    fmt::printf("Elapsed time: %.2f seconds", 1.23);
  \endrst
 */
template <typename... Args>
inline int printf(string_view format_str, const Args & ... args) {
  return vprintf(format_str,
    make_format_args<typename printf_context<internal::buffer>::type>(args...));
}

template <typename... Args>
inline int printf(wstring_view format_str, const Args & ... args) {
  return vprintf(format_str,
    make_format_args<typename printf_context<internal::wbuffer>::type>(args...));
}

inline int vfprintf(std::ostream &os, string_view format_str,
                    printf_args args) {
  memory_buffer buffer;
  printf(buffer, format_str, args);
  internal::write(os, buffer);
  return static_cast<int>(buffer.size());
}

inline int vfprintf(std::wostream &os, wstring_view format_str,
                    wprintf_args args) {
  wmemory_buffer buffer;
  printf(buffer, format_str, args);
  internal::write(os, buffer);
  return static_cast<int>(buffer.size());
}

/**
  \rst
  Prints formatted data to the stream *os*.

  **Example**::

    fmt::fprintf(cerr, "Don't %s!", "panic");
  \endrst
 */
template <typename... Args>
inline int fprintf(std::ostream &os, string_view format_str,
                   const Args & ... args) {
  auto vargs = make_format_args<
    typename printf_context<internal::buffer>::type>(args...);
  return vfprintf(os, format_str, vargs);
}

template <typename... Args>
inline int fprintf(std::wostream &os, wstring_view format_str,
                   const Args & ... args) {
  auto vargs = make_format_args<
    typename printf_context<internal::buffer>::type>(args...);
  return vfprintf(os, format_str, vargs);
}
FMT_END_NAMESPACE

#endif  // FMT_PRINTF_H_
