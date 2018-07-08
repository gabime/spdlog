// Formatting library for C++
//
// Copyright (c) 2012 - 2016, Victor Zverovich
// All rights reserved.
//
// For the license information refer to format.h.

#ifndef FMT_FORMAT_INL_H_
#define FMT_FORMAT_INL_H_

#include "format.h"

#include <string.h>

#include <cctype>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstddef>  // for std::ptrdiff_t
#include <locale>

#if defined(_WIN32) && defined(__MINGW32__)
# include <cstring>
#endif

#if FMT_USE_WINDOWS_H
# if !defined(FMT_HEADER_ONLY) && !defined(WIN32_LEAN_AND_MEAN)
#  define WIN32_LEAN_AND_MEAN
# endif
# if defined(NOMINMAX) || defined(FMT_WIN_MINMAX)
#  include <windows.h>
# else
#  define NOMINMAX
#  include <windows.h>
#  undef NOMINMAX
# endif
#endif

#if FMT_EXCEPTIONS
# define FMT_TRY try
# define FMT_CATCH(x) catch (x)
#else
# define FMT_TRY if (true)
# define FMT_CATCH(x) if (false)
#endif

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4127)  // conditional expression is constant
# pragma warning(disable: 4702)  // unreachable code
// Disable deprecation warning for strerror. The latter is not called but
// MSVC fails to detect it.
# pragma warning(disable: 4996)
#endif

// Dummy implementations of strerror_r and strerror_s called if corresponding
// system functions are not available.
inline fmt::internal::null<> strerror_r(int, char *, ...) {
  return fmt::internal::null<>();
}
inline fmt::internal::null<> strerror_s(char *, std::size_t, ...) {
  return fmt::internal::null<>();
}

FMT_BEGIN_NAMESPACE

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

typedef void (*FormatFunc)(internal::buffer &, int, string_view);

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
  FMT_ASSERT(buffer != FMT_NULL && buffer_size != 0, "invalid buffer");

  class dispatcher {
   private:
    int error_code_;
    char *&buffer_;
    std::size_t buffer_size_;

    // A noop assignment operator to avoid bogus warnings.
    void operator=(const dispatcher &) {}

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
    int handle(internal::null<>) {
      return fallback(strerror_s(buffer_, buffer_size_, error_code_));
    }

    // Fallback to strerror_s when strerror_r is not available.
    int fallback(int result) {
      // If the buffer is full then the message is probably truncated.
      return result == 0 && strlen(buffer_) == buffer_size_ - 1 ?
            ERANGE : result;
    }

    // Fallback to strerror if strerror_r and strerror_s are not available.
    int fallback(internal::null<>) {
      errno = 0;
      buffer_ = strerror(error_code_);
      return errno;
    }

   public:
    dispatcher(int err_code, char *&buf, std::size_t buf_size)
      : error_code_(err_code), buffer_(buf), buffer_size_(buf_size) {}

    int run() {
      return handle(strerror_r(error_code_, buffer_, buffer_size_));
    }
  };
  return dispatcher(error_code, buffer, buffer_size).run();
}

void format_error_code(internal::buffer &out, int error_code,
                       string_view message) FMT_NOEXCEPT {
  // Report error code making sure that the output fits into
  // inline_buffer_size to avoid dynamic memory allocation and potential
  // bad_alloc.
  out.resize(0);
  static const char SEP[] = ": ";
  static const char ERROR_STR[] = "error ";
  // Subtract 2 to account for terminating null characters in SEP and ERROR_STR.
  std::size_t error_code_size = sizeof(SEP) + sizeof(ERROR_STR) - 2;
  typedef internal::int_traits<int>::main_type main_type;
  main_type abs_value = static_cast<main_type>(error_code);
  if (internal::is_negative(error_code)) {
    abs_value = 0 - abs_value;
    ++error_code_size;
  }
  error_code_size += internal::count_digits(abs_value);
  writer w(out);
  if (message.size() <= inline_buffer_size - error_code_size) {
    w.write(message);
    w.write(SEP);
  }
  w.write(ERROR_STR);
  w.write(error_code);
  assert(out.size() <= inline_buffer_size);
}

void report_error(FormatFunc func, int error_code,
                  string_view message) FMT_NOEXCEPT {
  memory_buffer full_message;
  func(full_message, error_code, message);
  // Use Writer::data instead of Writer::c_str to avoid potential memory
  // allocation.
  std::fwrite(full_message.data(), full_message.size(), 1, stderr);
  std::fputc('\n', stderr);
}
}  // namespace

class locale {
 private:
  std::locale locale_;

 public:
  explicit locale(std::locale loc = std::locale()) : locale_(loc) {}
  std::locale get() { return locale_; }
};

template <typename Char>
FMT_FUNC Char internal::thousands_sep(locale_provider *lp) {
  std::locale loc = lp ? lp->locale().get() : std::locale();
  return std::use_facet<std::numpunct<Char>>(loc).thousands_sep();
}

FMT_FUNC void system_error::init(
    int err_code, string_view format_str, format_args args) {
  error_code_ = err_code;
  memory_buffer buffer;
  format_system_error(buffer, err_code, vformat(format_str, args));
  std::runtime_error &base = *this;
  base = std::runtime_error(to_string(buffer));
}

namespace internal {
template <typename T>
int char_traits<char>::format_float(
    char *buffer, std::size_t size, const char *format, int precision, T value) {
  return precision < 0 ?
      FMT_SNPRINTF(buffer, size, format, value) :
      FMT_SNPRINTF(buffer, size, format, precision, value);
}

template <typename T>
int char_traits<wchar_t>::format_float(
    wchar_t *buffer, std::size_t size, const wchar_t *format, int precision,
    T value) {
  return precision < 0 ?
      FMT_SWPRINTF(buffer, size, format, value) :
      FMT_SWPRINTF(buffer, size, format, precision, value);
}

template <typename T>
const char basic_data<T>::DIGITS[] =
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
const uint32_t basic_data<T>::POWERS_OF_10_32[] = {
  0, FMT_POWERS_OF_10(1)
};

template <typename T>
const uint64_t basic_data<T>::POWERS_OF_10_64[] = {
  0,
  FMT_POWERS_OF_10(1),
  FMT_POWERS_OF_10(1000000000ull),
  10000000000000000000ull
};

// Normalized 64-bit significands of pow(10, k), for k = -348, -340, ..., 340.
// These are generated by support/compute-powers.py.
template <typename T>
const uint64_t basic_data<T>::POW10_SIGNIFICANDS[] = {
  0xfa8fd5a0081c0288, 0xbaaee17fa23ebf76, 0x8b16fb203055ac76, 0xcf42894a5dce35ea,
  0x9a6bb0aa55653b2d, 0xe61acf033d1a45df, 0xab70fe17c79ac6ca, 0xff77b1fcbebcdc4f,
  0xbe5691ef416bd60c, 0x8dd01fad907ffc3c, 0xd3515c2831559a83, 0x9d71ac8fada6c9b5,
  0xea9c227723ee8bcb, 0xaecc49914078536d, 0x823c12795db6ce57, 0xc21094364dfb5637,
  0x9096ea6f3848984f, 0xd77485cb25823ac7, 0xa086cfcd97bf97f4, 0xef340a98172aace5,
  0xb23867fb2a35b28e, 0x84c8d4dfd2c63f3b, 0xc5dd44271ad3cdba, 0x936b9fcebb25c996,
  0xdbac6c247d62a584, 0xa3ab66580d5fdaf6, 0xf3e2f893dec3f126, 0xb5b5ada8aaff80b8,
  0x87625f056c7c4a8b, 0xc9bcff6034c13053, 0x964e858c91ba2655, 0xdff9772470297ebd,
  0xa6dfbd9fb8e5b88f, 0xf8a95fcf88747d94, 0xb94470938fa89bcf, 0x8a08f0f8bf0f156b,
  0xcdb02555653131b6, 0x993fe2c6d07b7fac, 0xe45c10c42a2b3b06, 0xaa242499697392d3,
  0xfd87b5f28300ca0e, 0xbce5086492111aeb, 0x8cbccc096f5088cc, 0xd1b71758e219652c,
  0x9c40000000000000, 0xe8d4a51000000000, 0xad78ebc5ac620000, 0x813f3978f8940984,
  0xc097ce7bc90715b3, 0x8f7e32ce7bea5c70, 0xd5d238a4abe98068, 0x9f4f2726179a2245,
  0xed63a231d4c4fb27, 0xb0de65388cc8ada8, 0x83c7088e1aab65db, 0xc45d1df942711d9a,
  0x924d692ca61be758, 0xda01ee641a708dea, 0xa26da3999aef774a, 0xf209787bb47d6b85,
  0xb454e4a179dd1877, 0x865b86925b9bc5c2, 0xc83553c5c8965d3d, 0x952ab45cfa97a0b3,
  0xde469fbd99a05fe3, 0xa59bc234db398c25, 0xf6c69a72a3989f5c, 0xb7dcbf5354e9bece,
  0x88fcf317f22241e2, 0xcc20ce9bd35c78a5, 0x98165af37b2153df, 0xe2a0b5dc971f303a,
  0xa8d9d1535ce3b396, 0xfb9b7cd9a4a7443c, 0xbb764c4ca7a44410, 0x8bab8eefb6409c1a,
  0xd01fef10a657842c, 0x9b10a4e5e9913129, 0xe7109bfba19c0c9d, 0xac2820d9623bf429,
  0x80444b5e7aa7cf85, 0xbf21e44003acdd2d, 0x8e679c2f5e44ff8f, 0xd433179d9c8cb841,
  0x9e19db92b4e31ba9, 0xeb96bf6ebadf77d9, 0xaf87023b9bf0ee6b
};

// Binary exponents of pow(10, k), for k = -348, -340, ..., 340, corresponding
// to significands above.
template <typename T>
const int16_t basic_data<T>::POW10_EXPONENTS[] = {
  -1220, -1193, -1166, -1140, -1113, -1087, -1060, -1034, -1007,  -980,  -954,
   -927,  -901,  -874,  -847,  -821,  -794,  -768,  -741,  -715,  -688,  -661,
   -635,  -608,  -582,  -555,  -529,  -502,  -475,  -449,  -422,  -396,  -369,
   -343,  -316,  -289,  -263,  -236,  -210,  -183,  -157,  -130,  -103,   -77,
    -50,   -24,     3,    30,    56,    83,   109,   136,   162,   189,   216,
    242,   269,   295,   322,   348,   375,   402,   428,   455,   481,   508,
    534,   561,   588,   614,   641,   667,   694,   720,   747,   774,   800,
    827,   853,   880,   907,   933,   960,   986,  1013,  1039,  1066
};

template <typename T> const char basic_data<T>::RESET_COLOR[] = "\x1b[0m";
template <typename T> const wchar_t basic_data<T>::WRESET_COLOR[] = L"\x1b[0m";

FMT_FUNC fp operator*(fp x, fp y) {
  // Multiply 32-bit parts of significands.
  uint64_t mask = (1ULL << 32) - 1;
  uint64_t a = x.f >> 32, b = x.f & mask;
  uint64_t c = y.f >> 32, d = y.f & mask;
  uint64_t ac = a * c, bc = b * c, ad = a * d, bd = b * d;
  // Compute mid 64-bit of result and round.
  uint64_t mid = (bd >> 32) + (ad & mask) + (bc & mask) + (1U << 31);
  return fp(ac + (ad >> 32) + (bc >> 32) + (mid >> 32), x.e + y.e + 64);
}

FMT_FUNC fp get_cached_power(int min_exponent, int &pow10_exponent) {
  const double one_over_log2_10 = 0.30102999566398114;  // 1 / log2(10)
  int index = static_cast<int>(std::ceil(
        (min_exponent + fp::significand_size - 1) * one_over_log2_10));
  // Decimal exponent of the first (smallest) cached power of 10.
  const int first_dec_exp = -348;
  // Difference between two consecutive decimal exponents in cached powers of 10.
  const int dec_exp_step = 8;
  index = (index - first_dec_exp - 1) / dec_exp_step + 1;
  pow10_exponent = first_dec_exp + index * dec_exp_step;
  return fp(data::POW10_SIGNIFICANDS[index], data::POW10_EXPONENTS[index]);
}
}  // namespace internal

#if FMT_USE_WINDOWS_H

FMT_FUNC internal::utf8_to_utf16::utf8_to_utf16(string_view s) {
  static const char ERROR_MSG[] = "cannot convert string from UTF-8 to UTF-16";
  if (s.size() > INT_MAX)
    FMT_THROW(windows_error(ERROR_INVALID_PARAMETER, ERROR_MSG));
  int s_size = static_cast<int>(s.size());
  if (s_size == 0) {
    // MultiByteToWideChar does not support zero length, handle separately.
    buffer_.resize(1);
    buffer_[0] = 0;
    return;
  }

  int length = MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), s_size, FMT_NULL, 0);
  if (length == 0)
    FMT_THROW(windows_error(GetLastError(), ERROR_MSG));
  buffer_.resize(length + 1);
  length = MultiByteToWideChar(
    CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), s_size, &buffer_[0], length);
  if (length == 0)
    FMT_THROW(windows_error(GetLastError(), ERROR_MSG));
  buffer_[length] = 0;
}

FMT_FUNC internal::utf16_to_utf8::utf16_to_utf8(wstring_view s) {
  if (int error_code = convert(s)) {
    FMT_THROW(windows_error(error_code,
        "cannot convert string from UTF-16 to UTF-8"));
  }
}

FMT_FUNC int internal::utf16_to_utf8::convert(wstring_view s) {
  if (s.size() > INT_MAX)
    return ERROR_INVALID_PARAMETER;
  int s_size = static_cast<int>(s.size());
  if (s_size == 0) {
    // WideCharToMultiByte does not support zero length, handle separately.
    buffer_.resize(1);
    buffer_[0] = 0;
    return 0;
  }

  int length = WideCharToMultiByte(
        CP_UTF8, 0, s.data(), s_size, FMT_NULL, 0, FMT_NULL, FMT_NULL);
  if (length == 0)
    return GetLastError();
  buffer_.resize(length + 1);
  length = WideCharToMultiByte(
    CP_UTF8, 0, s.data(), s_size, &buffer_[0], length, FMT_NULL, FMT_NULL);
  if (length == 0)
    return GetLastError();
  buffer_[length] = 0;
  return 0;
}

FMT_FUNC void windows_error::init(
    int err_code, string_view format_str, format_args args) {
  error_code_ = err_code;
  memory_buffer buffer;
  internal::format_windows_error(buffer, err_code, vformat(format_str, args));
  std::runtime_error &base = *this;
  base = std::runtime_error(to_string(buffer));
}

FMT_FUNC void internal::format_windows_error(
    internal::buffer &out, int error_code, string_view message) FMT_NOEXCEPT {
  FMT_TRY {
    wmemory_buffer buf;
    buf.resize(inline_buffer_size);
    for (;;) {
      wchar_t *system_message = &buf[0];
      int result = FormatMessageW(
          FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
          FMT_NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          system_message, static_cast<uint32_t>(buf.size()), FMT_NULL);
      if (result != 0) {
        utf16_to_utf8 utf8_message;
        if (utf8_message.convert(system_message) == ERROR_SUCCESS) {
          writer w(out);
          w.write(message);
          w.write(": ");
          w.write(utf8_message);
          return;
        }
        break;
      }
      if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        break;  // Can't get error message, report error code instead.
      buf.resize(buf.size() * 2);
    }
  } FMT_CATCH(...) {}
  format_error_code(out, error_code, message);
}

#endif  // FMT_USE_WINDOWS_H

FMT_FUNC void format_system_error(
    internal::buffer &out, int error_code, string_view message) FMT_NOEXCEPT {
  FMT_TRY {
    memory_buffer buf;
    buf.resize(inline_buffer_size);
    for (;;) {
      char *system_message = &buf[0];
      int result = safe_strerror(error_code, system_message, buf.size());
      if (result == 0) {
        writer w(out);
        w.write(message);
        w.write(": ");
        w.write(system_message);
        return;
      }
      if (result != ERANGE)
        break;  // Can't get error message, report error code instead.
      buf.resize(buf.size() * 2);
    }
  } FMT_CATCH(...) {}
  format_error_code(out, error_code, message);
}

template <typename Char>
void basic_fixed_buffer<Char>::grow(std::size_t) {
  FMT_THROW(std::runtime_error("buffer overflow"));
}

FMT_FUNC void internal::error_handler::on_error(const char *message) {
  FMT_THROW(format_error(message));
}

FMT_FUNC void report_system_error(
    int error_code, fmt::string_view message) FMT_NOEXCEPT {
  report_error(format_system_error, error_code, message);
}

#if FMT_USE_WINDOWS_H
FMT_FUNC void report_windows_error(
    int error_code, fmt::string_view message) FMT_NOEXCEPT {
  report_error(internal::format_windows_error, error_code, message);
}
#endif

FMT_FUNC void vprint(std::FILE *f, string_view format_str, format_args args) {
  memory_buffer buffer;
  vformat_to(buffer, format_str, args);
  std::fwrite(buffer.data(), 1, buffer.size(), f);
}

FMT_FUNC void vprint(std::FILE *f, wstring_view format_str, wformat_args args) {
  wmemory_buffer buffer;
  vformat_to(buffer, format_str, args);
  std::fwrite(buffer.data(), sizeof(wchar_t), buffer.size(), f);
}

FMT_FUNC void vprint(string_view format_str, format_args args) {
  vprint(stdout, format_str, args);
}

FMT_FUNC void vprint(wstring_view format_str, wformat_args args) {
  vprint(stdout, format_str, args);
}

#ifndef FMT_EXTENDED_COLORS
FMT_FUNC void vprint_colored(color c, string_view format, format_args args) {
  char escape[] = "\x1b[30m";
  escape[3] = static_cast<char>('0' + c);
  std::fputs(escape, stdout);
  vprint(format, args);
  std::fputs(internal::data::RESET_COLOR, stdout);
}

FMT_FUNC void vprint_colored(color c, wstring_view format, wformat_args args) {
  wchar_t escape[] = L"\x1b[30m";
  escape[3] = static_cast<wchar_t>('0' + c);
  std::fputws(escape, stdout);
  vprint(format, args);
  std::fputws(internal::data::WRESET_COLOR, stdout);
}
#else
namespace internal {
FMT_CONSTEXPR void to_esc(uint8_t c, char out[], int offset) {
  out[offset + 0] = static_cast<char>('0' + c / 100);
  out[offset + 1] = static_cast<char>('0' + c / 10 % 10);
  out[offset + 2] = static_cast<char>('0' + c % 10);
}
} // namespace internal

FMT_FUNC void vprint_rgb(rgb fd, string_view format, format_args args) {
  char escape_fd[] = "\x1b[38;2;000;000;000m";
  internal::to_esc(fd.r, escape_fd, 7);
  internal::to_esc(fd.g, escape_fd, 11);
  internal::to_esc(fd.b, escape_fd, 15);

  std::fputs(escape_fd, stdout);
  vprint(format, args);
  std::fputs(internal::data::RESET_COLOR, stdout);
}

FMT_FUNC void vprint_rgb(rgb fd, rgb bg, string_view format, format_args args) {
  char escape_fd[] = "\x1b[38;2;000;000;000m"; // foreground color
  char escape_bg[] = "\x1b[48;2;000;000;000m"; // background color
  internal::to_esc(fd.r, escape_fd, 7);
  internal::to_esc(fd.g, escape_fd, 11);
  internal::to_esc(fd.b, escape_fd, 15);

  internal::to_esc(bg.r, escape_bg, 7);
  internal::to_esc(bg.g, escape_bg, 11);
  internal::to_esc(bg.b, escape_bg, 15);

  std::fputs(escape_fd, stdout);
  std::fputs(escape_bg, stdout);
  vprint(format, args);
  std::fputs(internal::data::RESET_COLOR, stdout);
}
#endif

FMT_FUNC locale locale_provider::locale() { return fmt::locale(); }

FMT_END_NAMESPACE

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif  // FMT_FORMAT_INL_H_
