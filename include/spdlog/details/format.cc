/*
 Formatting library for C++

 Copyright (c) 2012 - 2015, Victor Zverovich
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "format.h"

#include <string.h>

#include <cctype>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdarg>

#ifdef _WIN32
# ifdef __MINGW32__
#  include <cstring>
# endif
# include <windows.h>
#endif

using fmt::internal::Arg;

// Check if exceptions are disabled.
#if __GNUC__ && !__EXCEPTIONS
# define FMT_EXCEPTIONS 0
#endif
#if _MSC_VER && !_HAS_EXCEPTIONS
# define FMT_EXCEPTIONS 0
#endif
#ifndef FMT_EXCEPTIONS
# define FMT_EXCEPTIONS 1
#endif

#if FMT_EXCEPTIONS
# define FMT_TRY try
# define FMT_CATCH(x) catch (x)
#else
# define FMT_TRY if (true)
# define FMT_CATCH(x) if (false)
#endif

#ifndef FMT_THROW
# if FMT_EXCEPTIONS
#  define FMT_THROW(x) throw x
# else
#  define FMT_THROW(x) assert(false)
# endif
#endif

#ifdef FMT_HEADER_ONLY
# define FMT_FUNC inline
#else
# define FMT_FUNC
#endif

#if _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4127)  // conditional expression is constant
# pragma warning(disable: 4702)  // unreachable code
// Disable deprecation warning for strerror. The latter is not called but
// MSVC fails to detect it.
# pragma warning(disable: 4996)
#endif

// Dummy implementations of strerror_r and strerror_s called if corresponding
// system functions are not available.
static inline fmt::internal::None<> strerror_r(int, char *, ...) {
  return fmt::internal::None<>();
}
static inline fmt::internal::None<> strerror_s(char *, std::size_t, ...) {
  return fmt::internal::None<>();
}

namespace {

#ifndef _MSC_VER
# define FMT_SNPRINTF snprintf
#else  // _MSC_VER
inline int fmt_snprintf(char *buffer, size_t size, const char *format, ...) {
  va_list args;
  va_start(args, format);
  int result = vsnprintf_s(buffer, size, _TRUNCATE, format, args);
  va_end(args);
  return result;
}
# define FMT_SNPRINTF fmt_snprintf
#endif  // _MSC_VER

#if defined(_WIN32) && defined(__MINGW32__) && !defined(__NO_ISOCEXT)
# define FMT_SWPRINTF snwprintf
#else
# define FMT_SWPRINTF swprintf
#endif // defined(_WIN32) && defined(__MINGW32__) && !defined(__NO_ISOCEXT)

// Checks if a value fits in int - used to avoid warnings about comparing
// signed and unsigned integers.
template <bool IsSigned>
struct IntChecker {
  template <typename T>
  static bool fits_in_int(T value) {
    unsigned max = INT_MAX;
    return value <= max;
  }
};

template <>
struct IntChecker<true> {
  template <typename T>
  static bool fits_in_int(T value) {
    return value >= INT_MIN && value <= INT_MAX;
  }
};

const char RESET_COLOR[] = "\x1b[0m";

typedef void (*FormatFunc)(fmt::Writer &, int, fmt::StringRef);

// Portable thread-safe version of strerror.
// Sets buffer to point to a string describing the error code.
// This can be either a pointer to a string stored in buffer,
// or a pointer to some static immutable string.
// Returns one of the following values:
//   0      - success
//   ERANGE - buffer is not large enough to store the error message
//   other  - failure
// Buffer should be at least of size 1.
int safe_strerror(
    int error_code, char *&buffer, std::size_t buffer_size) FMT_NOEXCEPT {
  assert(buffer != 0 && buffer_size != 0);

  class StrError {
   private:
    int error_code_;
    char *&buffer_;
    std::size_t buffer_size_;

    // A noop assignment operator to avoid bogus warnings.
    void operator=(const StrError &) {}

    // Handle the result of XSI-compliant version of strerror_r.
    int handle(int result) {
      // glibc versions before 2.13 return result in errno.
      return result == -1 ? errno : result;
    }

    // Handle the result of GNU-specific version of strerror_r.
    int handle(char *message) {
      // If the buffer is full then the message is probably truncated.
      if (message == buffer_ && strlen(buffer_) == buffer_size_ - 1)
        return ERANGE;
      buffer_ = message;
      return 0;
    }

    // Handle the case when strerror_r is not available.
    int handle(fmt::internal::None<>) {
      return fallback(strerror_s(buffer_, buffer_size_, error_code_));
    }

    // Fallback to strerror_s when strerror_r is not available.
    int fallback(int result) {
      // If the buffer is full then the message is probably truncated.
      return result == 0 && strlen(buffer_) == buffer_size_ - 1 ?
            ERANGE : result;
    }

    // Fallback to strerror if strerror_r and strerror_s are not available.
    int fallback(fmt::internal::None<>) {
      errno = 0;
      buffer_ = strerror(error_code_);
      return errno;
    }

   public:
    StrError(int error_code, char *&buffer, std::size_t buffer_size)
      : error_code_(error_code), buffer_(buffer), buffer_size_(buffer_size) {}

    int run() { return handle(strerror_r(error_code_, buffer_, buffer_size_)); }
  };
  return StrError(error_code, buffer, buffer_size).run();
}

void format_error_code(fmt::Writer &out, int error_code,
                       fmt::StringRef message) FMT_NOEXCEPT {
  // Report error code making sure that the output fits into
  // INLINE_BUFFER_SIZE to avoid dynamic memory allocation and potential
  // bad_alloc.
  out.clear();
  static const char SEP[] = ": ";
  static const char ERROR_STR[] = "error ";
  fmt::internal::IntTraits<int>::MainType ec_value = error_code;
  // Subtract 2 to account for terminating null characters in SEP and ERROR_STR.
  std::size_t error_code_size = sizeof(SEP) + sizeof(ERROR_STR) - 2;
  error_code_size += fmt::internal::count_digits(ec_value);
  if (message.size() <= fmt::internal::INLINE_BUFFER_SIZE - error_code_size)
    out << message << SEP;
  out << ERROR_STR << error_code;
  assert(out.size() <= fmt::internal::INLINE_BUFFER_SIZE);
}

void report_error(FormatFunc func,
    int error_code, fmt::StringRef message) FMT_NOEXCEPT {
  fmt::MemoryWriter full_message;
  func(full_message, error_code, message);
  // Use Writer::data instead of Writer::c_str to avoid potential memory
  // allocation.
  std::fwrite(full_message.data(), full_message.size(), 1, stderr);
  std::fputc('\n', stderr);
}

// IsZeroInt::visit(arg) returns true iff arg is a zero integer.
class IsZeroInt : public fmt::internal::ArgVisitor<IsZeroInt, bool> {
 public:
  template <typename T>
  bool visit_any_int(T value) { return value == 0; }
};

// Parses an unsigned integer advancing s to the end of the parsed input.
// This function assumes that the first character of s is a digit.
template <typename Char>
int parse_nonnegative_int(const Char *&s) {
  assert('0' <= *s && *s <= '9');
  unsigned value = 0;
  do {
    unsigned new_value = value * 10 + (*s++ - '0');
    // Check if value wrapped around.
    if (new_value < value) {
      value = UINT_MAX;
      break;
    }
    value = new_value;
  } while ('0' <= *s && *s <= '9');
  if (value > INT_MAX)
    FMT_THROW(fmt::FormatError("number is too big"));
  return value;
}

inline void require_numeric_argument(const Arg &arg, char spec) {
  if (arg.type > Arg::LAST_NUMERIC_TYPE) {
    std::string message =
        fmt::format("format specifier '{}' requires numeric argument", spec);
    FMT_THROW(fmt::FormatError(message));
  }
}

template <typename Char>
void check_sign(const Char *&s, const Arg &arg) {
  char sign = static_cast<char>(*s);
  require_numeric_argument(arg, sign);
  if (arg.type == Arg::UINT || arg.type == Arg::ULONG_LONG) {
    FMT_THROW(fmt::FormatError(fmt::format(
      "format specifier '{}' requires signed argument", sign)));
  }
  ++s;
}

// Checks if an argument is a valid printf width specifier and sets
// left alignment if it is negative.
class WidthHandler : public fmt::internal::ArgVisitor<WidthHandler, unsigned> {
 private:
  fmt::FormatSpec &spec_;

  FMT_DISALLOW_COPY_AND_ASSIGN(WidthHandler);

 public:
  explicit WidthHandler(fmt::FormatSpec &spec) : spec_(spec) {}

  void report_unhandled_arg() {
    FMT_THROW(fmt::FormatError("width is not integer"));
  }

  template <typename T>
  unsigned visit_any_int(T value) {
    typedef typename fmt::internal::IntTraits<T>::MainType UnsignedType;
    UnsignedType width = value;
    if (fmt::internal::is_negative(value)) {
      spec_.align_ = fmt::ALIGN_LEFT;
      width = 0 - width;
    }
    if (width > INT_MAX)
      FMT_THROW(fmt::FormatError("number is too big"));
    return static_cast<unsigned>(width);
  }
};

class PrecisionHandler :
    public fmt::internal::ArgVisitor<PrecisionHandler, int> {
 public:
  void report_unhandled_arg() {
    FMT_THROW(fmt::FormatError("precision is not integer"));
  }

  template <typename T>
  int visit_any_int(T value) {
    if (!IntChecker<std::numeric_limits<T>::is_signed>::fits_in_int(value))
      FMT_THROW(fmt::FormatError("number is too big"));
    return static_cast<int>(value);
  }
};

// Converts an integer argument to an integral type T for printf.
template <typename T>
class ArgConverter : public fmt::internal::ArgVisitor<ArgConverter<T>, void> {
 private:
  fmt::internal::Arg &arg_;
  wchar_t type_;

  FMT_DISALLOW_COPY_AND_ASSIGN(ArgConverter);

 public:
  ArgConverter(fmt::internal::Arg &arg, wchar_t type)
    : arg_(arg), type_(type) {}

  template <typename U>
  void visit_any_int(U value) {
    bool is_signed = type_ == 'd' || type_ == 'i';
    using fmt::internal::Arg;
    if (sizeof(T) <= sizeof(int)) {
      // Extra casts are used to silence warnings.
      if (is_signed) {
        arg_.type = Arg::INT;
        arg_.int_value = static_cast<int>(static_cast<T>(value));
      } else {
        arg_.type = Arg::UINT;
        arg_.uint_value = static_cast<unsigned>(
            static_cast<typename fmt::internal::MakeUnsigned<T>::Type>(value));
      }
    } else {
      if (is_signed) {
        arg_.type = Arg::LONG_LONG;
        arg_.long_long_value =
            static_cast<typename fmt::internal::MakeUnsigned<U>::Type>(value);
      } else {
        arg_.type = Arg::ULONG_LONG;
        arg_.ulong_long_value =
            static_cast<typename fmt::internal::MakeUnsigned<U>::Type>(value);
      }
    }
  }
};

// Converts an integer argument to char for printf.
class CharConverter : public fmt::internal::ArgVisitor<CharConverter, void> {
 private:
  fmt::internal::Arg &arg_;

  FMT_DISALLOW_COPY_AND_ASSIGN(CharConverter);

 public:
  explicit CharConverter(fmt::internal::Arg &arg) : arg_(arg) {}

  template <typename T>
  void visit_any_int(T value) {
    arg_.type = Arg::CHAR;
    arg_.int_value = static_cast<char>(value);
  }
};

// This function template is used to prevent compile errors when handling
// incompatible string arguments, e.g. handling a wide string in a narrow
// string formatter.
template <typename Char>
Arg::StringValue<Char> ignore_incompatible_str(Arg::StringValue<wchar_t>);

template <>
inline Arg::StringValue<char> ignore_incompatible_str(
    Arg::StringValue<wchar_t>) { return Arg::StringValue<char>(); }

template <>
inline Arg::StringValue<wchar_t> ignore_incompatible_str(
    Arg::StringValue<wchar_t> s) { return s; }
}  // namespace

FMT_FUNC void fmt::SystemError::init(
    int err_code, StringRef format_str, ArgList args) {
  error_code_ = err_code;
  MemoryWriter w;
  internal::format_system_error(w, err_code, format(format_str, args));
  std::runtime_error &base = *this;
  base = std::runtime_error(w.str());
}

template <typename T>
int fmt::internal::CharTraits<char>::format_float(
    char *buffer, std::size_t size, const char *format,
    unsigned width, int precision, T value) {
  if (width == 0) {
    return precision < 0 ?
        FMT_SNPRINTF(buffer, size, format, value) :
        FMT_SNPRINTF(buffer, size, format, precision, value);
  }
  return precision < 0 ?
      FMT_SNPRINTF(buffer, size, format, width, value) :
      FMT_SNPRINTF(buffer, size, format, width, precision, value);
}

template <typename T>
int fmt::internal::CharTraits<wchar_t>::format_float(
    wchar_t *buffer, std::size_t size, const wchar_t *format,
    unsigned width, int precision, T value) {
  if (width == 0) {
    return precision < 0 ?
        FMT_SWPRINTF(buffer, size, format, value) :
        FMT_SWPRINTF(buffer, size, format, precision, value);
  }
  return precision < 0 ?
      FMT_SWPRINTF(buffer, size, format, width, value) :
      FMT_SWPRINTF(buffer, size, format, width, precision, value);
}

template <typename T>
const char fmt::internal::BasicData<T>::DIGITS[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

#define FMT_POWERS_OF_10(factor) \
  factor * 10, \
  factor * 100, \
  factor * 1000, \
  factor * 10000, \
  factor * 100000, \
  factor * 1000000, \
  factor * 10000000, \
  factor * 100000000, \
  factor * 1000000000

template <typename T>
const uint32_t fmt::internal::BasicData<T>::POWERS_OF_10_32[] = {
  0, FMT_POWERS_OF_10(1)
};

template <typename T>
const uint64_t fmt::internal::BasicData<T>::POWERS_OF_10_64[] = {
  0,
  FMT_POWERS_OF_10(1),
  FMT_POWERS_OF_10(fmt::ULongLong(1000000000)),
  // Multiply several constants instead of using a single long long constant
  // to avoid warnings about C++98 not supporting long long.
  fmt::ULongLong(1000000000) * fmt::ULongLong(1000000000) * 10
};

FMT_FUNC void fmt::internal::report_unknown_type(char code, const char *type) {
  (void)type;
  if (std::isprint(static_cast<unsigned char>(code))) {
    FMT_THROW(fmt::FormatError(
        fmt::format("unknown format code '{}' for {}", code, type)));
  }
  FMT_THROW(fmt::FormatError(
      fmt::format("unknown format code '\\x{:02x}' for {}",
        static_cast<unsigned>(code), type)));
}

#ifdef _WIN32

FMT_FUNC fmt::internal::UTF8ToUTF16::UTF8ToUTF16(fmt::StringRef s) {
  int length = MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), -1, 0, 0);
  static const char ERROR_MSG[] = "cannot convert string from UTF-8 to UTF-16";
  if (length == 0)
    FMT_THROW(WindowsError(GetLastError(), ERROR_MSG));
  buffer_.resize(length);
  length = MultiByteToWideChar(
    CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), -1, &buffer_[0], length);
  if (length == 0)
    FMT_THROW(WindowsError(GetLastError(), ERROR_MSG));
}

FMT_FUNC fmt::internal::UTF16ToUTF8::UTF16ToUTF8(fmt::WStringRef s) {
  if (int error_code = convert(s)) {
    FMT_THROW(WindowsError(error_code,
        "cannot convert string from UTF-16 to UTF-8"));
  }
}

FMT_FUNC int fmt::internal::UTF16ToUTF8::convert(fmt::WStringRef s) {
  int length = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, 0, 0, 0, 0);
  if (length == 0)
    return GetLastError();
  buffer_.resize(length);
  length = WideCharToMultiByte(
    CP_UTF8, 0, s.c_str(), -1, &buffer_[0], length, 0, 0);
  if (length == 0)
    return GetLastError();
  return 0;
}

FMT_FUNC void fmt::WindowsError::init(
    int err_code, StringRef format_str, ArgList args) {
  error_code_ = err_code;
  MemoryWriter w;
  internal::format_windows_error(w, err_code, format(format_str, args));
  std::runtime_error &base = *this;
  base = std::runtime_error(w.str());
}

#endif

FMT_FUNC void fmt::internal::format_system_error(
    fmt::Writer &out, int error_code,
    fmt::StringRef message) FMT_NOEXCEPT {
  FMT_TRY {
    MemoryBuffer<char, INLINE_BUFFER_SIZE> buffer;
    buffer.resize(INLINE_BUFFER_SIZE);
    for (;;) {
      char *system_message = &buffer[0];
      int result = safe_strerror(error_code, system_message, buffer.size());
      if (result == 0) {
        out << message << ": " << system_message;
        return;
      }
      if (result != ERANGE)
        break;  // Can't get error message, report error code instead.
      buffer.resize(buffer.size() * 2);
    }
  } FMT_CATCH(...) {}
  format_error_code(out, error_code, message);
}

#ifdef _WIN32
FMT_FUNC void fmt::internal::format_windows_error(
    fmt::Writer &out, int error_code,
    fmt::StringRef message) FMT_NOEXCEPT {
  class String {
   private:
    LPWSTR str_;

   public:
    String() : str_() {}
    ~String() { LocalFree(str_); }
    LPWSTR *ptr() { return &str_; }
    LPCWSTR c_str() const { return str_; }
  };
  FMT_TRY {
    String system_message;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0,
        error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(system_message.ptr()), 0, 0)) {
      UTF16ToUTF8 utf8_message;
      if (utf8_message.convert(system_message.c_str()) == ERROR_SUCCESS) {
        out << message << ": " << utf8_message;
        return;
      }
    }
  } FMT_CATCH(...) {}
  format_error_code(out, error_code, message);
}
#endif

// An argument formatter.
template <typename Char>
class fmt::internal::ArgFormatter :
    public fmt::internal::ArgVisitor<fmt::internal::ArgFormatter<Char>, void> {
 private:
  fmt::BasicFormatter<Char> &formatter_;
  fmt::BasicWriter<Char> &writer_;
  fmt::FormatSpec &spec_;
  const Char *format_;

  FMT_DISALLOW_COPY_AND_ASSIGN(ArgFormatter);

 public:
  ArgFormatter(
      fmt::BasicFormatter<Char> &f,fmt::FormatSpec &s, const Char *fmt)
  : formatter_(f), writer_(f.writer()), spec_(s), format_(fmt) {}

  template <typename T>
  void visit_any_int(T value) { writer_.write_int(value, spec_); }

  template <typename T>
  void visit_any_double(T value) { writer_.write_double(value, spec_); }

  void visit_char(int value) {
    if (spec_.type_ && spec_.type_ != 'c') {
      spec_.flags_ |= CHAR_FLAG;
      writer_.write_int(value, spec_);
      return;
    }
    if (spec_.align_ == ALIGN_NUMERIC || spec_.flags_ != 0)
      FMT_THROW(FormatError("invalid format specifier for char"));
    typedef typename fmt::BasicWriter<Char>::CharPtr CharPtr;
    Char fill = static_cast<Char>(spec_.fill());
    if (spec_.precision_ == 0) {
      std::fill_n(writer_.grow_buffer(spec_.width_), spec_.width_, fill);
      return;
    }
    CharPtr out = CharPtr();
    if (spec_.width_ > 1) {
      out = writer_.grow_buffer(spec_.width_);
      if (spec_.align_ == fmt::ALIGN_RIGHT) {
        std::fill_n(out, spec_.width_ - 1, fill);
        out += spec_.width_ - 1;
      } else if (spec_.align_ == fmt::ALIGN_CENTER) {
        out = writer_.fill_padding(out, spec_.width_, 1, fill);
      } else {
        std::fill_n(out + 1, spec_.width_ - 1, fill);
      }
    } else {
      out = writer_.grow_buffer(1);
    }
    *out = static_cast<Char>(value);
  }

  void visit_string(Arg::StringValue<char> value) {
    writer_.write_str(value, spec_);
  }
  void visit_wstring(Arg::StringValue<wchar_t> value) {
    writer_.write_str(ignore_incompatible_str<Char>(value), spec_);
  }

  void visit_pointer(const void *value) {
    if (spec_.type_ && spec_.type_ != 'p')
      fmt::internal::report_unknown_type(spec_.type_, "pointer");
    spec_.flags_ = fmt::HASH_FLAG;
    spec_.type_ = 'x';
    writer_.write_int(reinterpret_cast<uintptr_t>(value), spec_);
  }

  void visit_custom(Arg::CustomValue c) {
    c.format(&formatter_, c.value, &format_);
  }
};

template <typename Char>
void fmt::internal::FixedBuffer<Char>::grow(std::size_t) {
  FMT_THROW(std::runtime_error("buffer overflow"));
}

template <typename Char>
template <typename StrChar>
void fmt::BasicWriter<Char>::write_str(
    const Arg::StringValue<StrChar> &s, const FormatSpec &spec) {
  // Check if StrChar is convertible to Char.
  internal::CharTraits<Char>::convert(StrChar());
  if (spec.type_ && spec.type_ != 's')
    internal::report_unknown_type(spec.type_, "string");
  const StrChar *str_value = s.value;
  std::size_t str_size = s.size;
  if (str_size == 0) {
    if (!str_value)
      FMT_THROW(FormatError("string pointer is null"));
    if (*str_value)
      str_size = std::char_traits<StrChar>::length(str_value);
  }
  std::size_t precision = spec.precision_;
  if (spec.precision_ >= 0 && precision < str_size)
    str_size = spec.precision_;
  write_str(str_value, str_size, spec);
}

template <typename Char>
inline Arg fmt::BasicFormatter<Char>::parse_arg_index(const Char *&s) {
  const char *error = 0;
  Arg arg = *s < '0' || *s > '9' ?
        next_arg(error) : get_arg(parse_nonnegative_int(s), error);
  if (error) {
    FMT_THROW(FormatError(
                *s != '}' && *s != ':' ? "invalid format string" : error));
  }
  return arg;
}

FMT_FUNC Arg fmt::internal::FormatterBase::do_get_arg(
    unsigned arg_index, const char *&error) {
  Arg arg = args_[arg_index];
  if (arg.type == Arg::NONE)
    error = "argument index out of range";
  return arg;
}

inline Arg fmt::internal::FormatterBase::next_arg(const char *&error) {
  if (next_arg_index_ >= 0)
    return do_get_arg(next_arg_index_++, error);
  error = "cannot switch from manual to automatic argument indexing";
  return Arg();
}

inline Arg fmt::internal::FormatterBase::get_arg(
    unsigned arg_index, const char *&error) {
  if (next_arg_index_ <= 0) {
    next_arg_index_ = -1;
    return do_get_arg(arg_index, error);
  }
  error = "cannot switch from automatic to manual argument indexing";
  return Arg();
}

template <typename Char>
void fmt::internal::PrintfFormatter<Char>::parse_flags(
    FormatSpec &spec, const Char *&s) {
  for (;;) {
    switch (*s++) {
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
        --s;
        return;
    }
  }
}

template <typename Char>
Arg fmt::internal::PrintfFormatter<Char>::get_arg(
    const Char *s, unsigned arg_index) {
  (void)s;
  const char *error = 0;
  Arg arg = arg_index == UINT_MAX ?
    next_arg(error) : FormatterBase::get_arg(arg_index - 1, error);
  if (error)
    FMT_THROW(FormatError(!*s ? "invalid format string" : error));
  return arg;
}

template <typename Char>
unsigned fmt::internal::PrintfFormatter<Char>::parse_header(
  const Char *&s, FormatSpec &spec) {
  unsigned arg_index = UINT_MAX;
  Char c = *s;
  if (c >= '0' && c <= '9') {
    // Parse an argument index (if followed by '$') or a width possibly
    // preceded with '0' flag(s).
    unsigned value = parse_nonnegative_int(s);
    if (*s == '$') {  // value is an argument index
      ++s;
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
  parse_flags(spec, s);
  // Parse width.
  if (*s >= '0' && *s <= '9') {
    spec.width_ = parse_nonnegative_int(s);
  } else if (*s == '*') {
    ++s;
    spec.width_ = WidthHandler(spec).visit(get_arg(s));
  }
  return arg_index;
}

template <typename Char>
void fmt::internal::PrintfFormatter<Char>::format(
    BasicWriter<Char> &writer, BasicStringRef<Char> format_str,
    const ArgList &args) {
  const Char *start = format_str.c_str();
  set_args(args);
  const Char *s = start;
  while (*s) {
    Char c = *s++;
    if (c != '%') continue;
    if (*s == c) {
      write(writer, start, s);
      start = ++s;
      continue;
    }
    write(writer, start, s - 1);

    FormatSpec spec;
    spec.align_ = ALIGN_RIGHT;

    // Parse argument index, flags and width.
    unsigned arg_index = parse_header(s, spec);

    // Parse precision.
    if (*s == '.') {
      ++s;
      if ('0' <= *s && *s <= '9') {
        spec.precision_ = parse_nonnegative_int(s);
      } else if (*s == '*') {
        ++s;
        spec.precision_ = PrecisionHandler().visit(get_arg(s));
      }
    }

    Arg arg = get_arg(s, arg_index);
    if (spec.flag(HASH_FLAG) && IsZeroInt().visit(arg))
      spec.flags_ &= ~HASH_FLAG;
    if (spec.fill_ == '0') {
      if (arg.type <= Arg::LAST_NUMERIC_TYPE)
        spec.align_ = ALIGN_NUMERIC;
      else
        spec.fill_ = ' ';  // Ignore '0' flag for non-numeric types.
    }

    // Parse length and convert the argument to the required type.
    switch (*s++) {
    case 'h':
      if (*s == 'h')
        ArgConverter<signed char>(arg, *++s).visit(arg);
      else
        ArgConverter<short>(arg, *s).visit(arg);
      break;
    case 'l':
      if (*s == 'l')
        ArgConverter<fmt::LongLong>(arg, *++s).visit(arg);
      else
        ArgConverter<long>(arg, *s).visit(arg);
      break;
    case 'j':
      ArgConverter<intmax_t>(arg, *s).visit(arg);
      break;
    case 'z':
      ArgConverter<size_t>(arg, *s).visit(arg);
      break;
    case 't':
      ArgConverter<ptrdiff_t>(arg, *s).visit(arg);
      break;
    case 'L':
      // printf produces garbage when 'L' is omitted for long double, no
      // need to do the same.
      break;
    default:
      --s;
      ArgConverter<int>(arg, *s).visit(arg);
    }

    // Parse type.
    if (!*s)
      FMT_THROW(FormatError("invalid format string"));
    spec.type_ = static_cast<char>(*s++);
    if (arg.type <= Arg::LAST_INTEGER_TYPE) {
      // Normalize type.
      switch (spec.type_) {
      case 'i': case 'u':
        spec.type_ = 'd';
        break;
      case 'c':
        // TODO: handle wchar_t
        CharConverter(arg).visit(arg);
        break;
      }
    }

    start = s;

    // Format argument.
    switch (arg.type) {
    case Arg::INT:
      writer.write_int(arg.int_value, spec);
      break;
    case Arg::UINT:
      writer.write_int(arg.uint_value, spec);
      break;
    case Arg::LONG_LONG:
      writer.write_int(arg.long_long_value, spec);
      break;
    case Arg::ULONG_LONG:
      writer.write_int(arg.ulong_long_value, spec);
      break;
    case Arg::CHAR: {
      if (spec.type_ && spec.type_ != 'c')
        writer.write_int(arg.int_value, spec);
      typedef typename BasicWriter<Char>::CharPtr CharPtr;
      CharPtr out = CharPtr();
      if (spec.width_ > 1) {
        Char fill = ' ';
        out = writer.grow_buffer(spec.width_);
        if (spec.align_ != ALIGN_LEFT) {
          std::fill_n(out, spec.width_ - 1, fill);
          out += spec.width_ - 1;
        } else {
          std::fill_n(out + 1, spec.width_ - 1, fill);
        }
      } else {
        out = writer.grow_buffer(1);
      }
      *out = static_cast<Char>(arg.int_value);
      break;
    }
    case Arg::DOUBLE:
      writer.write_double(arg.double_value, spec);
      break;
    case Arg::LONG_DOUBLE:
      writer.write_double(arg.long_double_value, spec);
      break;
    case Arg::CSTRING:
      arg.string.size = 0;
      writer.write_str(arg.string, spec);
      break;
    case Arg::STRING:
      writer.write_str(arg.string, spec);
      break;
    case Arg::WSTRING:
      writer.write_str(ignore_incompatible_str<Char>(arg.wstring), spec);
      break;
    case Arg::POINTER:
      if (spec.type_ && spec.type_ != 'p')
        internal::report_unknown_type(spec.type_, "pointer");
      spec.flags_= HASH_FLAG;
      spec.type_ = 'x';
      writer.write_int(reinterpret_cast<uintptr_t>(arg.pointer), spec);
      break;
    case Arg::CUSTOM: {
      if (spec.type_)
        internal::report_unknown_type(spec.type_, "object");
      const void *str_format = "s";
      arg.custom.format(&writer, arg.custom.value, &str_format);
      break;
    }
    default:
      assert(false);
      break;
    }
  }
  write(writer, start, s);
}

template <typename Char>
const Char *fmt::BasicFormatter<Char>::format(
    const Char *&format_str, const Arg &arg) {
  const Char *s = format_str;
  FormatSpec spec;
  if (*s == ':') {
    if (arg.type == Arg::CUSTOM) {
      arg.custom.format(this, arg.custom.value, &s);
      return s;
    }
    ++s;
    // Parse fill and alignment.
    if (Char c = *s) {
      const Char *p = s + 1;
      spec.align_ = ALIGN_DEFAULT;
      do {
        switch (*p) {
          case '<':
            spec.align_ = ALIGN_LEFT;
            break;
          case '>':
            spec.align_ = ALIGN_RIGHT;
            break;
          case '=':
            spec.align_ = ALIGN_NUMERIC;
            break;
          case '^':
            spec.align_ = ALIGN_CENTER;
            break;
        }
        if (spec.align_ != ALIGN_DEFAULT) {
          if (p != s) {
            if (c == '}') break;
            if (c == '{')
              FMT_THROW(FormatError("invalid fill character '{'"));
            s += 2;
            spec.fill_ = c;
          } else ++s;
          if (spec.align_ == ALIGN_NUMERIC)
            require_numeric_argument(arg, '=');
          break;
        }
      } while (--p >= s);
    }

    // Parse sign.
    switch (*s) {
      case '+':
        check_sign(s, arg);
        spec.flags_ |= SIGN_FLAG | PLUS_FLAG;
        break;
      case '-':
        check_sign(s, arg);
        spec.flags_ |= MINUS_FLAG;
        break;
      case ' ':
        check_sign(s, arg);
        spec.flags_ |= SIGN_FLAG;
        break;
    }

    if (*s == '#') {
      require_numeric_argument(arg, '#');
      spec.flags_ |= HASH_FLAG;
      ++s;
    }

    // Parse width and zero flag.
    if ('0' <= *s && *s <= '9') {
      if (*s == '0') {
        require_numeric_argument(arg, '0');
        spec.align_ = ALIGN_NUMERIC;
        spec.fill_ = '0';
      }
      // Zero may be parsed again as a part of the width, but it is simpler
      // and more efficient than checking if the next char is a digit.
      spec.width_ = parse_nonnegative_int(s);
    }

    // Parse precision.
    if (*s == '.') {
      ++s;
      spec.precision_ = 0;
      if ('0' <= *s && *s <= '9') {
        spec.precision_ = parse_nonnegative_int(s);
      } else if (*s == '{') {
        ++s;
        const Arg &precision_arg = parse_arg_index(s);
        if (*s++ != '}')
          FMT_THROW(FormatError("invalid format string"));
        ULongLong value = 0;
        switch (precision_arg.type) {
          case Arg::INT:
            if (precision_arg.int_value < 0)
              FMT_THROW(FormatError("negative precision"));
            value = precision_arg.int_value;
            break;
          case Arg::UINT:
            value = precision_arg.uint_value;
            break;
          case Arg::LONG_LONG:
            if (precision_arg.long_long_value < 0)
              FMT_THROW(FormatError("negative precision"));
            value = precision_arg.long_long_value;
            break;
          case Arg::ULONG_LONG:
            value = precision_arg.ulong_long_value;
            break;
          default:
            FMT_THROW(FormatError("precision is not integer"));
        }
        if (value > INT_MAX)
          FMT_THROW(FormatError("number is too big"));
        spec.precision_ = static_cast<int>(value);
      } else {
        FMT_THROW(FormatError("missing precision specifier"));
      }
      if (arg.type < Arg::LAST_INTEGER_TYPE || arg.type == Arg::POINTER) {
        FMT_THROW(FormatError(
            fmt::format("precision not allowed in {} format specifier",
            arg.type == Arg::POINTER ? "pointer" : "integer")));
      }
    }

    // Parse type.
    if (*s != '}' && *s)
      spec.type_ = static_cast<char>(*s++);
  }

  if (*s++ != '}')
    FMT_THROW(FormatError("missing '}' in format string"));
  start_ = s;

  // Format argument.
  internal::ArgFormatter<Char>(*this, spec, s - 1).visit(arg);
  return s;
}

template <typename Char>
void fmt::BasicFormatter<Char>::format(
    BasicStringRef<Char> format_str, const ArgList &args) {
  const Char *s = start_ = format_str.c_str();
  set_args(args);
  while (*s) {
    Char c = *s++;
    if (c != '{' && c != '}') continue;
    if (*s == c) {
      write(writer_, start_, s);
      start_ = ++s;
      continue;
    }
    if (c == '}')
      FMT_THROW(FormatError("unmatched '}' in format string"));
    write(writer_, start_, s - 1);
    Arg arg = parse_arg_index(s);
    s = format(s, arg);
  }
  write(writer_, start_, s);
}

FMT_FUNC void fmt::report_system_error(
    int error_code, fmt::StringRef message) FMT_NOEXCEPT {
  report_error(internal::format_system_error, error_code, message);
}

#ifdef _WIN32
FMT_FUNC void fmt::report_windows_error(
    int error_code, fmt::StringRef message) FMT_NOEXCEPT {
  report_error(internal::format_windows_error, error_code, message);
}
#endif

FMT_FUNC void fmt::print(std::FILE *f, StringRef format_str, ArgList args) {
  MemoryWriter w;
  w.write(format_str, args);
  std::fwrite(w.data(), 1, w.size(), f);
}

FMT_FUNC void fmt::print(StringRef format_str, ArgList args) {
  print(stdout, format_str, args);
}

FMT_FUNC void fmt::print(std::ostream &os, StringRef format_str, ArgList args) {
  MemoryWriter w;
  w.write(format_str, args);
  os.write(w.data(), w.size());
}

FMT_FUNC void fmt::print_colored(Color c, StringRef format, ArgList args) {
  char escape[] = "\x1b[30m";
  escape[3] = '0' + static_cast<char>(c);
  std::fputs(escape, stdout);
  print(format, args);
  std::fputs(RESET_COLOR, stdout);
}

FMT_FUNC int fmt::fprintf(std::FILE *f, StringRef format, ArgList args) {
  MemoryWriter w;
  printf(w, format, args);
  std::size_t size = w.size();
  return std::fwrite(w.data(), 1, size, f) < size ? -1 : static_cast<int>(size);
}

#ifndef FMT_HEADER_ONLY

// Explicit instantiations for char.

template void fmt::internal::FixedBuffer<char>::grow(std::size_t);

template const char *fmt::BasicFormatter<char>::format(
    const char *&format_str, const fmt::internal::Arg &arg);

template void fmt::BasicFormatter<char>::format(
  BasicStringRef<char> format, const ArgList &args);

template void fmt::internal::PrintfFormatter<char>::format(
  BasicWriter<char> &writer, BasicStringRef<char> format, const ArgList &args);

template int fmt::internal::CharTraits<char>::format_float(
    char *buffer, std::size_t size, const char *format,
    unsigned width, int precision, double value);

template int fmt::internal::CharTraits<char>::format_float(
    char *buffer, std::size_t size, const char *format,
    unsigned width, int precision, long double value);

// Explicit instantiations for wchar_t.

template void fmt::internal::FixedBuffer<wchar_t>::grow(std::size_t);

template const wchar_t *fmt::BasicFormatter<wchar_t>::format(
    const wchar_t *&format_str, const fmt::internal::Arg &arg);

template void fmt::BasicFormatter<wchar_t>::format(
    BasicStringRef<wchar_t> format, const ArgList &args);

template void fmt::internal::PrintfFormatter<wchar_t>::format(
    BasicWriter<wchar_t> &writer, BasicStringRef<wchar_t> format,
    const ArgList &args);

template int fmt::internal::CharTraits<wchar_t>::format_float(
    wchar_t *buffer, std::size_t size, const wchar_t *format,
    unsigned width, int precision, double value);

template int fmt::internal::CharTraits<wchar_t>::format_float(
    wchar_t *buffer, std::size_t size, const wchar_t *format,
    unsigned width, int precision, long double value);

#endif  // FMT_HEADER_ONLY

#if _MSC_VER
# pragma warning(pop)
#endif