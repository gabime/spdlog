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
#include <cstring>  // for std::memmove
#if !defined(FMT_STATIC_THOUSANDS_SEPARATOR)
# include <locale>
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

#if !FMT_MSC_VER
    // Fallback to strerror if strerror_r and strerror_s are not available.
    int fallback(internal::null<>) {
      errno = 0;
      buffer_ = strerror(error_code_);
      return errno;
    }
#endif

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
  error_code_size += internal::to_unsigned(internal::count_digits(abs_value));
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

FMT_FUNC size_t internal::count_code_points(basic_string_view<char8_t> s) {
  const char8_t *data = s.data();
  size_t num_code_points = 0;
  for (size_t i = 0, size = s.size(); i != size; ++i) {
    if ((data[i] & 0xc0) != 0x80)
      ++num_code_points;
  }
  return num_code_points;
}

#if !defined(FMT_STATIC_THOUSANDS_SEPARATOR)
namespace internal {

template <typename Locale>
locale_ref::locale_ref(const Locale &loc) : locale_(&loc) {
  static_assert(std::is_same<Locale, std::locale>::value, "");
}

template <typename Locale>
Locale locale_ref::get() const {
  static_assert(std::is_same<Locale, std::locale>::value, "");
  return locale_ ? *static_cast<const std::locale*>(locale_) : std::locale();
}

template <typename Char>
FMT_FUNC Char thousands_sep_impl(locale_ref loc) {
  return std::use_facet<std::numpunct<Char> >(
    loc.get<std::locale>()).thousands_sep();
}
}
#else
template <typename Char>
FMT_FUNC Char internal::thousands_sep_impl(locale_ref) {
  return FMT_STATIC_THOUSANDS_SEPARATOR;
}
#endif

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
    char *buf, std::size_t size, const char *format, int precision, T value) {
  return precision < 0 ?
      FMT_SNPRINTF(buf, size, format, value) :
      FMT_SNPRINTF(buf, size, format, precision, value);
}

template <typename T>
int char_traits<wchar_t>::format_float(
    wchar_t *buf, std::size_t size, const wchar_t *format, int precision,
    T value) {
  return precision < 0 ?
      FMT_SWPRINTF(buf, size, format, value) :
      FMT_SWPRINTF(buf, size, format, precision, value);
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
  1, FMT_POWERS_OF_10(1)
};

template <typename T>
const uint32_t basic_data<T>::ZERO_OR_POWERS_OF_10_32[] = {
  0, FMT_POWERS_OF_10(1)
};

template <typename T>
const uint64_t basic_data<T>::ZERO_OR_POWERS_OF_10_64[] = {
  0,
  FMT_POWERS_OF_10(1),
  FMT_POWERS_OF_10(1000000000ull),
  10000000000000000000ull
};

// Normalized 64-bit significands of pow(10, k), for k = -348, -340, ..., 340.
// These are generated by support/compute-powers.py.
template <typename T>
const uint64_t basic_data<T>::POW10_SIGNIFICANDS[] = {
  0xfa8fd5a0081c0288, 0xbaaee17fa23ebf76, 0x8b16fb203055ac76,
  0xcf42894a5dce35ea, 0x9a6bb0aa55653b2d, 0xe61acf033d1a45df,
  0xab70fe17c79ac6ca, 0xff77b1fcbebcdc4f, 0xbe5691ef416bd60c,
  0x8dd01fad907ffc3c, 0xd3515c2831559a83, 0x9d71ac8fada6c9b5,
  0xea9c227723ee8bcb, 0xaecc49914078536d, 0x823c12795db6ce57,
  0xc21094364dfb5637, 0x9096ea6f3848984f, 0xd77485cb25823ac7,
  0xa086cfcd97bf97f4, 0xef340a98172aace5, 0xb23867fb2a35b28e,
  0x84c8d4dfd2c63f3b, 0xc5dd44271ad3cdba, 0x936b9fcebb25c996,
  0xdbac6c247d62a584, 0xa3ab66580d5fdaf6, 0xf3e2f893dec3f126,
  0xb5b5ada8aaff80b8, 0x87625f056c7c4a8b, 0xc9bcff6034c13053,
  0x964e858c91ba2655, 0xdff9772470297ebd, 0xa6dfbd9fb8e5b88f,
  0xf8a95fcf88747d94, 0xb94470938fa89bcf, 0x8a08f0f8bf0f156b,
  0xcdb02555653131b6, 0x993fe2c6d07b7fac, 0xe45c10c42a2b3b06,
  0xaa242499697392d3, 0xfd87b5f28300ca0e, 0xbce5086492111aeb,
  0x8cbccc096f5088cc, 0xd1b71758e219652c, 0x9c40000000000000,
  0xe8d4a51000000000, 0xad78ebc5ac620000, 0x813f3978f8940984,
  0xc097ce7bc90715b3, 0x8f7e32ce7bea5c70, 0xd5d238a4abe98068,
  0x9f4f2726179a2245, 0xed63a231d4c4fb27, 0xb0de65388cc8ada8,
  0x83c7088e1aab65db, 0xc45d1df942711d9a, 0x924d692ca61be758,
  0xda01ee641a708dea, 0xa26da3999aef774a, 0xf209787bb47d6b85,
  0xb454e4a179dd1877, 0x865b86925b9bc5c2, 0xc83553c5c8965d3d,
  0x952ab45cfa97a0b3, 0xde469fbd99a05fe3, 0xa59bc234db398c25,
  0xf6c69a72a3989f5c, 0xb7dcbf5354e9bece, 0x88fcf317f22241e2,
  0xcc20ce9bd35c78a5, 0x98165af37b2153df, 0xe2a0b5dc971f303a,
  0xa8d9d1535ce3b396, 0xfb9b7cd9a4a7443c, 0xbb764c4ca7a44410,
  0x8bab8eefb6409c1a, 0xd01fef10a657842c, 0x9b10a4e5e9913129,
  0xe7109bfba19c0c9d, 0xac2820d9623bf429, 0x80444b5e7aa7cf85,
  0xbf21e44003acdd2d, 0x8e679c2f5e44ff8f, 0xd433179d9c8cb841,
  0x9e19db92b4e31ba9, 0xeb96bf6ebadf77d9, 0xaf87023b9bf0ee6b,
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

template <typename T> const char basic_data<T>::FOREGROUND_COLOR[] = "\x1b[38;2;";
template <typename T> const char basic_data<T>::BACKGROUND_COLOR[] = "\x1b[48;2;";
template <typename T> const char basic_data<T>::RESET_COLOR[] = "\x1b[0m";
template <typename T> const wchar_t basic_data<T>::WRESET_COLOR[] = L"\x1b[0m";

// A handmade floating-point number f * pow(2, e).
class fp {
 private:
  typedef uint64_t significand_type;

  // All sizes are in bits.
  static FMT_CONSTEXPR_DECL const int char_size =
    std::numeric_limits<unsigned char>::digits;
  // Subtract 1 to account for an implicit most significant bit in the
  // normalized form.
  static FMT_CONSTEXPR_DECL const int double_significand_size =
    std::numeric_limits<double>::digits - 1;
  static FMT_CONSTEXPR_DECL const uint64_t implicit_bit =
    1ull << double_significand_size;

 public:
  significand_type f;
  int e;

  static FMT_CONSTEXPR_DECL const int significand_size =
    sizeof(significand_type) * char_size;

  fp(): f(0), e(0) {}
  fp(uint64_t f_val, int e_val): f(f_val), e(e_val) {}

  // Constructs fp from an IEEE754 double. It is a template to prevent compile
  // errors on platforms where double is not IEEE754.
  template <typename Double>
  explicit fp(Double d) {
    // Assume double is in the format [sign][exponent][significand].
    typedef std::numeric_limits<Double> limits;
    const int double_size = static_cast<int>(sizeof(Double) * char_size);
    const int exponent_size =
      double_size - double_significand_size - 1;  // -1 for sign
    const uint64_t significand_mask = implicit_bit - 1;
    const uint64_t exponent_mask = (~0ull >> 1) & ~significand_mask;
    const int exponent_bias = (1 << exponent_size) - limits::max_exponent - 1;
    auto u = bit_cast<uint64_t>(d);
    auto biased_e = (u & exponent_mask) >> double_significand_size;
    f = u & significand_mask;
    if (biased_e != 0)
      f += implicit_bit;
    else
      biased_e = 1;  // Subnormals use biased exponent 1 (min exponent).
    e = static_cast<int>(biased_e - exponent_bias - double_significand_size);
  }

  // Normalizes the value converted from double and multiplied by (1 << SHIFT).
  template <int SHIFT = 0>
  void normalize() {
    // Handle subnormals.
    auto shifted_implicit_bit = implicit_bit << SHIFT;
    while ((f & shifted_implicit_bit) == 0) {
      f <<= 1;
      --e;
    }
    // Subtract 1 to account for hidden bit.
    auto offset = significand_size - double_significand_size - SHIFT - 1;
    f <<= offset;
    e -= offset;
  }

  // Compute lower and upper boundaries (m^- and m^+ in the Grisu paper), where
  // a boundary is a value half way between the number and its predecessor
  // (lower) or successor (upper). The upper boundary is normalized and lower
  // has the same exponent but may be not normalized.
  void compute_boundaries(fp &lower, fp &upper) const {
    lower = f == implicit_bit ?
          fp((f << 2) - 1, e - 2) : fp((f << 1) - 1, e - 1);
    upper = fp((f << 1) + 1, e - 1);
    upper.normalize<1>();  // 1 is to account for the exponent shift above.
    lower.f <<= lower.e - upper.e;
    lower.e = upper.e;
  }
};

// Returns an fp number representing x - y. Result may not be normalized.
inline fp operator-(fp x, fp y) {
  FMT_ASSERT(x.f >= y.f && x.e == y.e, "invalid operands");
  return fp(x.f - y.f, x.e);
}

// Computes an fp number r with r.f = x.f * y.f / pow(2, 64) rounded to nearest
// with half-up tie breaking, r.e = x.e + y.e + 64. Result may not be normalized.
FMT_API fp operator*(fp x, fp y);

// Returns cached power (of 10) c_k = c_k.f * pow(2, c_k.e) such that its
// (binary) exponent satisfies min_exponent <= c_k.e <= min_exponent + 3.
FMT_API fp get_cached_power(int min_exponent, int &pow10_exponent);

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
  // Difference between 2 consecutive decimal exponents in cached powers of 10.
  const int dec_exp_step = 8;
  index = (index - first_dec_exp - 1) / dec_exp_step + 1;
  pow10_exponent = first_dec_exp + index * dec_exp_step;
  return fp(data::POW10_SIGNIFICANDS[index], data::POW10_EXPONENTS[index]);
}

FMT_FUNC bool grisu2_round(
    char *buf, int &size, int max_digits, uint64_t delta,
    uint64_t remainder, uint64_t exp, uint64_t diff, int &exp10) {
  while (remainder < diff && delta - remainder >= exp &&
        (remainder + exp < diff || diff - remainder > remainder + exp - diff)) {
    --buf[size - 1];
    remainder += exp;
  }
  if (size > max_digits) {
    --size;
    ++exp10;
    if (buf[size] >= '5')
      return false;
  }
  return true;
}

// Generates output using Grisu2 digit-gen algorithm.
FMT_FUNC bool grisu2_gen_digits(
    char *buf, int &size, uint32_t hi, uint64_t lo, int &exp,
    uint64_t delta, const fp &one, const fp &diff, int max_digits) {
  // Generate digits for the most significant part (hi).
  while (exp > 0) {
    uint32_t digit = 0;
    // This optimization by miloyip reduces the number of integer divisions by
    // one per iteration.
    switch (exp) {
    case 10: digit = hi / 1000000000; hi %= 1000000000; break;
    case  9: digit = hi /  100000000; hi %=  100000000; break;
    case  8: digit = hi /   10000000; hi %=   10000000; break;
    case  7: digit = hi /    1000000; hi %=    1000000; break;
    case  6: digit = hi /     100000; hi %=     100000; break;
    case  5: digit = hi /      10000; hi %=      10000; break;
    case  4: digit = hi /       1000; hi %=       1000; break;
    case  3: digit = hi /        100; hi %=        100; break;
    case  2: digit = hi /         10; hi %=         10; break;
    case  1: digit = hi;              hi =           0; break;
    default:
      FMT_ASSERT(false, "invalid number of digits");
    }
    if (digit != 0 || size != 0)
      buf[size++] = static_cast<char>('0' + digit);
    --exp;
    uint64_t remainder = (static_cast<uint64_t>(hi) << -one.e) + lo;
    if (remainder <= delta || size > max_digits) {
      return grisu2_round(
            buf, size, max_digits, delta, remainder,
            static_cast<uint64_t>(data::POWERS_OF_10_32[exp]) << -one.e,
            diff.f, exp);
    }
  }
  // Generate digits for the least significant part (lo).
  for (;;) {
    lo *= 10;
    delta *= 10;
    char digit = static_cast<char>(lo >> -one.e);
    if (digit != 0 || size != 0)
      buf[size++] = static_cast<char>('0' + digit);
    lo &= one.f - 1;
    --exp;
    if (lo < delta || size > max_digits) {
      return grisu2_round(buf, size, max_digits, delta, lo, one.f,
                          diff.f * data::POWERS_OF_10_32[-exp], exp);
    }
  }
}

#if FMT_CLANG_VERSION
# define FMT_FALLTHROUGH [[clang::fallthrough]];
#elif FMT_GCC_VERSION >= 700
# define FMT_FALLTHROUGH [[gnu::fallthrough]];
#else
# define FMT_FALLTHROUGH
#endif

struct gen_digits_params {
  int num_digits;
  bool fixed;
  bool upper;
  bool trailing_zeros;
};

struct prettify_handler {
  char *data;
  ptrdiff_t size;
  buffer &buf;

  explicit prettify_handler(buffer &b, ptrdiff_t n)
    : data(b.data()), size(n), buf(b) {}
  ~prettify_handler() {
    assert(buf.size() >= to_unsigned(size));
    buf.resize(to_unsigned(size));
  }

  template <typename F>
  void insert(ptrdiff_t pos, ptrdiff_t n, F f) {
    std::memmove(data + pos + n, data + pos, to_unsigned(size - pos));
    f(data + pos);
    size += n;
  }

  void insert(ptrdiff_t pos, char c) {
    std::memmove(data + pos + 1, data + pos, to_unsigned(size - pos));
    data[pos] = c;
    ++size;
  }

  void append(ptrdiff_t n, char c) {
    std::uninitialized_fill_n(data + size, n, c);
    size += n;
  }

  void append(char c) { data[size++] = c; }

  void remove_trailing(char c) {
    while (data[size - 1] == c) --size;
  }
};

// Writes the exponent exp in the form "[+-]d{2,3}" to buffer.
template <typename Handler>
FMT_FUNC void write_exponent(int exp, Handler &&h) {
  FMT_ASSERT(-1000 < exp && exp < 1000, "exponent out of range");
  if (exp < 0) {
    h.append('-');
    exp = -exp;
  } else {
    h.append('+');
  }
  if (exp >= 100) {
    h.append(static_cast<char>('0' + exp / 100));
    exp %= 100;
    const char *d = data::DIGITS + exp * 2;
    h.append(d[0]);
    h.append(d[1]);
  } else {
    const char *d = data::DIGITS + exp * 2;
    h.append(d[0]);
    h.append(d[1]);
  }
}

struct fill {
  size_t n;
  void operator()(char *buf) const {
    buf[0] = '0';
    buf[1] = '.';
    std::uninitialized_fill_n(buf + 2, n, '0');
  }
};

// The number is given as v = f * pow(10, exp), where f has size digits.
template <typename Handler>
FMT_FUNC void grisu2_prettify(const gen_digits_params &params,
                              int size, int exp, Handler &&handler) {
  if (!params.fixed) {
    // Insert a decimal point after the first digit and add an exponent.
    handler.insert(1, '.');
    exp += size - 1;
    if (size < params.num_digits)
      handler.append(params.num_digits - size, '0');
    handler.append(params.upper ? 'E' : 'e');
    write_exponent(exp, handler);
    return;
  }
  // pow(10, full_exp - 1) <= v <= pow(10, full_exp).
  int full_exp = size + exp;
  const int exp_threshold = 21;
  if (size <= full_exp && full_exp <= exp_threshold) {
    // 1234e7 -> 12340000000[.0+]
    handler.append(full_exp - size, '0');
    int num_zeros = params.num_digits - full_exp;
    if (num_zeros > 0 && params.trailing_zeros) {
      handler.append('.');
      handler.append(num_zeros, '0');
    }
  } else if (full_exp > 0) {
    // 1234e-2 -> 12.34[0+]
    handler.insert(full_exp, '.');
    if (!params.trailing_zeros) {
      // Remove trailing zeros.
      handler.remove_trailing('0');
    } else if (params.num_digits > size) {
      // Add trailing zeros.
      ptrdiff_t num_zeros = params.num_digits - size;
      handler.append(num_zeros, '0');
    }
  } else {
    // 1234e-6 -> 0.001234
    handler.insert(0, 2 - full_exp, fill{to_unsigned(-full_exp)});
  }
}

struct char_counter {
  ptrdiff_t size;

  template <typename F>
  void insert(ptrdiff_t, ptrdiff_t n, F) { size += n; }
  void insert(ptrdiff_t, char) { ++size; }
  void append(ptrdiff_t n, char) { size += n; }
  void append(char) { ++size; }
  void remove_trailing(char) {}
};

// Converts format specifiers into parameters for digit generation and computes
// output buffer size for a number in the range [pow(10, exp - 1), pow(10, exp)
// or 0 if exp == 1.
FMT_FUNC gen_digits_params process_specs(const core_format_specs &specs,
                                         int exp, buffer &buf) {
  auto params = gen_digits_params();
  int num_digits = specs.precision >= 0 ? specs.precision : 6;
  switch (specs.type) {
  case 'G':
    params.upper = true;
    FMT_FALLTHROUGH
  case '\0': case 'g':
    params.trailing_zeros = (specs.flags & HASH_FLAG) != 0;
    if (-4 <= exp && exp < num_digits + 1) {
      params.fixed = true;
      if (!specs.type && params.trailing_zeros && exp >= 0)
        num_digits = exp + 1;
    }
    break;
  case 'F':
    params.upper = true;
    FMT_FALLTHROUGH
  case 'f': {
    params.fixed = true;
    params.trailing_zeros = true;
    int adjusted_min_digits = num_digits + exp;
    if (adjusted_min_digits > 0)
      num_digits = adjusted_min_digits;
    break;
  }
  case 'E':
    params.upper = true;
    FMT_FALLTHROUGH
  case 'e':
    ++num_digits;
    break;
  }
  params.num_digits = num_digits;
  char_counter counter{num_digits};
  grisu2_prettify(params, params.num_digits, exp - num_digits, counter);
  buf.resize(to_unsigned(counter.size));
  return params;
}

template <typename Double>
FMT_FUNC typename std::enable_if<sizeof(Double) == sizeof(uint64_t), bool>::type
    grisu2_format(Double value, buffer &buf, core_format_specs specs) {
  FMT_ASSERT(value >= 0, "value is negative");
  if (value == 0) {
    gen_digits_params params = process_specs(specs, 1, buf);
    const size_t size = 1;
    buf[0] = '0';
    grisu2_prettify(params, size, 0, prettify_handler(buf, size));
    return true;
  }

  fp fp_value(value);
  fp lower, upper;  // w^- and w^+ in the Grisu paper.
  fp_value.compute_boundaries(lower, upper);

  // Find a cached power of 10 close to 1 / upper and use it to scale upper.
  const int min_exp = -60;  // alpha in Grisu.
  int cached_exp = 0;  // K in Grisu.
  auto cached_pow = get_cached_power(  // \tilde{c}_{-k} in Grisu.
      min_exp - (upper.e + fp::significand_size), cached_exp);
  cached_exp = -cached_exp;
  upper = upper * cached_pow;  // \tilde{M}^+ in Grisu.
  --upper.f;  // \tilde{M}^+ - 1 ulp -> M^+_{\downarrow}.
  fp one(1ull << -upper.e, upper.e);
  // hi (p1 in Grisu) contains the most significant digits of scaled_upper.
  // hi = floor(upper / one).
  uint32_t hi = static_cast<uint32_t>(upper.f >> -one.e);
  int exp = count_digits(hi);  // kappa in Grisu.
  gen_digits_params params = process_specs(specs, cached_exp + exp, buf);
  fp_value.normalize();
  fp scaled_value = fp_value * cached_pow;
  lower = lower * cached_pow;  // \tilde{M}^- in Grisu.
  ++lower.f;  // \tilde{M}^- + 1 ulp -> M^-_{\uparrow}.
  uint64_t delta = upper.f - lower.f;
  fp diff = upper - scaled_value; // wp_w in Grisu.
  // lo (p2 in Grisu) contains the least significants digits of scaled_upper.
  // lo = supper % one.
  uint64_t lo = upper.f & (one.f - 1);
  int size = 0;
  if (!grisu2_gen_digits(buf.data(), size, hi, lo, exp, delta, one, diff,
                         params.num_digits)) {
    buf.clear();
    return false;
  }
  grisu2_prettify(params, size, cached_exp + exp, prettify_handler(buf, size));
  return true;
}

template <typename Double>
void sprintf_format(Double value, internal::buffer &buf,
                    core_format_specs spec) {
  // Buffer capacity must be non-zero, otherwise MSVC's vsnprintf_s will fail.
  FMT_ASSERT(buf.capacity() != 0, "empty buffer");

  // Build format string.
  enum { MAX_FORMAT_SIZE = 10}; // longest format: %#-*.*Lg
  char format[MAX_FORMAT_SIZE];
  char *format_ptr = format;
  *format_ptr++ = '%';
  if (spec.has(HASH_FLAG))
    *format_ptr++ = '#';
  if (spec.precision >= 0) {
    *format_ptr++ = '.';
    *format_ptr++ = '*';
  }
  if (std::is_same<Double, long double>::value)
    *format_ptr++ = 'L';
  *format_ptr++ = spec.type;
  *format_ptr = '\0';

  // Format using snprintf.
  char *start = FMT_NULL;
  for (;;) {
    std::size_t buffer_size = buf.capacity();
    start = &buf[0];
    int result = internal::char_traits<char>::format_float(
        start, buffer_size, format, spec.precision, value);
    if (result >= 0) {
      unsigned n = internal::to_unsigned(result);
      if (n < buf.capacity()) {
        buf.resize(n);
        break;  // The buffer is large enough - continue with formatting.
      }
      buf.reserve(n + 1);
    } else {
      // If result is negative we ask to increase the capacity by at least 1,
      // but as std::vector, the buffer grows exponentially.
      buf.reserve(buf.capacity() + 1);
    }
  }
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
  internal::vformat_to(buffer, format_str,
                       basic_format_args<buffer_context<char>::type>(args));
  std::fwrite(buffer.data(), 1, buffer.size(), f);
}

FMT_FUNC void vprint(std::FILE *f, wstring_view format_str, wformat_args args) {
  wmemory_buffer buffer;
  internal::vformat_to(buffer, format_str, args);
  std::fwrite(buffer.data(), sizeof(wchar_t), buffer.size(), f);
}

FMT_FUNC void vprint(string_view format_str, format_args args) {
  vprint(stdout, format_str, args);
}

FMT_FUNC void vprint(wstring_view format_str, wformat_args args) {
  vprint(stdout, format_str, args);
}

FMT_END_NAMESPACE

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif  // FMT_FORMAT_INL_H_
