/*
 Formatting library for C++

 Copyright (c) 2012 - present, Victor Zverovich
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

#ifndef FMT_FORMAT_H_
#define FMT_FORMAT_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <stdint.h>

#ifdef __clang__
# define FMT_CLANG_VERSION (__clang_major__ * 100 + __clang_minor__)
#else
# define FMT_CLANG_VERSION 0
#endif

#ifdef __INTEL_COMPILER
# define FMT_ICC_VERSION __INTEL_COMPILER
#elif defined(__ICL)
# define FMT_ICC_VERSION __ICL
#else
# define FMT_ICC_VERSION 0
#endif

#ifdef __NVCC__
# define FMT_CUDA_VERSION (__CUDACC_VER_MAJOR__ * 100 + __CUDACC_VER_MINOR__)
#else
# define FMT_CUDA_VERSION 0
#endif

#include "core.h"

#if FMT_GCC_VERSION >= 406 || FMT_CLANG_VERSION
# pragma GCC diagnostic push

// Disable the warning about declaration shadowing because it affects too
// many valid cases.
# pragma GCC diagnostic ignored "-Wshadow"

// Disable the warning about implicit conversions that may change the sign of
// an integer; silencing it otherwise would require many explicit casts.
# pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

# if FMT_CLANG_VERSION
#  pragma GCC diagnostic ignored "-Wgnu-string-literal-operator-template"
# endif

#ifdef _SECURE_SCL
# define FMT_SECURE_SCL _SECURE_SCL
#else
# define FMT_SECURE_SCL 0
#endif

#if FMT_SECURE_SCL
# include <iterator>
#endif

#ifdef __has_builtin
# define FMT_HAS_BUILTIN(x) __has_builtin(x)
#else
# define FMT_HAS_BUILTIN(x) 0
#endif

#ifdef __GNUC_LIBSTD__
# define FMT_GNUC_LIBSTD_VERSION (__GNUC_LIBSTD__ * 100 + __GNUC_LIBSTD_MINOR__)
#endif

#ifndef FMT_THROW
# if FMT_EXCEPTIONS
#  if FMT_MSC_VER
FMT_BEGIN_NAMESPACE
namespace internal {
template <typename Exception>
inline void do_throw(const Exception &x) {
  // Silence unreachable code warnings in MSVC because these are nearly
  // impossible to fix in a generic code.
  volatile bool b = true;
  if (b)
    throw x;
}
}
FMT_END_NAMESPACE
#   define FMT_THROW(x) fmt::internal::do_throw(x)
#  else
#   define FMT_THROW(x) throw x
#  endif
# else
#  define FMT_THROW(x) do { static_cast<void>(sizeof(x)); assert(false); } while(false);
# endif
#endif

#ifndef FMT_USE_USER_DEFINED_LITERALS
// For Intel's compiler and NVIDIA's compiler both it and the system gcc/msc
// must support UDLs.
# if (FMT_HAS_FEATURE(cxx_user_literals) || \
      FMT_GCC_VERSION >= 407 || FMT_MSC_VER >= 1900) && \
      (!(FMT_ICC_VERSION || FMT_CUDA_VERSION) || \
       FMT_ICC_VERSION >= 1500 || FMT_CUDA_VERSION >= 700)
#  define FMT_USE_USER_DEFINED_LITERALS 1
# else
#  define FMT_USE_USER_DEFINED_LITERALS 0
# endif
#endif

// EDG C++ Front End based compilers (icc, nvcc) do not currently support UDL
// templates.
#if FMT_USE_USER_DEFINED_LITERALS && \
    FMT_ICC_VERSION == 0 && \
    FMT_CUDA_VERSION == 0 && \
    ((FMT_GCC_VERSION >= 600 && __cplusplus >= 201402L) || \
    (defined(FMT_CLANG_VERSION) && FMT_CLANG_VERSION >= 304))
# define FMT_UDL_TEMPLATE 1
#else
# define FMT_UDL_TEMPLATE 0
#endif

#ifndef FMT_USE_EXTERN_TEMPLATES
# ifndef FMT_HEADER_ONLY
#  define FMT_USE_EXTERN_TEMPLATES \
     ((FMT_CLANG_VERSION >= 209 && __cplusplus >= 201103L) || \
      (FMT_GCC_VERSION >= 303 && FMT_HAS_GXX_CXX11))
# else
#  define FMT_USE_EXTERN_TEMPLATES 0
# endif
#endif

#if FMT_HAS_GXX_CXX11 || FMT_HAS_FEATURE(cxx_trailing_return) || \
    FMT_MSC_VER >= 1600
# define FMT_USE_TRAILING_RETURN 1
#else
# define FMT_USE_TRAILING_RETURN 0
#endif

#ifndef FMT_USE_GRISU
# define FMT_USE_GRISU 0
#endif

// __builtin_clz is broken in clang with Microsoft CodeGen:
// https://github.com/fmtlib/fmt/issues/519
#ifndef _MSC_VER
# if FMT_GCC_VERSION >= 400 || FMT_HAS_BUILTIN(__builtin_clz)
#  define FMT_BUILTIN_CLZ(n) __builtin_clz(n)
# endif

# if FMT_GCC_VERSION >= 400 || FMT_HAS_BUILTIN(__builtin_clzll)
#  define FMT_BUILTIN_CLZLL(n) __builtin_clzll(n)
# endif
#endif

// A workaround for gcc 4.4 that doesn't support union members with ctors.
#if (FMT_GCC_VERSION && FMT_GCC_VERSION <= 404) || \
    (FMT_MSC_VER && FMT_MSC_VER <= 1800)
# define FMT_UNION struct
#else
# define FMT_UNION union
#endif

// Some compilers masquerade as both MSVC and GCC-likes or otherwise support
// __builtin_clz and __builtin_clzll, so only define FMT_BUILTIN_CLZ using the
// MSVC intrinsics if the clz and clzll builtins are not available.
#if FMT_MSC_VER && !defined(FMT_BUILTIN_CLZLL) && !defined(_MANAGED)
# include <intrin.h>  // _BitScanReverse, _BitScanReverse64

FMT_BEGIN_NAMESPACE
namespace internal {
// Avoid Clang with Microsoft CodeGen's -Wunknown-pragmas warning.
# ifndef __clang__
#  pragma intrinsic(_BitScanReverse)
# endif
inline uint32_t clz(uint32_t x) {
  unsigned long r = 0;
  _BitScanReverse(&r, x);

  assert(x != 0);
  // Static analysis complains about using uninitialized data
  // "r", but the only way that can happen is if "x" is 0,
  // which the callers guarantee to not happen.
# pragma warning(suppress: 6102)
  return 31 - r;
}
# define FMT_BUILTIN_CLZ(n) fmt::internal::clz(n)

# if defined(_WIN64) && !defined(__clang__)
#  pragma intrinsic(_BitScanReverse64)
# endif

inline uint32_t clzll(uint64_t x) {
  unsigned long r = 0;
# ifdef _WIN64
  _BitScanReverse64(&r, x);
# else
  // Scan the high 32 bits.
  if (_BitScanReverse(&r, static_cast<uint32_t>(x >> 32)))
    return 63 - (r + 32);

  // Scan the low 32 bits.
  _BitScanReverse(&r, static_cast<uint32_t>(x));
# endif

  assert(x != 0);
  // Static analysis complains about using uninitialized data
  // "r", but the only way that can happen is if "x" is 0,
  // which the callers guarantee to not happen.
# pragma warning(suppress: 6102)
  return 63 - r;
}
# define FMT_BUILTIN_CLZLL(n) fmt::internal::clzll(n)
}
FMT_END_NAMESPACE
#endif

FMT_BEGIN_NAMESPACE
namespace internal {

// An equivalent of `*reinterpret_cast<Dest*>(&source)` that doesn't produce
// undefined behavior (e.g. due to type aliasing).
// Example: uint64_t d = bit_cast<uint64_t>(2.718);
template <typename Dest, typename Source>
inline Dest bit_cast(const Source& source) {
  static_assert(sizeof(Dest) == sizeof(Source), "size mismatch");
  Dest dest;
  std::memcpy(&dest, &source, sizeof(dest));
  return dest;
}

// An implementation of begin and end for pre-C++11 compilers such as gcc 4.
template <typename C>
FMT_CONSTEXPR auto begin(const C &c) -> decltype(c.begin()) {
  return c.begin();
}
template <typename T, std::size_t N>
FMT_CONSTEXPR T *begin(T (&array)[N]) FMT_NOEXCEPT { return array; }
template <typename C>
FMT_CONSTEXPR auto end(const C &c) -> decltype(c.end()) { return c.end(); }
template <typename T, std::size_t N>
FMT_CONSTEXPR T *end(T (&array)[N]) FMT_NOEXCEPT { return array + N; }

// For std::result_of in gcc 4.4.
template <typename Result>
struct function {
  template <typename T>
  struct result { typedef Result type; };
};

struct dummy_int {
  int data[2];
  operator int() const { return 0; }
};
typedef std::numeric_limits<internal::dummy_int> fputil;

// Dummy implementations of system functions such as signbit and ecvt called
// if the latter are not available.
inline dummy_int signbit(...) { return dummy_int(); }
inline dummy_int _ecvt_s(...) { return dummy_int(); }
inline dummy_int isinf(...) { return dummy_int(); }
inline dummy_int _finite(...) { return dummy_int(); }
inline dummy_int isnan(...) { return dummy_int(); }
inline dummy_int _isnan(...) { return dummy_int(); }

inline bool use_grisu() {
  return FMT_USE_GRISU && std::numeric_limits<double>::is_iec559;
}

// Formats value using Grisu2 algorithm:
// https://www.cs.tufts.edu/~nr/cs257/archive/florian-loitsch/printf.pdf
FMT_API void grisu2_format(double value, char *buffer, size_t &size, char type,
                           int precision, bool write_decimal_point);

template <typename Allocator>
typename Allocator::value_type *allocate(Allocator& alloc, std::size_t n) {
#if __cplusplus >= 201103L || FMT_MSC_VER >= 1700
  return std::allocator_traits<Allocator>::allocate(alloc, n);
#else
  return alloc.allocate(n);
#endif
}

// A helper function to suppress bogus "conditional expression is constant"
// warnings.
template <typename T>
inline T const_check(T value) { return value; }
}  // namespace internal
FMT_END_NAMESPACE

namespace std {
// Standard permits specialization of std::numeric_limits. This specialization
// is used to resolve ambiguity between isinf and std::isinf in glibc:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=48891
// and the same for isnan and signbit.
template <>
class numeric_limits<fmt::internal::dummy_int> :
    public std::numeric_limits<int> {
 public:
  // Portable version of isinf.
  template <typename T>
  static bool isinfinity(T x) {
    using namespace fmt::internal;
    // The resolution "priority" is:
    // isinf macro > std::isinf > ::isinf > fmt::internal::isinf
    if (const_check(sizeof(isinf(x)) != sizeof(dummy_int)))
      return isinf(x) != 0;
    return !_finite(static_cast<double>(x));
  }

  // Portable version of isnan.
  template <typename T>
  static bool isnotanumber(T x) {
    using namespace fmt::internal;
    if (const_check(sizeof(isnan(x)) != sizeof(fmt::internal::dummy_int)))
      return isnan(x) != 0;
    return _isnan(static_cast<double>(x)) != 0;
  }

  // Portable version of signbit.
  static bool isnegative(double x) {
    using namespace fmt::internal;
    if (const_check(sizeof(signbit(x)) != sizeof(fmt::internal::dummy_int)))
      return signbit(x) != 0;
    if (x < 0) return true;
    if (!isnotanumber(x)) return false;
    int dec = 0, sign = 0;
    char buffer[2];  // The buffer size must be >= 2 or _ecvt_s will fail.
    _ecvt_s(buffer, sizeof(buffer), x, 0, &dec, &sign);
    return sign != 0;
  }
};
}  // namespace std

FMT_BEGIN_NAMESPACE
template <typename Range>
class basic_writer;

template <typename OutputIt, typename T = typename OutputIt::value_type>
class output_range {
 private:
  OutputIt it_;

  // Unused yet.
  typedef void sentinel;
  sentinel end() const;

 public:
  typedef OutputIt iterator;
  typedef T value_type;

  explicit output_range(OutputIt it): it_(it) {}
  OutputIt begin() const { return it_; }
};

// A range where begin() returns back_insert_iterator.
template <typename Container>
class back_insert_range:
    public output_range<std::back_insert_iterator<Container>> {
  typedef output_range<std::back_insert_iterator<Container>> base;
 public:
  typedef typename Container::value_type value_type;

  back_insert_range(Container &c): base(std::back_inserter(c)) {}
  back_insert_range(typename base::iterator it): base(it) {}
};

typedef basic_writer<back_insert_range<internal::buffer>> writer;
typedef basic_writer<back_insert_range<internal::wbuffer>> wwriter;

/** A formatting error such as invalid format string. */
class format_error : public std::runtime_error {
 public:
  explicit format_error(const char *message)
  : std::runtime_error(message) {}

  explicit format_error(const std::string &message)
  : std::runtime_error(message) {}
};

namespace internal {

#if FMT_SECURE_SCL
template <typename T>
struct checked { typedef stdext::checked_array_iterator<T*> type; };

// Make a checked iterator to avoid warnings on MSVC.
template <typename T>
inline stdext::checked_array_iterator<T*> make_checked(T *p, std::size_t size) {
  return {p, size};
}
#else
template <typename T>
struct checked { typedef T *type; };
template <typename T>
inline T *make_checked(T *p, std::size_t) { return p; }
#endif

template <typename T>
template <typename U>
void basic_buffer<T>::append(const U *begin, const U *end) {
  std::size_t new_size = size_ + internal::to_unsigned(end - begin);
  reserve(new_size);
  std::uninitialized_copy(begin, end,
                          internal::make_checked(ptr_, capacity_) + size_);
  size_ = new_size;
}
}  // namespace internal

// A UTF-8 code unit type.
struct char8_t {
  char value;
  FMT_CONSTEXPR explicit operator bool() const FMT_NOEXCEPT {
    return value != 0;
  }
};

// A UTF-8 string view.
class u8string_view : public basic_string_view<char8_t> {
 private:
  typedef basic_string_view<char8_t> base;

 public:
  using basic_string_view::basic_string_view;
  using basic_string_view::char_type;

  u8string_view(const char *s)
    : base(reinterpret_cast<const char8_t*>(s)) {}

  u8string_view(const char *s, size_t count) FMT_NOEXCEPT
    : base(reinterpret_cast<const char8_t*>(s), count) {}
};

#if FMT_USE_USER_DEFINED_LITERALS
inline namespace literals {
inline u8string_view operator"" _u(const char *s, std::size_t n) {
  return u8string_view(s, n);
}
}
#endif

// A wrapper around std::locale used to reduce compile times since <locale>
// is very heavy.
class locale;

class locale_provider {
 public:
  virtual ~locale_provider() {}
  virtual fmt::locale locale();
};

// The number of characters to store in the basic_memory_buffer object itself
// to avoid dynamic memory allocation.
enum { inline_buffer_size = 500 };

/**
  \rst
  A dynamically growing memory buffer for trivially copyable/constructible types
  with the first ``SIZE`` elements stored in the object itself.

  You can use one of the following typedefs for common character types:

  +----------------+------------------------------+
  | Type           | Definition                   |
  +================+==============================+
  | memory_buffer  | basic_memory_buffer<char>    |
  +----------------+------------------------------+
  | wmemory_buffer | basic_memory_buffer<wchar_t> |
  +----------------+------------------------------+

  **Example**::

     fmt::memory_buffer out;
     format_to(out, "The answer is {}.", 42);

  This will write the following output to the ``out`` object:

  .. code-block:: none

     The answer is 42.

  The output can be converted to an ``std::string`` with ``to_string(out)``.
  \endrst
 */
template <typename T, std::size_t SIZE = inline_buffer_size,
          typename Allocator = std::allocator<T> >
class basic_memory_buffer: private Allocator, public internal::basic_buffer<T> {
 private:
  T store_[SIZE];

  // Deallocate memory allocated by the buffer.
  void deallocate() {
    T* data = this->data();
    if (data != store_) Allocator::deallocate(data, this->capacity());
  }

 protected:
  void grow(std::size_t size) FMT_OVERRIDE;

 public:
  explicit basic_memory_buffer(const Allocator &alloc = Allocator())
      : Allocator(alloc) {
    this->set(store_, SIZE);
  }
  ~basic_memory_buffer() { deallocate(); }

 private:
  // Move data from other to this buffer.
  void move(basic_memory_buffer &other) {
    Allocator &this_alloc = *this, &other_alloc = other;
    this_alloc = std::move(other_alloc);
    T* data = other.data();
    std::size_t size = other.size(), capacity = other.capacity();
    if (data == other.store_) {
      this->set(store_, capacity);
      std::uninitialized_copy(other.store_, other.store_ + size,
                              internal::make_checked(store_, capacity));
    } else {
      this->set(data, capacity);
      // Set pointer to the inline array so that delete is not called
      // when deallocating.
      other.set(other.store_, 0);
    }
    this->resize(size);
  }

 public:
  /**
    \rst
    Constructs a :class:`fmt::basic_memory_buffer` object moving the content
    of the other object to it.
    \endrst
   */
  basic_memory_buffer(basic_memory_buffer &&other) {
    move(other);
  }

  /**
    \rst
    Moves the content of the other ``basic_memory_buffer`` object to this one.
    \endrst
   */
  basic_memory_buffer &operator=(basic_memory_buffer &&other) {
    assert(this != &other);
    deallocate();
    move(other);
    return *this;
  }

  // Returns a copy of the allocator associated with this buffer.
  Allocator get_allocator() const { return *this; }
};

template <typename T, std::size_t SIZE, typename Allocator>
void basic_memory_buffer<T, SIZE, Allocator>::grow(std::size_t size) {
  std::size_t old_capacity = this->capacity();
  std::size_t new_capacity = old_capacity + old_capacity / 2;
  if (size > new_capacity)
      new_capacity = size;
  T *old_data = this->data();
  T *new_data = internal::allocate<Allocator>(*this, new_capacity);
  // The following code doesn't throw, so the raw pointer above doesn't leak.
  std::uninitialized_copy(old_data, old_data + this->size(),
                          internal::make_checked(new_data, new_capacity));
  this->set(new_data, new_capacity);
  // deallocate must not throw according to the standard, but even if it does,
  // the buffer already uses the new storage and will deallocate it in
  // destructor.
  if (old_data != store_)
    Allocator::deallocate(old_data, old_capacity);
}

typedef basic_memory_buffer<char> memory_buffer;
typedef basic_memory_buffer<wchar_t> wmemory_buffer;

/**
  \rst
  A fixed-size memory buffer. For a dynamically growing buffer use
  :class:`fmt::basic_memory_buffer`.

  Trying to increase the buffer size past the initial capacity will throw
  ``std::runtime_error``.
  \endrst
 */
template <typename Char>
class basic_fixed_buffer : public internal::basic_buffer<Char> {
 public:
  /**
   \rst
   Constructs a :class:`fmt::basic_fixed_buffer` object for *array* of the
   given size.
   \endrst
   */
  basic_fixed_buffer(Char *array, std::size_t size) {
    this->set(array, size);
  }

  /**
   \rst
   Constructs a :class:`fmt::basic_fixed_buffer` object for *array* of the
   size known at compile time.
   \endrst
   */
  template <std::size_t SIZE>
  explicit basic_fixed_buffer(Char (&array)[SIZE]) {
    this->set(array, SIZE);
  }

 protected:
  FMT_API void grow(std::size_t size) FMT_OVERRIDE;
};

namespace internal {

template <typename Char>
struct char_traits;

template <>
struct char_traits<char> {
  // Formats a floating-point number.
  template <typename T>
  FMT_API static int format_float(char *buffer, std::size_t size,
      const char *format, int precision, T value);
};

template <>
struct char_traits<wchar_t> {
  template <typename T>
  FMT_API static int format_float(wchar_t *buffer, std::size_t size,
      const wchar_t *format, int precision, T value);
};

#if FMT_USE_EXTERN_TEMPLATES
extern template int char_traits<char>::format_float<double>(
    char *buffer, std::size_t size, const char* format, int precision,
    double value);
extern template int char_traits<char>::format_float<long double>(
    char *buffer, std::size_t size, const char* format, int precision,
    long double value);

extern template int char_traits<wchar_t>::format_float<double>(
    wchar_t *buffer, std::size_t size, const wchar_t* format, int precision,
    double value);
extern template int char_traits<wchar_t>::format_float<long double>(
    wchar_t *buffer, std::size_t size, const wchar_t* format, int precision,
    long double value);
#endif

template <typename Container>
inline typename std::enable_if<
  is_contiguous<Container>::value,
  typename checked<typename Container::value_type>::type>::type
    reserve(std::back_insert_iterator<Container> &it, std::size_t n) {
  Container &c = internal::get_container(it);
  std::size_t size = c.size();
  c.resize(size + n);
  return make_checked(&c[size], n);
}

template <typename Iterator>
inline Iterator &reserve(Iterator &it, std::size_t) { return it; }

template <typename Char>
class null_terminating_iterator;

template <typename Char>
FMT_CONSTEXPR_DECL const Char *pointer_from(null_terminating_iterator<Char> it);

// An iterator that produces a null terminator on *end. This simplifies parsing
// and allows comparing the performance of processing a null-terminated string
// vs string_view.
template <typename Char>
class null_terminating_iterator {
 public:
  typedef std::ptrdiff_t difference_type;
  typedef Char value_type;
  typedef const Char* pointer;
  typedef const Char& reference;
  typedef std::random_access_iterator_tag iterator_category;

  null_terminating_iterator() : ptr_(0), end_(0) {}

  FMT_CONSTEXPR null_terminating_iterator(const Char *ptr, const Char *end)
    : ptr_(ptr), end_(end) {}

  template <typename Range>
  FMT_CONSTEXPR explicit null_terminating_iterator(const Range &r)
    : ptr_(r.begin()), end_(r.end()) {}

  FMT_CONSTEXPR null_terminating_iterator &operator=(const Char *ptr) {
    assert(ptr <= end_);
    ptr_ = ptr;
    return *this;
  }

  FMT_CONSTEXPR Char operator*() const {
    return ptr_ != end_ ? *ptr_ : 0;
  }

  FMT_CONSTEXPR null_terminating_iterator operator++() {
    ++ptr_;
    return *this;
  }

  FMT_CONSTEXPR null_terminating_iterator operator++(int) {
    null_terminating_iterator result(*this);
    ++ptr_;
    return result;
  }

  FMT_CONSTEXPR null_terminating_iterator operator--() {
    --ptr_;
    return *this;
  }

  FMT_CONSTEXPR null_terminating_iterator operator+(difference_type n) {
    return null_terminating_iterator(ptr_ + n, end_);
  }

  FMT_CONSTEXPR null_terminating_iterator operator-(difference_type n) {
    return null_terminating_iterator(ptr_ - n, end_);
  }

  FMT_CONSTEXPR null_terminating_iterator operator+=(difference_type n) {
    ptr_ += n;
    return *this;
  }

  FMT_CONSTEXPR difference_type operator-(
      null_terminating_iterator other) const {
    return ptr_ - other.ptr_;
  }

  FMT_CONSTEXPR bool operator!=(null_terminating_iterator other) const {
    return ptr_ != other.ptr_;
  }

  bool operator>=(null_terminating_iterator other) const {
    return ptr_ >= other.ptr_;
  }

  // This should be a friend specialization pointer_from<Char> but the latter
  // doesn't compile by gcc 5.1 due to a compiler bug.
  template <typename CharT>
  friend FMT_CONSTEXPR_DECL const CharT *pointer_from(
      null_terminating_iterator<CharT> it);

 private:
  const Char *ptr_;
  const Char *end_;
};

template <typename T>
FMT_CONSTEXPR const T *pointer_from(const T *p) { return p; }

template <typename Char>
FMT_CONSTEXPR const Char *pointer_from(null_terminating_iterator<Char> it) {
  return it.ptr_;
}

// An output iterator that counts the number of objects written to it and
// discards them.
template <typename T>
class counting_iterator {
 private:
  std::size_t count_;
  mutable T blackhole_;

 public:
  typedef std::output_iterator_tag iterator_category;
  typedef T value_type;
  typedef std::ptrdiff_t difference_type;
  typedef T* pointer;
  typedef T& reference;
  typedef counting_iterator _Unchecked_type;  // Mark iterator as checked.

  counting_iterator(): count_(0) {}

  std::size_t count() const { return count_; }

  counting_iterator& operator++() {
    ++count_;
    return *this;
  }

  counting_iterator operator++(int) {
    auto it = *this;
    ++*this;
    return it;
  }

  T &operator*() const { return blackhole_; }
};

// An output iterator that truncates the output and counts the number of objects
// written to it.
template <typename OutputIt>
class truncating_iterator {
 private:
  typedef std::iterator_traits<OutputIt> traits;

  OutputIt out_;
  std::size_t limit_;
  std::size_t count_;
  mutable typename traits::value_type blackhole_;

 public:
  typedef std::output_iterator_tag iterator_category;
  typedef typename traits::value_type value_type;
  typedef typename traits::difference_type difference_type;
  typedef typename traits::pointer pointer;
  typedef typename traits::reference reference;
  typedef truncating_iterator _Unchecked_type;  // Mark iterator as checked.

  truncating_iterator(OutputIt out, std::size_t limit)
    : out_(out), limit_(limit), count_(0) {}

  OutputIt base() const { return out_; }
  std::size_t count() const { return count_; }

  truncating_iterator& operator++() {
    if (count_++ < limit_)
      ++out_;
    return *this;
  }

  truncating_iterator operator++(int) {
    auto it = *this;
    ++*this;
    return it;
  }

  reference operator*() const { return count_ < limit_ ? *out_ : blackhole_; }
};

// Returns true if value is negative, false otherwise.
// Same as (value < 0) but doesn't produce warnings if T is an unsigned type.
template <typename T>
FMT_CONSTEXPR typename std::enable_if<
    std::numeric_limits<T>::is_signed, bool>::type is_negative(T value) {
  return value < 0;
}
template <typename T>
FMT_CONSTEXPR typename std::enable_if<
    !std::numeric_limits<T>::is_signed, bool>::type is_negative(T) {
  return false;
}

template <typename T>
struct int_traits {
  // Smallest of uint32_t and uint64_t that is large enough to represent
  // all values of T.
  typedef typename std::conditional<
    std::numeric_limits<T>::digits <= 32, uint32_t, uint64_t>::type main_type;
};

// Static data is placed in this class template to allow header-only
// configuration.
template <typename T = void>
struct FMT_API basic_data {
  static const uint32_t POWERS_OF_10_32[];
  static const uint32_t ZERO_OR_POWERS_OF_10_32[];
  static const uint64_t ZERO_OR_POWERS_OF_10_64[];
  static const uint64_t POW10_SIGNIFICANDS[];
  static const int16_t POW10_EXPONENTS[];
  static const char DIGITS[];
  static const char RESET_COLOR[];
  static const wchar_t WRESET_COLOR[];
};

#if FMT_USE_EXTERN_TEMPLATES
extern template struct basic_data<void>;
#endif

typedef basic_data<> data;

#ifdef FMT_BUILTIN_CLZLL
// Returns the number of decimal digits in n. Leading zeros are not counted
// except for n == 0 in which case count_digits returns 1.
inline unsigned count_digits(uint64_t n) {
  // Based on http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
  // and the benchmark https://github.com/localvoid/cxx-benchmark-count-digits.
  int t = (64 - FMT_BUILTIN_CLZLL(n | 1)) * 1233 >> 12;
  return to_unsigned(t) - (n < data::ZERO_OR_POWERS_OF_10_64[t]) + 1;
}
#else
// Fallback version of count_digits used when __builtin_clz is not available.
inline unsigned count_digits(uint64_t n) {
  unsigned count = 1;
  for (;;) {
    // Integer division is slow so do it for a group of four digits instead
    // of for every digit. The idea comes from the talk by Alexandrescu
    // "Three Optimization Tips for C++". See speed-test for a comparison.
    if (n < 10) return count;
    if (n < 100) return count + 1;
    if (n < 1000) return count + 2;
    if (n < 10000) return count + 3;
    n /= 10000u;
    count += 4;
  }
}
#endif

// Counts the number of code points in a UTF-8 string.
FMT_API size_t count_code_points(u8string_view s);

#if FMT_HAS_CPP_ATTRIBUTE(always_inline)
# define FMT_ALWAYS_INLINE __attribute__((always_inline))
#else
# define FMT_ALWAYS_INLINE
#endif

template <typename Handler>
inline char *lg(uint32_t n, Handler h) FMT_ALWAYS_INLINE;

// Computes g = floor(log10(n)) and calls h.on<g>(n);
template <typename Handler>
inline char *lg(uint32_t n, Handler h) {
  return n < 100 ? n < 10 ? h.template on<0>(n) : h.template on<1>(n)
                 : n < 1000000
                       ? n < 10000 ? n < 1000 ? h.template on<2>(n)
                                              : h.template on<3>(n)
                                   : n < 100000 ? h.template on<4>(n)
                                                : h.template on<5>(n)
                       : n < 100000000 ? n < 10000000 ? h.template on<6>(n)
                                                      : h.template on<7>(n)
                                       : n < 1000000000 ? h.template on<8>(n)
                                                        : h.template on<9>(n);
}

// An lg handler that formats a decimal number.
// Usage: lg(n, decimal_formatter(buffer));
class decimal_formatter {
 private:
  char *buffer_;

  void write_pair(unsigned N, uint32_t index) {
    std::memcpy(buffer_ + N, data::DIGITS + index * 2, 2);
  }

 public:
  explicit decimal_formatter(char *buf) : buffer_(buf) {}

  template <unsigned N> char *on(uint32_t u) {
    if (N == 0) {
      *buffer_ = static_cast<char>(u) + '0';
    } else if (N == 1) {
      write_pair(0, u);
    } else {
      // The idea of using 4.32 fixed-point numbers is based on
      // https://github.com/jeaiii/itoa
      unsigned n = N - 1;
      unsigned a = n / 5 * n * 53 / 16;
      uint64_t t = ((1ULL << (32 + a)) /
                   data::ZERO_OR_POWERS_OF_10_32[n] + 1 - n / 9);
      t = ((t * u) >> a) + n / 5 * 4;
      write_pair(0, t >> 32);
      for (unsigned i = 2; i < N; i += 2) {
        t = 100ULL * static_cast<uint32_t>(t);
        write_pair(i, t >> 32);
      }
      if (N % 2 == 0) {
        buffer_[N] = static_cast<char>(
          (10ULL * static_cast<uint32_t>(t)) >> 32) + '0';
      }
    }
    return buffer_ += N + 1;
  }
};

// An lg handler that formats a decimal number with a terminating null.
class decimal_formatter_null : public decimal_formatter {
 public:
  explicit decimal_formatter_null(char *buf) : decimal_formatter(buf) {}

  template <unsigned N> char *on(uint32_t u) {
    char *buf = decimal_formatter::on<N>(u);
    *buf = '\0';
    return buf;
  }
};

#ifdef FMT_BUILTIN_CLZ
// Optional version of count_digits for better performance on 32-bit platforms.
inline unsigned count_digits(uint32_t n) {
  int t = (32 - FMT_BUILTIN_CLZ(n | 1)) * 1233 >> 12;
  return to_unsigned(t) - (n < data::ZERO_OR_POWERS_OF_10_32[t]) + 1;
}
#endif

// A functor that doesn't add a thousands separator.
struct no_thousands_sep {
  typedef char char_type;

  template <typename Char>
  void operator()(Char *) {}
};

// A functor that adds a thousands separator.
template <typename Char>
class add_thousands_sep {
 private:
  basic_string_view<Char> sep_;

  // Index of a decimal digit with the least significant digit having index 0.
  unsigned digit_index_;

 public:
  typedef Char char_type;

  explicit add_thousands_sep(basic_string_view<Char> sep)
    : sep_(sep), digit_index_(0) {}

  void operator()(Char *&buffer) {
    if (++digit_index_ % 3 != 0)
      return;
    buffer -= sep_.size();
    std::uninitialized_copy(sep_.data(), sep_.data() + sep_.size(),
                            internal::make_checked(buffer, sep_.size()));
  }
};

template <typename Char>
FMT_API Char thousands_sep(locale_provider *lp);

// Formats a decimal unsigned integer value writing into buffer.
// thousands_sep is a functor that is called after writing each char to
// add a thousands separator if necessary.
template <typename UInt, typename Char, typename ThousandsSep>
inline Char *format_decimal(Char *buffer, UInt value, unsigned num_digits,
                            ThousandsSep thousands_sep) {
  buffer += num_digits;
  Char *end = buffer;
  while (value >= 100) {
    // Integer division is slow so do it for a group of two digits instead
    // of for every digit. The idea comes from the talk by Alexandrescu
    // "Three Optimization Tips for C++". See speed-test for a comparison.
    unsigned index = static_cast<unsigned>((value % 100) * 2);
    value /= 100;
    *--buffer = data::DIGITS[index + 1];
    thousands_sep(buffer);
    *--buffer = data::DIGITS[index];
    thousands_sep(buffer);
  }
  if (value < 10) {
    *--buffer = static_cast<char>('0' + value);
    return end;
  }
  unsigned index = static_cast<unsigned>(value * 2);
  *--buffer = data::DIGITS[index + 1];
  thousands_sep(buffer);
  *--buffer = data::DIGITS[index];
  return end;
}

template <typename UInt, typename Iterator, typename ThousandsSep>
inline Iterator format_decimal(
    Iterator out, UInt value, unsigned num_digits, ThousandsSep sep) {
  typedef typename ThousandsSep::char_type char_type;
  // Buffer should be large enough to hold all digits (digits10 + 1) and null.
  char_type buffer[std::numeric_limits<UInt>::digits10 + 2];
  format_decimal(buffer, value, num_digits, sep);
  return std::copy_n(buffer, num_digits, out);
}

template <typename It, typename UInt>
inline It format_decimal(It out, UInt value, unsigned num_digits) {
  return format_decimal(out, value, num_digits, no_thousands_sep());
}

template <unsigned BASE_BITS, typename Char, typename UInt>
inline Char *format_uint(Char *buffer, UInt value, unsigned num_digits,
                         bool upper = false) {
  buffer += num_digits;
  Char *end = buffer;
  do {
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    unsigned digit = (value & ((1 << BASE_BITS) - 1));
    *--buffer = BASE_BITS < 4 ? static_cast<char>('0' + digit) : digits[digit];
  } while ((value >>= BASE_BITS) != 0);
  return end;
}

template <unsigned BASE_BITS, typename It, typename UInt>
inline It format_uint(It out, UInt value, unsigned num_digits,
                      bool upper = false) {
  // Buffer should be large enough to hold all digits (digits / BASE_BITS + 1)
  // and null.
  char buffer[std::numeric_limits<UInt>::digits / BASE_BITS + 2];
  format_uint<BASE_BITS>(buffer, value, num_digits, upper);
  return std::copy_n(buffer, num_digits, out);
}

#ifndef _WIN32
# define FMT_USE_WINDOWS_H 0
#elif !defined(FMT_USE_WINDOWS_H)
# define FMT_USE_WINDOWS_H 1
#endif

// Define FMT_USE_WINDOWS_H to 0 to disable use of windows.h.
// All the functionality that relies on it will be disabled too.
#if FMT_USE_WINDOWS_H
// A converter from UTF-8 to UTF-16.
// It is only provided for Windows since other systems support UTF-8 natively.
class utf8_to_utf16 {
 private:
  wmemory_buffer buffer_;

 public:
  FMT_API explicit utf8_to_utf16(string_view s);
  operator wstring_view() const { return wstring_view(&buffer_[0], size()); }
  size_t size() const { return buffer_.size() - 1; }
  const wchar_t *c_str() const { return &buffer_[0]; }
  std::wstring str() const { return std::wstring(&buffer_[0], size()); }
};

// A converter from UTF-16 to UTF-8.
// It is only provided for Windows since other systems support UTF-8 natively.
class utf16_to_utf8 {
 private:
  memory_buffer buffer_;

 public:
  utf16_to_utf8() {}
  FMT_API explicit utf16_to_utf8(wstring_view s);
  operator string_view() const { return string_view(&buffer_[0], size()); }
  size_t size() const { return buffer_.size() - 1; }
  const char *c_str() const { return &buffer_[0]; }
  std::string str() const { return std::string(&buffer_[0], size()); }

  // Performs conversion returning a system error code instead of
  // throwing exception on conversion error. This method may still throw
  // in case of memory allocation error.
  FMT_API int convert(wstring_view s);
};

FMT_API void format_windows_error(fmt::internal::buffer &out, int error_code,
                                  fmt::string_view message) FMT_NOEXCEPT;
#endif

template <typename T = void>
struct null {};
}  // namespace internal

enum alignment {
  ALIGN_DEFAULT, ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER, ALIGN_NUMERIC
};

// Flags.
enum {SIGN_FLAG = 1, PLUS_FLAG = 2, MINUS_FLAG = 4, HASH_FLAG = 8};

enum format_spec_tag {fill_tag, align_tag, width_tag, type_tag};

// Format specifier.
template <typename T, format_spec_tag>
class format_spec {
 private:
  T value_;

 public:
  typedef T value_type;

  explicit format_spec(T value) : value_(value) {}

  T value() const { return value_; }
};

// template <typename Char>
// typedef format_spec<Char, fill_tag> fill_spec;
template <typename Char>
class fill_spec : public format_spec<Char, fill_tag> {
 public:
  explicit fill_spec(Char value) : format_spec<Char, fill_tag>(value) {}
};

typedef format_spec<unsigned, width_tag> width_spec;
typedef format_spec<char, type_tag> type_spec;

// An empty format specifier.
struct empty_spec {};

// An alignment specifier.
struct align_spec : empty_spec {
  unsigned width_;
  // Fill is always wchar_t and cast to char if necessary to avoid having
  // two specialization of AlignSpec and its subclasses.
  wchar_t fill_;
  alignment align_;

  FMT_CONSTEXPR align_spec(
      unsigned width, wchar_t fill, alignment align = ALIGN_DEFAULT)
  : width_(width), fill_(fill), align_(align) {}

  FMT_CONSTEXPR unsigned width() const { return width_; }
  FMT_CONSTEXPR wchar_t fill() const { return fill_; }
  FMT_CONSTEXPR alignment align() const { return align_; }

  int precision() const { return -1; }
};

// Format specifiers.
template <typename Char>
class basic_format_specs : public align_spec {
 public:
  unsigned flags_;
  int precision_;
  Char type_;

  FMT_CONSTEXPR basic_format_specs(
      unsigned width = 0, char type = 0, wchar_t fill = ' ')
  : align_spec(width, fill), flags_(0), precision_(-1), type_(type) {}

  FMT_CONSTEXPR bool flag(unsigned f) const { return (flags_ & f) != 0; }
  FMT_CONSTEXPR int precision() const { return precision_; }
  FMT_CONSTEXPR Char type() const { return type_; }
};

typedef basic_format_specs<char> format_specs;

template <typename Char, typename ErrorHandler>
FMT_CONSTEXPR unsigned basic_parse_context<Char, ErrorHandler>::next_arg_id() {
  if (next_arg_id_ >= 0)
    return internal::to_unsigned(next_arg_id_++);
  on_error("cannot switch from manual to automatic argument indexing");
  return 0;
}

namespace internal {

template <typename S>
struct format_string_traits<
  S, typename std::enable_if<std::is_base_of<compile_string, S>::value>::type>:
    format_string_traits_base<char> {};

template <typename Char, typename Handler>
FMT_CONSTEXPR void handle_int_type_spec(Char spec, Handler &&handler) {
  switch (spec) {
  case 0: case 'd':
    handler.on_dec();
    break;
  case 'x': case 'X':
    handler.on_hex();
    break;
  case 'b': case 'B':
    handler.on_bin();
    break;
  case 'o':
    handler.on_oct();
    break;
  case 'n':
    handler.on_num();
    break;
  default:
    handler.on_error();
  }
}

template <typename Char, typename Handler>
FMT_CONSTEXPR void handle_float_type_spec(Char spec, Handler &&handler) {
  switch (spec) {
  case 0: case 'g': case 'G':
    handler.on_general();
    break;
  case 'e': case 'E':
    handler.on_exp();
    break;
  case 'f': case 'F':
    handler.on_fixed();
    break;
   case 'a': case 'A':
    handler.on_hex();
    break;
  default:
    handler.on_error();
    break;
  }
}

template <typename Char, typename Handler>
FMT_CONSTEXPR void handle_char_specs(
    const basic_format_specs<Char> *specs, Handler &&handler) {
  if (!specs) return handler.on_char();
  if (specs->type() && specs->type() != 'c') return handler.on_int();
  if (specs->align() == ALIGN_NUMERIC || specs->flag(~0u) != 0)
    handler.on_error("invalid format specifier for char");
  handler.on_char();
}

template <typename Char, typename Handler>
FMT_CONSTEXPR void handle_cstring_type_spec(Char spec, Handler &&handler) {
  if (spec == 0 || spec == 's')
    handler.on_string();
  else if (spec == 'p')
    handler.on_pointer();
  else
    handler.on_error("invalid type specifier");
}

template <typename Char, typename ErrorHandler>
FMT_CONSTEXPR void check_string_type_spec(Char spec, ErrorHandler &&eh) {
  if (spec != 0 && spec != 's')
    eh.on_error("invalid type specifier");
}

template <typename Char, typename ErrorHandler>
FMT_CONSTEXPR void check_pointer_type_spec(Char spec, ErrorHandler &&eh) {
  if (spec != 0 && spec != 'p')
    eh.on_error("invalid type specifier");
}

template <typename ErrorHandler>
class int_type_checker : private ErrorHandler {
 public:
  FMT_CONSTEXPR explicit int_type_checker(ErrorHandler eh) : ErrorHandler(eh) {}

  FMT_CONSTEXPR void on_dec() {}
  FMT_CONSTEXPR void on_hex() {}
  FMT_CONSTEXPR void on_bin() {}
  FMT_CONSTEXPR void on_oct() {}
  FMT_CONSTEXPR void on_num() {}

  FMT_CONSTEXPR void on_error() {
    ErrorHandler::on_error("invalid type specifier");
  }
};

template <typename ErrorHandler>
class float_type_checker : private ErrorHandler {
 public:
  FMT_CONSTEXPR explicit float_type_checker(ErrorHandler eh)
    : ErrorHandler(eh) {}

  FMT_CONSTEXPR void on_general() {}
  FMT_CONSTEXPR void on_exp() {}
  FMT_CONSTEXPR void on_fixed() {}
  FMT_CONSTEXPR void on_hex() {}

  FMT_CONSTEXPR void on_error() {
    ErrorHandler::on_error("invalid type specifier");
  }
};

template <typename ErrorHandler, typename CharType>
class char_specs_checker : public ErrorHandler {
 private:
  CharType type_;

 public:
  FMT_CONSTEXPR char_specs_checker(CharType type, ErrorHandler eh)
    : ErrorHandler(eh), type_(type) {}

  FMT_CONSTEXPR void on_int() {
    handle_int_type_spec(type_, int_type_checker<ErrorHandler>(*this));
  }
  FMT_CONSTEXPR void on_char() {}
};

template <typename ErrorHandler>
class cstring_type_checker : public ErrorHandler {
 public:
  FMT_CONSTEXPR explicit cstring_type_checker(ErrorHandler eh)
    : ErrorHandler(eh) {}

  FMT_CONSTEXPR void on_string() {}
  FMT_CONSTEXPR void on_pointer() {}
};

template <typename Context>
void arg_map<Context>::init(const basic_format_args<Context> &args) {
  if (map_)
    return;
  map_ = new entry[args.max_size()];
  bool use_values = args.type(max_packed_args - 1) == internal::none_type;
  if (use_values) {
    for (unsigned i = 0;/*nothing*/; ++i) {
      internal::type arg_type = args.type(i);
      switch (arg_type) {
        case internal::none_type:
          return;
        case internal::named_arg_type:
          push_back(args.values_[i]);
          break;
        default:
          break; // Do nothing.
      }
    }
  }
  for (unsigned i = 0; ; ++i) {
    switch (args.args_[i].type_) {
      case internal::none_type:
        return;
      case internal::named_arg_type:
        push_back(args.args_[i].value_);
        break;
      default:
        break; // Do nothing.
    }
  }
}

template <typename Range>
class arg_formatter_base {
 public:
  typedef typename Range::value_type char_type;
  typedef decltype(internal::declval<Range>().begin()) iterator;
  typedef basic_format_specs<char_type> format_specs;

 private:
  typedef basic_writer<Range> writer_type;
  writer_type writer_;
  format_specs *specs_;

  struct char_writer {
    char_type value;
    template <typename It>
    void operator()(It &&it) const { *it++ = value; }
  };

  void write_char(char_type value) {
    if (specs_)
      writer_.write_padded(1, *specs_, char_writer{value});
    else
      writer_.write(value);
  }

  void write_pointer(const void *p) {
    format_specs specs = specs_ ? *specs_ : format_specs();
    specs.flags_ = HASH_FLAG;
    specs.type_ = 'x';
    writer_.write_int(reinterpret_cast<uintptr_t>(p), specs);
  }

 protected:
  writer_type &writer() { return writer_; }
  format_specs *spec() { return specs_; }
  iterator out() { return writer_.out(); }

  void write(bool value) {
    string_view sv(value ? "true" : "false");
    specs_ ? writer_.write_str(sv, *specs_) : writer_.write(sv);
  }

  void write(const char_type *value) {
    if (!value)
      FMT_THROW(format_error("string pointer is null"));
    auto length = std::char_traits<char_type>::length(value);
    basic_string_view<char_type> sv(value, length);
    specs_ ? writer_.write_str(sv, *specs_) : writer_.write(sv);
  }

 public:
  arg_formatter_base(Range r, format_specs *s): writer_(r), specs_(s) {}

  iterator operator()(monostate) {
    FMT_ASSERT(false, "invalid argument type");
    return out();
  }

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value, iterator>::type
      operator()(T value) {
    // MSVC2013 fails to compile separate overloads for bool and char_type so
    // use std::is_same instead.
    if (std::is_same<T, bool>::value) {
      if (specs_ && specs_->type_)
        return (*this)(value ? 1 : 0);
      write(value != 0);
    } else if (std::is_same<T, char_type>::value) {
      internal::handle_char_specs(
        specs_, char_spec_handler(*this, static_cast<char_type>(value)));
    } else {
      specs_ ? writer_.write_int(value, *specs_) : writer_.write(value);
    }
    return out();
  }

  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value, iterator>::type
      operator()(T value) {
    writer_.write_double(value, specs_ ? *specs_ : format_specs());
    return out();
  }

  struct char_spec_handler : internal::error_handler {
    arg_formatter_base &formatter;
    char_type value;

    char_spec_handler(arg_formatter_base& f, char_type val)
      : formatter(f), value(val) {}

    void on_int() {
      if (formatter.specs_)
        formatter.writer_.write_int(value, *formatter.specs_);
      else
        formatter.writer_.write(value);
    }
    void on_char() { formatter.write_char(value); }
  };

  struct cstring_spec_handler : internal::error_handler {
    arg_formatter_base &formatter;
    const char_type *value;

    cstring_spec_handler(arg_formatter_base &f, const char_type *val)
      : formatter(f), value(val) {}

    void on_string() { formatter.write(value); }
    void on_pointer() { formatter.write_pointer(value); }
  };

  iterator operator()(const char_type *value) {
    if (!specs_) return write(value), out();
    internal::handle_cstring_type_spec(
          specs_->type_, cstring_spec_handler(*this, value));
    return out();
  }

  iterator operator()(basic_string_view<char_type> value) {
    if (specs_) {
      internal::check_string_type_spec(
            specs_->type_, internal::error_handler());
      writer_.write_str(value, *specs_);
    } else {
      writer_.write(value);
    }
    return out();
  }

  iterator operator()(const void *value) {
    if (specs_)
      check_pointer_type_spec(specs_->type_, internal::error_handler());
    write_pointer(value);
    return out();
  }
};

template <typename Char>
FMT_CONSTEXPR bool is_name_start(Char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || '_' == c;
}

// DEPRECATED: Parses the input as an unsigned integer. This function assumes
// that the first character is a digit and presence of a non-digit character at
// the end.
// it: an iterator pointing to the beginning of the input range.
template <typename Iterator, typename ErrorHandler>
FMT_CONSTEXPR unsigned parse_nonnegative_int(Iterator &it, ErrorHandler &&eh) {
  assert('0' <= *it && *it <= '9');
  unsigned value = 0;
  // Convert to unsigned to prevent a warning.
  unsigned max_int = (std::numeric_limits<int>::max)();
  unsigned big = max_int / 10;
  do {
    // Check for overflow.
    if (value > big) {
      value = max_int + 1;
      break;
    }
    value = value * 10 + unsigned(*it - '0');
    // Workaround for MSVC "setup_exception stack overflow" error:
    auto next = it;
    ++next;
    it = next;
  } while ('0' <= *it && *it <= '9');
  if (value > max_int)
    eh.on_error("number is too big");
  return value;
}

// Parses the range [begin, end) as an unsigned integer. This function assumes
// that the range is non-empty and the first character is a digit.
template <typename Char, typename ErrorHandler>
FMT_CONSTEXPR unsigned parse_nonnegative_int(
    const Char *&begin, const Char *end, ErrorHandler &&eh) {
  assert(begin != end && '0' <= *begin && *begin <= '9');
  unsigned value = 0;
  // Convert to unsigned to prevent a warning.
  unsigned max_int = (std::numeric_limits<int>::max)();
  unsigned big = max_int / 10;
  do {
    // Check for overflow.
    if (value > big) {
      value = max_int + 1;
      break;
    }
    value = value * 10 + unsigned(*begin++ - '0');
  } while (begin != end && '0' <= *begin && *begin <= '9');
  if (value > max_int)
    eh.on_error("number is too big");
  return value;
}

template <typename Char, typename Context>
class custom_formatter: public function<bool> {
 private:
  Context &ctx_;

 public:
  explicit custom_formatter(Context &ctx): ctx_(ctx) {}

  bool operator()(typename basic_format_arg<Context>::handle h) const {
    h.format(ctx_);
    return true;
  }

  template <typename T>
  bool operator()(T) const { return false; }
};

template <typename T>
struct is_integer {
  enum {
    value = std::is_integral<T>::value && !std::is_same<T, bool>::value &&
            !std::is_same<T, char>::value && !std::is_same<T, wchar_t>::value
  };
};

template <typename ErrorHandler>
class width_checker: public function<unsigned long long> {
 public:
  explicit FMT_CONSTEXPR width_checker(ErrorHandler &eh) : handler_(eh) {}

  template <typename T>
  FMT_CONSTEXPR
  typename std::enable_if<
      is_integer<T>::value, unsigned long long>::type operator()(T value) {
    if (is_negative(value))
      handler_.on_error("negative width");
    return static_cast<unsigned long long>(value);
  }

  template <typename T>
  FMT_CONSTEXPR typename std::enable_if<
      !is_integer<T>::value, unsigned long long>::type operator()(T) {
    handler_.on_error("width is not integer");
    return 0;
  }

 private:
  ErrorHandler &handler_;
};

template <typename ErrorHandler>
class precision_checker: public function<unsigned long long> {
 public:
  explicit FMT_CONSTEXPR precision_checker(ErrorHandler &eh) : handler_(eh) {}

  template <typename T>
  FMT_CONSTEXPR typename std::enable_if<
      is_integer<T>::value, unsigned long long>::type operator()(T value) {
    if (is_negative(value))
      handler_.on_error("negative precision");
    return static_cast<unsigned long long>(value);
  }

  template <typename T>
  FMT_CONSTEXPR typename std::enable_if<
      !is_integer<T>::value, unsigned long long>::type operator()(T) {
    handler_.on_error("precision is not integer");
    return 0;
  }

 private:
  ErrorHandler &handler_;
};

// A format specifier handler that sets fields in basic_format_specs.
template <typename Char>
class specs_setter {
 public:
  explicit FMT_CONSTEXPR specs_setter(basic_format_specs<Char> &specs):
    specs_(specs) {}

  FMT_CONSTEXPR specs_setter(const specs_setter &other) : specs_(other.specs_) {}

  FMT_CONSTEXPR void on_align(alignment align) { specs_.align_ = align; }
  FMT_CONSTEXPR void on_fill(Char fill) { specs_.fill_ = fill; }
  FMT_CONSTEXPR void on_plus() { specs_.flags_ |= SIGN_FLAG | PLUS_FLAG; }
  FMT_CONSTEXPR void on_minus() { specs_.flags_ |= MINUS_FLAG; }
  FMT_CONSTEXPR void on_space() { specs_.flags_ |= SIGN_FLAG; }
  FMT_CONSTEXPR void on_hash() { specs_.flags_ |= HASH_FLAG; }

  FMT_CONSTEXPR void on_zero() {
    specs_.align_ = ALIGN_NUMERIC;
    specs_.fill_ = '0';
  }

  FMT_CONSTEXPR void on_width(unsigned width) { specs_.width_ = width; }
  FMT_CONSTEXPR void on_precision(unsigned precision) {
    specs_.precision_ = static_cast<int>(precision);
  }
  FMT_CONSTEXPR void end_precision() {}

  FMT_CONSTEXPR void on_type(Char type) { specs_.type_ = type; }

 protected:
  basic_format_specs<Char> &specs_;
};

// A format specifier handler that checks if specifiers are consistent with the
// argument type.
template <typename Handler>
class specs_checker : public Handler {
 public:
  FMT_CONSTEXPR specs_checker(const Handler& handler, internal::type arg_type)
    : Handler(handler), arg_type_(arg_type) {}

  FMT_CONSTEXPR specs_checker(const specs_checker &other)
    : Handler(other), arg_type_(other.arg_type_) {}

  FMT_CONSTEXPR void on_align(alignment align) {
    if (align == ALIGN_NUMERIC)
      require_numeric_argument();
    Handler::on_align(align);
  }

  FMT_CONSTEXPR void on_plus() {
    check_sign();
    Handler::on_plus();
  }

  FMT_CONSTEXPR void on_minus() {
    check_sign();
    Handler::on_minus();
  }

  FMT_CONSTEXPR void on_space() {
    check_sign();
    Handler::on_space();
  }

  FMT_CONSTEXPR void on_hash() {
    require_numeric_argument();
    Handler::on_hash();
  }

  FMT_CONSTEXPR void on_zero() {
    require_numeric_argument();
    Handler::on_zero();
  }

  FMT_CONSTEXPR void end_precision() {
    if (is_integral(arg_type_) || arg_type_ == pointer_type)
      this->on_error("precision not allowed for this argument type");
  }

 private:
  FMT_CONSTEXPR void require_numeric_argument() {
    if (!is_arithmetic(arg_type_))
      this->on_error("format specifier requires numeric argument");
  }

  FMT_CONSTEXPR void check_sign() {
    require_numeric_argument();
    if (is_integral(arg_type_) && arg_type_ != int_type &&
        arg_type_ != long_long_type && arg_type_ != internal::char_type) {
      this->on_error("format specifier requires signed argument");
    }
  }

  internal::type arg_type_;
};

template <template <typename> class Handler, typename T,
          typename Context, typename ErrorHandler>
FMT_CONSTEXPR void set_dynamic_spec(
    T &value, basic_format_arg<Context> arg, ErrorHandler eh) {
  unsigned long long big_value = fmt::visit(Handler<ErrorHandler>(eh), arg);
  if (big_value > (std::numeric_limits<int>::max)())
    eh.on_error("number is too big");
  value = static_cast<T>(big_value);
}

struct auto_id {};

// The standard format specifier handler with checking.
template <typename Context>
class specs_handler: public specs_setter<typename Context::char_type> {
 public:
  typedef typename Context::char_type char_type;

  FMT_CONSTEXPR specs_handler(
      basic_format_specs<char_type> &specs, Context &ctx)
    : specs_setter<char_type>(specs), context_(ctx) {}

  template <typename Id>
  FMT_CONSTEXPR void on_dynamic_width(Id arg_id) {
    set_dynamic_spec<width_checker>(
          this->specs_.width_, get_arg(arg_id), context_.error_handler());
  }

  template <typename Id>
  FMT_CONSTEXPR void on_dynamic_precision(Id arg_id) {
    set_dynamic_spec<precision_checker>(
          this->specs_.precision_, get_arg(arg_id), context_.error_handler());
  }

  void on_error(const char *message) {
    context_.on_error(message);
  }

 private:
  FMT_CONSTEXPR basic_format_arg<Context> get_arg(auto_id) {
    return context_.next_arg();
  }

  template <typename Id>
  FMT_CONSTEXPR basic_format_arg<Context> get_arg(Id arg_id) {
    context_.parse_context().check_arg_id(arg_id);
    return context_.get_arg(arg_id);
  }

  Context &context_;
};

// An argument reference.
template <typename Char>
struct arg_ref {
  enum Kind { NONE, INDEX, NAME };

  FMT_CONSTEXPR arg_ref() : kind(NONE), index(0) {}
  FMT_CONSTEXPR explicit arg_ref(unsigned index) : kind(INDEX), index(index) {}
  explicit arg_ref(basic_string_view<Char> name) : kind(NAME), name(name) {}

  FMT_CONSTEXPR arg_ref &operator=(unsigned idx) {
    kind = INDEX;
    index = idx;
    return *this;
  }

  Kind kind;
  FMT_UNION {
    unsigned index;
    basic_string_view<Char> name;
  };
};

// Format specifiers with width and precision resolved at formatting rather
// than parsing time to allow re-using the same parsed specifiers with
// differents sets of arguments (precompilation of format strings).
template <typename Char>
struct dynamic_format_specs : basic_format_specs<Char> {
  arg_ref<Char> width_ref;
  arg_ref<Char> precision_ref;
};

// Format spec handler that saves references to arguments representing dynamic
// width and precision to be resolved at formatting time.
template <typename ParseContext>
class dynamic_specs_handler :
    public specs_setter<typename ParseContext::char_type> {
 public:
  typedef typename ParseContext::char_type char_type;

  FMT_CONSTEXPR dynamic_specs_handler(
      dynamic_format_specs<char_type> &specs, ParseContext &ctx)
    : specs_setter<char_type>(specs), specs_(specs), context_(ctx) {}

  FMT_CONSTEXPR dynamic_specs_handler(const dynamic_specs_handler &other)
    : specs_setter<char_type>(other),
      specs_(other.specs_), context_(other.context_) {}

  template <typename Id>
  FMT_CONSTEXPR void on_dynamic_width(Id arg_id) {
    specs_.width_ref = make_arg_ref(arg_id);
  }

  template <typename Id>
  FMT_CONSTEXPR void on_dynamic_precision(Id arg_id) {
    specs_.precision_ref = make_arg_ref(arg_id);
  }

  FMT_CONSTEXPR void on_error(const char *message) {
    context_.on_error(message);
  }

 private:
  typedef arg_ref<char_type> arg_ref_type;

  template <typename Id>
  FMT_CONSTEXPR arg_ref_type make_arg_ref(Id arg_id) {
    context_.check_arg_id(arg_id);
    return arg_ref_type(arg_id);
  }

  FMT_CONSTEXPR arg_ref_type make_arg_ref(auto_id) {
    return arg_ref_type(context_.next_arg_id());
  }

  dynamic_format_specs<char_type> &specs_;
  ParseContext &context_;
};

template <typename Iterator, typename IDHandler>
FMT_CONSTEXPR Iterator parse_arg_id(Iterator it, IDHandler &&handler) {
  typedef typename std::iterator_traits<Iterator>::value_type char_type;
  char_type c = *it;
  if (c == '}' || c == ':') {
    handler();
    return it;
  }
  if (c >= '0' && c <= '9') {
    unsigned index = parse_nonnegative_int(it, handler);
    if (*it != '}' && *it != ':') {
      handler.on_error("invalid format string");
      return it;
    }
    handler(index);
    return it;
  }
  if (!is_name_start(c)) {
    handler.on_error("invalid format string");
    return it;
  }
  auto start = it;
  do {
    c = *++it;
  } while (is_name_start(c) || ('0' <= c && c <= '9'));
  handler(basic_string_view<char_type>(
            pointer_from(start), to_unsigned(it - start)));
  return it;
}

template <typename Char, typename IDHandler>
FMT_CONSTEXPR const Char *parse_arg_id(
    const Char *begin, const Char *end, IDHandler &&handler) {
  assert(begin != end);
  Char c = *begin;
  if (c == '}' || c == ':')
    return handler(), begin;
  if (c >= '0' && c <= '9') {
    unsigned index = parse_nonnegative_int(begin, end, handler);
    if (begin == end || (*begin != '}' && *begin != ':'))
      return handler.on_error("invalid format string"), begin;
    handler(index);
    return begin;
  }
  if (!is_name_start(c))
    return handler.on_error("invalid format string"), begin;
  auto it = begin;
  do {
    c = *++it;
  } while (it != end && (is_name_start(c) || ('0' <= c && c <= '9')));
  handler(basic_string_view<Char>(begin, to_unsigned(it - begin)));
  return it;
}

// Adapts SpecHandler to IDHandler API for dynamic width.
template <typename SpecHandler, typename Char>
struct width_adapter {
  explicit FMT_CONSTEXPR width_adapter(SpecHandler &h) : handler(h) {}

  FMT_CONSTEXPR void operator()() { handler.on_dynamic_width(auto_id()); }
  FMT_CONSTEXPR void operator()(unsigned id) { handler.on_dynamic_width(id); }
  FMT_CONSTEXPR void operator()(basic_string_view<Char> id) {
    handler.on_dynamic_width(id);
  }

  FMT_CONSTEXPR void on_error(const char *message) {
    handler.on_error(message);
  }

  SpecHandler &handler;
};

// Adapts SpecHandler to IDHandler API for dynamic precision.
template <typename SpecHandler, typename Char>
struct precision_adapter {
  explicit FMT_CONSTEXPR precision_adapter(SpecHandler &h) : handler(h) {}

  FMT_CONSTEXPR void operator()() { handler.on_dynamic_precision(auto_id()); }
  FMT_CONSTEXPR void operator()(unsigned id) {
    handler.on_dynamic_precision(id);
  }
  FMT_CONSTEXPR void operator()(basic_string_view<Char> id) {
    handler.on_dynamic_precision(id);
  }

  FMT_CONSTEXPR void on_error(const char *message) { handler.on_error(message); }

  SpecHandler &handler;
};

// Parses standard format specifiers and sends notifications about parsed
// components to handler.
// it: an iterator pointing to the beginning of a null-terminated range of
//     characters, possibly emulated via null_terminating_iterator, representing
//     format specifiers.
template <typename Iterator, typename SpecHandler>
FMT_CONSTEXPR Iterator parse_format_specs(Iterator it, SpecHandler &&handler) {
  typedef typename std::iterator_traits<Iterator>::value_type char_type;
  char_type c = *it;
  if (c == '}' || !c)
    return it;

  // Parse fill and alignment.
  alignment align = ALIGN_DEFAULT;
  int i = 1;
  do {
    auto p = it + i;
    switch (*p) {
      case '<':
        align = ALIGN_LEFT;
        break;
      case '>':
        align = ALIGN_RIGHT;
        break;
      case '=':
        align = ALIGN_NUMERIC;
        break;
      case '^':
        align = ALIGN_CENTER;
        break;
    }
    if (align != ALIGN_DEFAULT) {
      if (p != it) {
        if (c == '{') {
          handler.on_error("invalid fill character '{'");
          return it;
        }
        it += 2;
        handler.on_fill(c);
      } else ++it;
      handler.on_align(align);
      break;
    }
  } while (--i >= 0);

  // Parse sign.
  switch (*it) {
    case '+':
      handler.on_plus();
      ++it;
      break;
    case '-':
      handler.on_minus();
      ++it;
      break;
    case ' ':
      handler.on_space();
      ++it;
      break;
  }

  if (*it == '#') {
    handler.on_hash();
    ++it;
  }

  // Parse zero flag.
  if (*it == '0') {
    handler.on_zero();
    ++it;
  }

  // Parse width.
  if ('0' <= *it && *it <= '9') {
    handler.on_width(parse_nonnegative_int(it, handler));
  } else if (*it == '{') {
    it = parse_arg_id(it + 1, width_adapter<SpecHandler, char_type>(handler));
    if (*it++ != '}') {
      handler.on_error("invalid format string");
      return it;
    }
  }

  // Parse precision.
  if (*it == '.') {
    ++it;
    if ('0' <= *it && *it <= '9') {
      handler.on_precision(parse_nonnegative_int(it, handler));
    } else if (*it == '{') {
      it = parse_arg_id(
            it + 1, precision_adapter<SpecHandler, char_type>(handler));
      if (*it++ != '}') {
        handler.on_error("invalid format string");
        return it;
      }
    } else {
      handler.on_error("missing precision specifier");
      return it;
    }
    handler.end_precision();
  }

  // Parse type.
  if (*it != '}' && *it)
    handler.on_type(*it++);
  return it;
}

// Return the result via the out param to workaround gcc bug 77539.
template <bool IS_CONSTEXPR, typename T, typename Ptr = const T*>
FMT_CONSTEXPR bool find(Ptr first, Ptr last, T value, Ptr &out) {
  for (out = first; out != last; ++out) {
    if (*out == value)
      return true;
  }
  return false;
}

template <>
inline bool find<false, char>(
    const char *first, const char *last, char value, const char *&out) {
  out = static_cast<const char*>(std::memchr(first, value, last - first));
  return out != FMT_NULL;
}

template <typename Handler, typename Char>
struct id_adapter {
  FMT_CONSTEXPR void operator()() { handler.on_arg_id(); }
  FMT_CONSTEXPR void operator()(unsigned id) { handler.on_arg_id(id); }
  FMT_CONSTEXPR void operator()(basic_string_view<Char> id) {
    handler.on_arg_id(id);
  }
  FMT_CONSTEXPR void on_error(const char *message) {
    handler.on_error(message);
  }
  Handler &handler;
};

template <bool IS_CONSTEXPR, typename Char, typename Handler>
FMT_CONSTEXPR void parse_format_string(
        basic_string_view<Char> format_str, Handler &&handler) {
  struct writer {
    FMT_CONSTEXPR void operator()(const Char *begin, const Char *end) {
      if (begin == end) return;
      for (;;) {
        const Char *p = FMT_NULL;
        if (!find<IS_CONSTEXPR>(begin, end, '}', p))
          return handler_.on_text(begin, end);
        ++p;
        if (p == end || *p != '}')
          return handler_.on_error("unmatched '}' in format string");
        handler_.on_text(begin, p);
        begin = p + 1;
      }
    }
    Handler &handler_;
  } write{handler};
  auto begin = format_str.data(), end = begin + format_str.size();
  while (begin != end) {
    // Doing two passes with memchr (one for '{' and another for '}') is up to
    // 2.5x faster than the naive one-pass implementation on big format strings.
    const Char *p = begin;
    if (*begin != '{' && !find<IS_CONSTEXPR>(begin, end, '{', p))
      return write(begin, end);
    write(begin, p);
    ++p;
    if (p == end)
      return handler.on_error("invalid format string");
    if (*p == '}') {
      handler.on_arg_id();
      handler.on_replacement_field(p);
    } else if (*p == '{') {
      handler.on_text(p, p + 1);
    } else {
      p = parse_arg_id(p, end, id_adapter<Handler, Char>{handler});
      Char c = p != end ? *p : 0;
      if (c == '}') {
        handler.on_replacement_field(p);
      } else if (c == ':') {
        internal::null_terminating_iterator<Char> it(p + 1, end);
        it = handler.on_format_specs(it);
        if (*it != '}')
          return handler.on_error("unknown format specifier");
        p = pointer_from(it);
      } else {
        return handler.on_error("missing '}' in format string");
      }
    }
    begin = p + 1;
  }
}

template <typename T, typename ParseContext>
FMT_CONSTEXPR const typename ParseContext::char_type *
    parse_format_specs(ParseContext &ctx) {
  // GCC 7.2 requires initializer.
  formatter<T, typename ParseContext::char_type> f{};
  return f.parse(ctx);
}

template <typename Char, typename ErrorHandler, typename... Args>
class format_string_checker {
 public:
  explicit FMT_CONSTEXPR format_string_checker(
      basic_string_view<Char> format_str, ErrorHandler eh)
    : arg_id_(-1), context_(format_str, eh),
      parse_funcs_{&parse_format_specs<Args, parse_context_type>...} {}

  typedef internal::null_terminating_iterator<Char> iterator;

  FMT_CONSTEXPR void on_text(const Char *, const Char *) {}

  FMT_CONSTEXPR void on_arg_id() {
    arg_id_ = context_.next_arg_id();
    check_arg_id();
  }
  FMT_CONSTEXPR void on_arg_id(unsigned id) {
    arg_id_ = id;
    context_.check_arg_id(id);
    check_arg_id();
  }
  FMT_CONSTEXPR void on_arg_id(basic_string_view<Char>) {}

  FMT_CONSTEXPR void on_replacement_field(const Char *) {}

  FMT_CONSTEXPR const Char *on_format_specs(iterator it) {
    auto p = pointer_from(it);
    context_.advance_to(p);
    return to_unsigned(arg_id_) < NUM_ARGS ?
          parse_funcs_[arg_id_](context_) : p;
  }

  FMT_CONSTEXPR void on_error(const char *message) {
    context_.on_error(message);
  }

 private:
  typedef basic_parse_context<Char, ErrorHandler> parse_context_type;
  enum { NUM_ARGS = sizeof...(Args) };

  FMT_CONSTEXPR void check_arg_id() {
    if (internal::to_unsigned(arg_id_) >= NUM_ARGS)
      context_.on_error("argument index out of range");
  }

  // Format specifier parsing function.
  typedef const Char *(*parse_func)(parse_context_type &);

  int arg_id_;
  parse_context_type context_;
  parse_func parse_funcs_[NUM_ARGS > 0 ? NUM_ARGS : 1];
};

template <typename Char, typename ErrorHandler, typename... Args>
FMT_CONSTEXPR bool check_format_string(
    basic_string_view<Char> s, ErrorHandler eh = ErrorHandler()) {
  format_string_checker<Char, ErrorHandler, Args...> checker(s, eh);
  parse_format_string<true>(s, checker);
  return true;
}

template <typename... Args, typename String>
typename std::enable_if<is_compile_string<String>::value>::type
    check_format_string(String format_str) {
  FMT_CONSTEXPR_DECL bool invalid_format =
      internal::check_format_string<char, internal::error_handler, Args...>(
        string_view(format_str.data(), format_str.size()));
  (void)invalid_format;
}

// Specifies whether to format T using the standard formatter.
// It is not possible to use get_type in formatter specialization directly
// because of a bug in MSVC.
template <typename Context, typename T>
struct format_type :
  std::integral_constant<bool, get_type<Context, T>::value != custom_type> {};

template <template <typename> class Handler, typename Spec, typename Context>
void handle_dynamic_spec(
    Spec &value, arg_ref<typename Context::char_type> ref, Context &ctx) {
  typedef typename Context::char_type char_type;
  switch (ref.kind) {
  case arg_ref<char_type>::NONE:
    break;
  case arg_ref<char_type>::INDEX:
    internal::set_dynamic_spec<Handler>(
          value, ctx.get_arg(ref.index), ctx.error_handler());
    break;
  case arg_ref<char_type>::NAME:
    internal::set_dynamic_spec<Handler>(
          value, ctx.get_arg(ref.name), ctx.error_handler());
    break;
  }
}
}  // namespace internal

/** The default argument formatter. */
template <typename Range>
class arg_formatter:
  public internal::function<
    typename internal::arg_formatter_base<Range>::iterator>,
  public internal::arg_formatter_base<Range> {
 private:
  typedef typename Range::value_type char_type;
  typedef internal::arg_formatter_base<Range> base;
  typedef basic_format_context<typename base::iterator, char_type> context_type;

  context_type &ctx_;

 public:
  typedef Range range;
  typedef typename base::iterator iterator;
  typedef typename base::format_specs format_specs;

  /**
    \rst
    Constructs an argument formatter object.
    *ctx* is a reference to the formatting context,
    *spec* contains format specifier information for standard argument types.
    \endrst
   */
  explicit arg_formatter(context_type &ctx, format_specs *spec = {})
  : base(Range(ctx.out()), spec), ctx_(ctx) {}

  // Deprecated.
  arg_formatter(context_type &ctx, format_specs &spec)
  : base(Range(ctx.out()), &spec), ctx_(ctx) {}

  using base::operator();

  /** Formats an argument of a user-defined type. */
  iterator operator()(typename basic_format_arg<context_type>::handle handle) {
    handle.format(ctx_);
    return this->out();
  }
};

/**
 An error returned by an operating system or a language runtime,
 for example a file opening error.
*/
class system_error : public std::runtime_error {
 private:
  FMT_API void init(int err_code, string_view format_str, format_args args);

 protected:
  int error_code_;

  system_error() : std::runtime_error("") {}

 public:
  /**
   \rst
   Constructs a :class:`fmt::system_error` object with a description
   formatted with `fmt::format_system_error`. *message* and additional
   arguments passed into the constructor are formatted similarly to
   `fmt::format`.

   **Example**::

     // This throws a system_error with the description
     //   cannot open file 'madeup': No such file or directory
     // or similar (system message may vary).
     const char *filename = "madeup";
     std::FILE *file = std::fopen(filename, "r");
     if (!file)
       throw fmt::system_error(errno, "cannot open file '{}'", filename);
   \endrst
  */
  template <typename... Args>
  system_error(int error_code, string_view message, const Args & ... args)
    : std::runtime_error("") {
    init(error_code, message, make_format_args(args...));
  }

  int error_code() const { return error_code_; }
};

/**
  \rst
  Formats an error returned by an operating system or a language runtime,
  for example a file opening error, and writes it to *out* in the following
  form:

  .. parsed-literal::
     *<message>*: *<system-message>*

  where *<message>* is the passed message and *<system-message>* is
  the system message corresponding to the error code.
  *error_code* is a system error code as given by ``errno``.
  If *error_code* is not a valid error code such as -1, the system message
  may look like "Unknown error -1" and is platform-dependent.
  \endrst
 */
FMT_API void format_system_error(internal::buffer &out, int error_code,
                                 fmt::string_view message) FMT_NOEXCEPT;

/**
  This template provides operations for formatting and writing data into a
  character range.
 */
template <typename Range>
class basic_writer {
 public:
  typedef typename Range::value_type char_type;
  typedef decltype(internal::declval<Range>().begin()) iterator;
  typedef basic_format_specs<char_type> format_specs;

 private:
  iterator out_;  // Output iterator.
  std::unique_ptr<locale_provider> locale_;

  iterator out() const { return out_; }

  // Attempts to reserve space for n extra characters in the output range.
  // Returns a pointer to the reserved range or a reference to out_.
  auto reserve(std::size_t n) -> decltype(internal::reserve(out_, n)) {
    return internal::reserve(out_, n);
  }

  // Writes a value in the format
  //   <left-padding><value><right-padding>
  // where <value> is written by f(it).
  template <typename F>
  void write_padded(std::size_t size, const align_spec &spec, F &&f);

  template <typename F>
  struct padded_int_writer {
    string_view prefix;
    char_type fill;
    std::size_t padding;
    F f;

    template <typename It>
    void operator()(It &&it) const {
      if (prefix.size() != 0)
        it = std::copy_n(prefix.data(), prefix.size(), it);
      it = std::fill_n(it, padding, fill);
      f(it);
    }
  };

  // Writes an integer in the format
  //   <left-padding><prefix><numeric-padding><digits><right-padding>
  // where <digits> are written by f(it).
  template <typename Spec, typename F>
  void write_int(unsigned num_digits, string_view prefix,
                 const Spec &spec, F f) {
    std::size_t size = prefix.size() + num_digits;
    char_type fill = static_cast<char_type>(spec.fill());
    std::size_t padding = 0;
    if (spec.align() == ALIGN_NUMERIC) {
      if (spec.width() > size) {
        padding = spec.width() - size;
        size = spec.width();
      }
    } else if (spec.precision() > static_cast<int>(num_digits)) {
      size = prefix.size() + static_cast<std::size_t>(spec.precision());
      padding = static_cast<std::size_t>(spec.precision()) - num_digits;
      fill = '0';
    }
    align_spec as = spec;
    if (spec.align() == ALIGN_DEFAULT)
      as.align_ = ALIGN_RIGHT;
    write_padded(size, as, padded_int_writer<F>{prefix, fill, padding, f});
  }

  // Writes a decimal integer.
  template <typename Int>
  void write_decimal(Int value) {
    typedef typename internal::int_traits<Int>::main_type main_type;
    main_type abs_value = static_cast<main_type>(value);
    bool is_negative = internal::is_negative(value);
    if (is_negative)
      abs_value = 0 - abs_value;
    unsigned num_digits = internal::count_digits(abs_value);
    auto &&it = reserve((is_negative ? 1 : 0) + num_digits);
    if (is_negative)
      *it++ = '-';
    it = internal::format_decimal(it, abs_value, num_digits);
  }

  // The handle_int_type_spec handler that writes an integer.
  template <typename Int, typename Spec>
  struct int_writer {
    typedef typename internal::int_traits<Int>::main_type unsigned_type;

    basic_writer<Range> &writer;
    const Spec &spec;
    unsigned_type abs_value;
    char prefix[4];
    unsigned prefix_size;

    string_view get_prefix() const { return string_view(prefix, prefix_size); }

    // Counts the number of digits in abs_value. BITS = log2(radix).
    template <unsigned BITS>
    unsigned count_digits() const {
      unsigned_type n = abs_value;
      unsigned num_digits = 0;
      do {
        ++num_digits;
      } while ((n >>= BITS) != 0);
      return num_digits;
    }

    int_writer(basic_writer<Range> &w, Int value, const Spec &s)
      : writer(w), spec(s), abs_value(static_cast<unsigned_type>(value)),
        prefix_size(0) {
      if (internal::is_negative(value)) {
        prefix[0] = '-';
        ++prefix_size;
        abs_value = 0 - abs_value;
      } else if (spec.flag(SIGN_FLAG)) {
        prefix[0] = spec.flag(PLUS_FLAG) ? '+' : ' ';
        ++prefix_size;
      }
    }

    struct dec_writer {
      unsigned_type abs_value;
      unsigned num_digits;

      template <typename It>
      void operator()(It &&it) const {
        it = internal::format_decimal(it, abs_value, num_digits);
      }
    };

    void on_dec() {
      unsigned num_digits = internal::count_digits(abs_value);
      writer.write_int(num_digits, get_prefix(), spec,
                       dec_writer{abs_value, num_digits});
    }

    struct hex_writer {
      int_writer &self;
      unsigned num_digits;

      template <typename It>
      void operator()(It &&it) const {
        it = internal::format_uint<4>(it, self.abs_value, num_digits,
                                      self.spec.type() != 'x');
      }
    };

    void on_hex() {
      if (spec.flag(HASH_FLAG)) {
        prefix[prefix_size++] = '0';
        prefix[prefix_size++] = static_cast<char>(spec.type());
      }
      unsigned num_digits = count_digits<4>();
      writer.write_int(num_digits, get_prefix(), spec,
                       hex_writer{*this, num_digits});
    }

    template <int BITS>
    struct bin_writer {
      unsigned_type abs_value;
      unsigned num_digits;

      template <typename It>
      void operator()(It &&it) const {
        it = internal::format_uint<BITS>(it, abs_value, num_digits);
      }
    };

    void on_bin() {
      if (spec.flag(HASH_FLAG)) {
        prefix[prefix_size++] = '0';
        prefix[prefix_size++] = static_cast<char>(spec.type());
      }
      unsigned num_digits = count_digits<1>();
      writer.write_int(num_digits, get_prefix(), spec,
                       bin_writer<1>{abs_value, num_digits});
    }

    void on_oct() {
      unsigned num_digits = count_digits<3>();
      if (spec.flag(HASH_FLAG) &&
          spec.precision() <= static_cast<int>(num_digits)) {
        // Octal prefix '0' is counted as a digit, so only add it if precision
        // is not greater than the number of digits.
        prefix[prefix_size++] = '0';
      }
      writer.write_int(num_digits, get_prefix(), spec,
                       bin_writer<3>{abs_value, num_digits});
    }

    enum { SEP_SIZE = 1 };

    struct num_writer {
      unsigned_type abs_value;
      unsigned size;
      char_type sep;

      template <typename It>
      void operator()(It &&it) const {
        basic_string_view<char_type> s(&sep, SEP_SIZE);
        it = format_decimal(it, abs_value, size,
                            internal::add_thousands_sep<char_type>(s));
      }
    };

    void on_num() {
      unsigned num_digits = internal::count_digits(abs_value);
      char_type sep = internal::thousands_sep<char_type>(writer.locale_.get());
      unsigned size = num_digits + SEP_SIZE * ((num_digits - 1) / 3);
      writer.write_int(size, get_prefix(), spec,
                       num_writer{abs_value, size, sep});
    }

    void on_error() {
      FMT_THROW(format_error("invalid type specifier"));
    }
  };

  // Writes a formatted integer.
  template <typename T, typename Spec>
  void write_int(T value, const Spec &spec) {
    internal::handle_int_type_spec(spec.type(),
                                   int_writer<T, Spec>(*this, value, spec));
  }

  enum {INF_SIZE = 3}; // This is an enum to workaround a bug in MSVC.

  struct inf_or_nan_writer {
    char sign;
    const char *str;

    template <typename It>
    void operator()(It &&it) const {
      if (sign)
        *it++ = sign;
      it = std::copy_n(str, static_cast<std::size_t>(INF_SIZE), it);
    }
  };

  struct double_writer {
    size_t n;
    char sign;
    basic_memory_buffer<char_type> &buffer;

    template <typename It>
    void operator()(It &&it) {
      if (sign) {
        *it++ = sign;
        --n;
      }
      it = std::copy_n(buffer.begin(), n, it);
    }
  };

  // Formats a floating-point number (double or long double).
  template <typename T>
  void write_double(T value, const format_specs &spec);
  template <typename T>
  void write_double_sprintf(T value, const format_specs &spec,
                            internal::basic_buffer<char_type>& buffer);

  template <typename Char>
  struct str_writer {
    const Char *s;
    std::size_t size;

    template <typename It>
    void operator()(It &&it) const {
      it = std::copy_n(s, size, it);
    }
  };

  // Writes a formatted string.
  template <typename Char>
  void write_str(const Char *s, std::size_t size, const align_spec &spec) {
    write_padded(size, spec, str_writer<Char>{s, size});
  }

  template <typename Char>
  void write_str(basic_string_view<Char> str, const format_specs &spec);

  // Appends floating-point length specifier to the format string.
  // The second argument is only used for overload resolution.
  void append_float_length(char_type *&format_ptr, long double) {
    *format_ptr++ = 'L';
  }

  template<typename T>
  void append_float_length(char_type *&, T) {}

  template <typename Char>
  friend class internal::arg_formatter_base;

 public:
  /** Constructs a ``basic_writer`` object. */
  explicit basic_writer(Range out): out_(out.begin()) {}

  void write(int value) { write_decimal(value); }
  void write(long value) { write_decimal(value); }
  void write(long long value) { write_decimal(value); }

  void write(unsigned value) { write_decimal(value); }
  void write(unsigned long value) { write_decimal(value); }
  void write(unsigned long long value) { write_decimal(value); }

  /**
    \rst
    Formats *value* and writes it to the buffer.
    \endrst
   */
  template <typename T, typename FormatSpec, typename... FormatSpecs>
  typename std::enable_if<std::is_integral<T>::value, void>::type
      write(T value, FormatSpec spec, FormatSpecs... specs) {
    format_specs s(spec, specs...);
    s.align_ = ALIGN_RIGHT;
    write_int(value, s);
  }

  void write(double value) {
    write_double(value, format_specs());
  }

  /**
    \rst
    Formats *value* using the general format for floating-point numbers
    (``'g'``) and writes it to the buffer.
    \endrst
   */
  void write(long double value) {
    write_double(value, format_specs());
  }

  /** Writes a character to the buffer. */
  void write(char value) {
    *reserve(1) = value;
  }

  void write(wchar_t value) {
    internal::require_wchar<char_type>();
    *reserve(1) = value;
  }

  /**
    \rst
    Writes *value* to the buffer.
    \endrst
   */
  void write(string_view value) {
    auto &&it = reserve(value.size());
    it = std::copy(value.begin(), value.end(), it);
  }

  void write(wstring_view value) {
    internal::require_wchar<char_type>();
    auto &&it = reserve(value.size());
    it = std::copy(value.begin(), value.end(), it);
  }

  template <typename... FormatSpecs>
  void write(basic_string_view<char_type> str, FormatSpecs... specs) {
    write_str(str, format_specs(specs...));
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, void>::value>::type
      write(const T *p) {
    format_specs specs;
    specs.flags_ = HASH_FLAG;
    specs.type_ = 'x';
    write_int(reinterpret_cast<uintptr_t>(p), specs);
  }
};

template <typename Range>
template <typename F>
void basic_writer<Range>::write_padded(
    std::size_t size, const align_spec &spec, F &&f) {
  unsigned width = spec.width();
  if (width <= size)
    return f(reserve(size));
  auto &&it = reserve(width);
  char_type fill = static_cast<char_type>(spec.fill());
  std::size_t padding = width - size;
  if (spec.align() == ALIGN_RIGHT) {
    it = std::fill_n(it, padding, fill);
    f(it);
  } else if (spec.align() == ALIGN_CENTER) {
    std::size_t left_padding = padding / 2;
    it = std::fill_n(it, left_padding, fill);
    f(it);
    it = std::fill_n(it, padding - left_padding, fill);
  } else {
    f(it);
    it = std::fill_n(it, padding, fill);
  }
}

template <typename Range>
template <typename Char>
void basic_writer<Range>::write_str(
    basic_string_view<Char> s, const format_specs &spec) {
  const Char *data = s.data();
  std::size_t size = s.size();
  std::size_t precision = static_cast<std::size_t>(spec.precision_);
  if (spec.precision_ >= 0 && precision < size)
    size = precision;
  write_str(data, size, spec);
}

template <typename Char>
struct float_spec_handler {
  Char type;
  bool upper;

  explicit float_spec_handler(Char t) : type(t), upper(false) {}

  void on_general() {
    if (type == 'G')
      upper = true;
    else
      type = 'g';
  }

  void on_exp() {
    if (type == 'E')
      upper = true;
  }

  void on_fixed() {
    if (type == 'F') {
      upper = true;
#if FMT_MSC_VER
      // MSVC's printf doesn't support 'F'.
      type = 'f';
#endif
    }
  }

  void on_hex() {
    if (type == 'A')
      upper = true;
  }

  void on_error() {
    FMT_THROW(format_error("invalid type specifier"));
  }
};

template <typename Range>
template <typename T>
void basic_writer<Range>::write_double(T value, const format_specs &spec) {
  // Check type.
  float_spec_handler<char_type> handler(spec.type());
  internal::handle_float_type_spec(spec.type(), handler);

  char sign = 0;
  // Use isnegative instead of value < 0 because the latter is always
  // false for NaN.
  if (internal::fputil::isnegative(static_cast<double>(value))) {
    sign = '-';
    value = -value;
  } else if (spec.flag(SIGN_FLAG)) {
    sign = spec.flag(PLUS_FLAG) ? '+' : ' ';
  }

  struct write_inf_or_nan_t {
    basic_writer &writer;
    format_specs spec;
    char sign;
    void operator()(const char *str) const {
      writer.write_padded(INF_SIZE + (sign ? 1 : 0), spec,
                          inf_or_nan_writer{sign, str});
    }
  } write_inf_or_nan = {*this, spec, sign};

  // Format NaN and ininity ourselves because sprintf's output is not consistent
  // across platforms.
  if (internal::fputil::isnotanumber(value))
    return write_inf_or_nan(handler.upper ? "NAN" : "nan");
  if (internal::fputil::isinfinity(value))
    return write_inf_or_nan(handler.upper ? "INF" : "inf");

  basic_memory_buffer<char_type> buffer;
  char type = static_cast<char>(spec.type());
  if (internal::const_check(
        internal::use_grisu() && sizeof(T) <= sizeof(double)) &&
      type != 'a' && type != 'A') {
    char buf[100]; // TODO: correct buffer size
    size_t size = 0;
    internal::grisu2_format(static_cast<double>(value), buf, size, type,
                            spec.precision(), spec.flag(HASH_FLAG));
    FMT_ASSERT(size <= 100, "buffer overflow");
    buffer.append(buf, buf + size); // TODO: avoid extra copy
  } else {
    format_specs normalized_spec(spec);
    normalized_spec.type_ = handler.type;
    write_double_sprintf(value, normalized_spec, buffer);
  }
  size_t n = buffer.size();
  align_spec as = spec;
  if (spec.align() == ALIGN_NUMERIC) {
    if (sign) {
      auto &&it = reserve(1);
      *it++ = sign;
      sign = 0;
      if (as.width_)
        --as.width_;
    }
    as.align_ = ALIGN_RIGHT;
  } else {
    if (spec.align() == ALIGN_DEFAULT)
      as.align_ = ALIGN_RIGHT;
    if (sign)
      ++n;
  }
  write_padded(n, as, double_writer{n, sign, buffer});
}

template <typename Range>
template <typename T>
void basic_writer<Range>::write_double_sprintf(
    T value, const format_specs &spec,
    internal::basic_buffer<char_type>& buffer) {
  // Buffer capacity must be non-zero, otherwise MSVC's vsnprintf_s will fail.
  FMT_ASSERT(buffer.capacity() != 0, "empty buffer");

  // Build format string.
  enum { MAX_FORMAT_SIZE = 10}; // longest format: %#-*.*Lg
  char_type format[MAX_FORMAT_SIZE];
  char_type *format_ptr = format;
  *format_ptr++ = '%';
  if (spec.flag(HASH_FLAG))
    *format_ptr++ = '#';
  if (spec.precision() >= 0) {
    *format_ptr++ = '.';
    *format_ptr++ = '*';
  }

  append_float_length(format_ptr, value);
  *format_ptr++ = spec.type();
  *format_ptr = '\0';

  // Format using snprintf.
  char_type *start = FMT_NULL;
  for (;;) {
    std::size_t buffer_size = buffer.capacity();
    start = &buffer[0];
    int result = internal::char_traits<char_type>::format_float(
        start, buffer_size, format, spec.precision(), value);
    if (result >= 0) {
      unsigned n = internal::to_unsigned(result);
      if (n < buffer.capacity()) {
        buffer.resize(n);
        break;  // The buffer is large enough - continue with formatting.
      }
      buffer.reserve(n + 1);
    } else {
      // If result is negative we ask to increase the capacity by at least 1,
      // but as std::vector, the buffer grows exponentially.
      buffer.reserve(buffer.capacity() + 1);
    }
  }
}

// Reports a system error without throwing an exception.
// Can be used to report errors from destructors.
FMT_API void report_system_error(int error_code,
                                 string_view message) FMT_NOEXCEPT;

#if FMT_USE_WINDOWS_H

/** A Windows error. */
class windows_error : public system_error {
 private:
  FMT_API void init(int error_code, string_view format_str, format_args args);

 public:
  /**
   \rst
   Constructs a :class:`fmt::windows_error` object with the description
   of the form

   .. parsed-literal::
     *<message>*: *<system-message>*

   where *<message>* is the formatted message and *<system-message>* is the
   system message corresponding to the error code.
   *error_code* is a Windows error code as given by ``GetLastError``.
   If *error_code* is not a valid error code such as -1, the system message
   will look like "error -1".

   **Example**::

     // This throws a windows_error with the description
     //   cannot open file 'madeup': The system cannot find the file specified.
     // or similar (system message may vary).
     const char *filename = "madeup";
     LPOFSTRUCT of = LPOFSTRUCT();
     HFILE file = OpenFile(filename, &of, OF_READ);
     if (file == HFILE_ERROR) {
       throw fmt::windows_error(GetLastError(),
                                "cannot open file '{}'", filename);
     }
   \endrst
  */
  template <typename... Args>
  windows_error(int error_code, string_view message, const Args & ... args) {
    init(error_code, message, make_format_args(args...));
  }
};

// Reports a Windows error without throwing an exception.
// Can be used to report errors from destructors.
FMT_API void report_windows_error(int error_code,
                                  string_view message) FMT_NOEXCEPT;

#endif

/** Fast integer formatter. */
class format_int {
 private:
  // Buffer should be large enough to hold all digits (digits10 + 1),
  // a sign and a null character.
  enum {BUFFER_SIZE = std::numeric_limits<unsigned long long>::digits10 + 3};
  mutable char buffer_[BUFFER_SIZE];
  char *str_;

  // Formats value in reverse and returns a pointer to the beginning.
  char *format_decimal(unsigned long long value) {
    char *ptr = buffer_ + BUFFER_SIZE - 1;
    while (value >= 100) {
      // Integer division is slow so do it for a group of two digits instead
      // of for every digit. The idea comes from the talk by Alexandrescu
      // "Three Optimization Tips for C++". See speed-test for a comparison.
      unsigned index = static_cast<unsigned>((value % 100) * 2);
      value /= 100;
      *--ptr = internal::data::DIGITS[index + 1];
      *--ptr = internal::data::DIGITS[index];
    }
    if (value < 10) {
      *--ptr = static_cast<char>('0' + value);
      return ptr;
    }
    unsigned index = static_cast<unsigned>(value * 2);
    *--ptr = internal::data::DIGITS[index + 1];
    *--ptr = internal::data::DIGITS[index];
    return ptr;
  }

  void format_signed(long long value) {
    unsigned long long abs_value = static_cast<unsigned long long>(value);
    bool negative = value < 0;
    if (negative)
      abs_value = 0 - abs_value;
    str_ = format_decimal(abs_value);
    if (negative)
      *--str_ = '-';
  }

 public:
  explicit format_int(int value) { format_signed(value); }
  explicit format_int(long value) { format_signed(value); }
  explicit format_int(long long value) { format_signed(value); }
  explicit format_int(unsigned value) : str_(format_decimal(value)) {}
  explicit format_int(unsigned long value) : str_(format_decimal(value)) {}
  explicit format_int(unsigned long long value) : str_(format_decimal(value)) {}

  /** Returns the number of characters written to the output buffer. */
  std::size_t size() const {
    return internal::to_unsigned(buffer_ - str_ + BUFFER_SIZE - 1);
  }

  /**
    Returns a pointer to the output buffer content. No terminating null
    character is appended.
   */
  const char *data() const { return str_; }

  /**
    Returns a pointer to the output buffer content with terminating null
    character appended.
   */
  const char *c_str() const {
    buffer_[BUFFER_SIZE - 1] = '\0';
    return str_;
  }

  /**
    \rst
    Returns the content of the output buffer as an ``std::string``.
    \endrst
   */
  std::string str() const { return std::string(str_, size()); }
};

// Formats a decimal integer value writing into buffer and returns
// a pointer to the end of the formatted string. This function doesn't
// write a terminating null character.
template <typename T>
inline void format_decimal(char *&buffer, T value) {
  typedef typename internal::int_traits<T>::main_type main_type;
  main_type abs_value = static_cast<main_type>(value);
  if (internal::is_negative(value)) {
    *buffer++ = '-';
    abs_value = 0 - abs_value;
  }
  if (abs_value < 100) {
    if (abs_value < 10) {
      *buffer++ = static_cast<char>('0' + abs_value);
      return;
    }
    unsigned index = static_cast<unsigned>(abs_value * 2);
    *buffer++ = internal::data::DIGITS[index];
    *buffer++ = internal::data::DIGITS[index + 1];
    return;
  }
  unsigned num_digits = internal::count_digits(abs_value);
  internal::format_decimal(buffer, abs_value, num_digits);
  buffer += num_digits;
}

// Formatter of objects of type T.
template <typename T, typename Char>
struct formatter<
    T, Char,
    typename std::enable_if<internal::format_type<
        typename buffer_context<Char>::type, T>::value>::type> {

  // Parses format specifiers stopping either at the end of the range or at the
  // terminating '}'.
  template <typename ParseContext>
  FMT_CONSTEXPR typename ParseContext::iterator parse(ParseContext &ctx) {
    auto it = internal::null_terminating_iterator<Char>(ctx);
    typedef internal::dynamic_specs_handler<ParseContext> handler_type;
    auto type = internal::get_type<
      typename buffer_context<Char>::type, T>::value;
    internal::specs_checker<handler_type>
        handler(handler_type(specs_, ctx), type);
    it = parse_format_specs(it, handler);
    auto type_spec = specs_.type();
    auto eh = ctx.error_handler();
    switch (type) {
    case internal::none_type:
    case internal::named_arg_type:
      FMT_ASSERT(false, "invalid argument type");
      break;
    case internal::int_type:
    case internal::uint_type:
    case internal::long_long_type:
    case internal::ulong_long_type:
    case internal::bool_type:
      handle_int_type_spec(
            type_spec, internal::int_type_checker<decltype(eh)>(eh));
      break;
    case internal::char_type:
      handle_char_specs(
          &specs_,
          internal::char_specs_checker<decltype(eh), decltype(type_spec)>(
              type_spec, eh));
      break;
    case internal::double_type:
    case internal::long_double_type:
      handle_float_type_spec(
            type_spec, internal::float_type_checker<decltype(eh)>(eh));
      break;
    case internal::cstring_type:
      internal::handle_cstring_type_spec(
            type_spec, internal::cstring_type_checker<decltype(eh)>(eh));
      break;
    case internal::string_type:
      internal::check_string_type_spec(type_spec, eh);
      break;
    case internal::pointer_type:
      internal::check_pointer_type_spec(type_spec, eh);
      break;
    case internal::custom_type:
      // Custom format specifiers should be checked in parse functions of
      // formatter specializations.
      break;
    }
    return pointer_from(it);
  }

  template <typename FormatContext>
  auto format(const T &val, FormatContext &ctx) -> decltype(ctx.out()) {
    internal::handle_dynamic_spec<internal::width_checker>(
      specs_.width_, specs_.width_ref, ctx);
    internal::handle_dynamic_spec<internal::precision_checker>(
      specs_.precision_, specs_.precision_ref, ctx);
    typedef output_range<typename FormatContext::iterator,
                         typename FormatContext::char_type> range_type;
    return fmt::visit(arg_formatter<range_type>(ctx, &specs_),
                      internal::make_arg<FormatContext>(val));
  }

 private:
  internal::dynamic_format_specs<Char> specs_;
};

// A formatter for types known only at run time such as variant alternatives.
//
// Usage:
//   typedef std::variant<int, std::string> variant;
//   template <>
//   struct formatter<variant>: dynamic_formatter<> {
//     void format(buffer &buf, const variant &v, context &ctx) {
//       visit([&](const auto &val) { format(buf, val, ctx); }, v);
//     }
//   };
template <typename Char = char>
class dynamic_formatter {
 private:
  struct null_handler: internal::error_handler {
    void on_align(alignment) {}
    void on_plus() {}
    void on_minus() {}
    void on_space() {}
    void on_hash() {}
  };

 public:
  template <typename ParseContext>
  auto parse(ParseContext &ctx) -> decltype(ctx.begin()) {
    auto it = internal::null_terminating_iterator<Char>(ctx);
    // Checks are deferred to formatting time when the argument type is known.
    internal::dynamic_specs_handler<ParseContext> handler(specs_, ctx);
    it = parse_format_specs(it, handler);
    return pointer_from(it);
  }

  template <typename T, typename FormatContext>
  auto format(const T &val, FormatContext &ctx) -> decltype(ctx.out()) {
    handle_specs(ctx);
    internal::specs_checker<null_handler>
        checker(null_handler(), internal::get_type<FormatContext, T>::value);
    checker.on_align(specs_.align());
    if (specs_.flags_ == 0) {
      // Do nothing.
    } else if (specs_.flag(SIGN_FLAG)) {
      if (specs_.flag(PLUS_FLAG))
        checker.on_plus();
      else
        checker.on_space();
    } else if (specs_.flag(MINUS_FLAG)) {
      checker.on_minus();
    } else if (specs_.flag(HASH_FLAG)) {
      checker.on_hash();
    }
    if (specs_.precision_ != -1)
      checker.end_precision();
    typedef output_range<typename FormatContext::iterator,
                         typename FormatContext::char_type> range;
    fmt::visit(arg_formatter<range>(ctx, &specs_),
               internal::make_arg<FormatContext>(val));
    return ctx.out();
  }

 private:
  template <typename Context>
  void handle_specs(Context &ctx) {
    internal::handle_dynamic_spec<internal::width_checker>(
      specs_.width_, specs_.width_ref, ctx);
    internal::handle_dynamic_spec<internal::precision_checker>(
      specs_.precision_, specs_.precision_ref, ctx);
  }

  internal::dynamic_format_specs<Char> specs_;
};

template <typename Range, typename Char>
typename basic_format_context<Range, Char>::format_arg
  basic_format_context<Range, Char>::get_arg(
    basic_string_view<char_type> name) {
  map_.init(this->args());
  format_arg arg = map_.find(name);
  if (arg.type() == internal::none_type)
    this->on_error("argument not found");
  return arg;
}

template <typename ArgFormatter, typename Char, typename Context>
struct format_handler : internal::error_handler {
  typedef internal::null_terminating_iterator<Char> iterator;
  typedef typename ArgFormatter::range range;

  format_handler(range r, basic_string_view<Char> str,
                 basic_format_args<Context> format_args)
    : context(r.begin(), str, format_args) {}

  void on_text(const Char *begin, const Char *end) {
    auto size = internal::to_unsigned(end - begin);
    auto out = context.out();
    auto &&it = internal::reserve(out, size);
    it = std::copy_n(begin, size, it);
    context.advance_to(out);
  }

  void on_arg_id() { arg = context.next_arg(); }
  void on_arg_id(unsigned id) {
    context.parse_context().check_arg_id(id);
    arg = context.get_arg(id);
  }
  void on_arg_id(basic_string_view<Char> id) {
    arg = context.get_arg(id);
  }

  void on_replacement_field(const Char *p) {
    context.parse_context().advance_to(p);
    if (!fmt::visit(internal::custom_formatter<Char, Context>(context), arg))
      context.advance_to(fmt::visit(ArgFormatter(context), arg));
  }

  iterator on_format_specs(iterator it) {
    auto& parse_ctx = context.parse_context();
    parse_ctx.advance_to(pointer_from(it));
    if (fmt::visit(internal::custom_formatter<Char, Context>(context), arg))
      return iterator(parse_ctx);
    basic_format_specs<Char> specs;
    using internal::specs_handler;
    internal::specs_checker<specs_handler<Context>>
        handler(specs_handler<Context>(specs, context), arg.type());
    it = parse_format_specs(it, handler);
    if (*it != '}')
      on_error("missing '}' in format string");
    parse_ctx.advance_to(pointer_from(it));
    context.advance_to(fmt::visit(ArgFormatter(context, &specs), arg));
    return it;
  }

  Context context;
  basic_format_arg<Context> arg;
};

/** Formats arguments and writes the output to the range. */
template <typename ArgFormatter, typename Char, typename Context>
typename Context::iterator vformat_to(typename ArgFormatter::range out,
                                      basic_string_view<Char> format_str,
                                      basic_format_args<Context> args) {
  format_handler<ArgFormatter, Char, Context> h(out, format_str, args);
  internal::parse_format_string<false>(format_str, h);
  return h.context.out();
}

// Casts ``p`` to ``const void*`` for pointer formatting.
// Example:
//   auto s = format("{}", ptr(p));
template <typename T>
inline const void *ptr(const T *p) { return p; }

template <typename It, typename Char>
struct arg_join {
  It begin;
  It end;
  basic_string_view<Char> sep;

  arg_join(It begin, It end, basic_string_view<Char> sep)
    : begin(begin), end(end), sep(sep) {}
};

template <typename It, typename Char>
struct formatter<arg_join<It, Char>, Char>:
    formatter<typename std::iterator_traits<It>::value_type, Char> {
  template <typename FormatContext>
  auto format(const arg_join<It, Char> &value, FormatContext &ctx)
      -> decltype(ctx.out()) {
    typedef formatter<typename std::iterator_traits<It>::value_type, Char> base;
    auto it = value.begin;
    auto out = ctx.out();
    if (it != value.end) {
      out = base::format(*it++, ctx);
      while (it != value.end) {
        out = std::copy(value.sep.begin(), value.sep.end(), out);
        ctx.advance_to(out);
        out = base::format(*it++, ctx);
      }
    }
    return out;
  }
};

template <typename It>
arg_join<It, char> join(It begin, It end, string_view sep) {
  return arg_join<It, char>(begin, end, sep);
}

template <typename It>
arg_join<It, wchar_t> join(It begin, It end, wstring_view sep) {
  return arg_join<It, wchar_t>(begin, end, sep);
}

// The following causes ICE in gcc 4.4.
#if FMT_USE_TRAILING_RETURN && (!FMT_GCC_VERSION || FMT_GCC_VERSION >= 405)
template <typename Range>
auto join(const Range &range, string_view sep)
    -> arg_join<decltype(internal::begin(range)), char> {
  return join(internal::begin(range), internal::end(range), sep);
}

template <typename Range>
auto join(const Range &range, wstring_view sep)
    -> arg_join<decltype(internal::begin(range)), wchar_t> {
  return join(internal::begin(range), internal::end(range), sep);
}
#endif

/**
  \rst
  Converts *value* to ``std::string`` using the default format for type *T*.
  It doesn't support user-defined types with custom formatters.

  **Example**::

    #include <fmt/format.h>

    std::string answer = fmt::to_string(42);
  \endrst
 */
template <typename T>
std::string to_string(const T &value) {
  std::string str;
  internal::container_buffer<std::string> buf(str);
  writer(buf).write(value);
  return str;
}

/**
  Converts *value* to ``std::wstring`` using the default format for type *T*.
 */
template <typename T>
std::wstring to_wstring(const T &value) {
  std::wstring str;
  internal::container_buffer<std::wstring> buf(str);
  wwriter(buf).write(value);
  return str;
}

template <typename Char, std::size_t SIZE>
std::basic_string<Char> to_string(const basic_memory_buffer<Char, SIZE> &buf) {
  return std::basic_string<Char>(buf.data(), buf.size());
}

inline format_context::iterator vformat_to(
    internal::buffer &buf, string_view format_str, format_args args) {
  typedef back_insert_range<internal::buffer> range;
  return vformat_to<arg_formatter<range>>(buf, format_str, args);
}

inline wformat_context::iterator vformat_to(
    internal::wbuffer &buf, wstring_view format_str, wformat_args args) {
  typedef back_insert_range<internal::wbuffer> range;
  return vformat_to<arg_formatter<range>>(buf, format_str, args);
}

template <
    typename String, typename... Args,
    std::size_t SIZE = inline_buffer_size,
    typename Char = typename internal::format_string_traits<String>::char_type>
inline typename buffer_context<Char>::type::iterator format_to(
    basic_memory_buffer<Char, SIZE> &buf, const String &format_str,
    const Args & ... args) {
  internal::check_format_string<Args...>(format_str);
  return vformat_to(
        buf, basic_string_view<Char>(format_str),
        make_format_args<typename buffer_context<Char>::type>(args...));
}

template <typename OutputIt, typename Char = char>
//using format_context_t = basic_format_context<OutputIt, Char>;
struct format_context_t { typedef basic_format_context<OutputIt, Char> type; };

template <typename OutputIt, typename Char = char>
//using format_args_t = basic_format_args<format_context_t<OutputIt, Char>>;
struct format_args_t {
  typedef basic_format_args<
    typename format_context_t<OutputIt, Char>::type> type;
};

template <typename OutputIt, typename... Args>
inline OutputIt vformat_to(OutputIt out, string_view format_str,
                           typename format_args_t<OutputIt>::type args) {
  typedef output_range<OutputIt, char> range;
  return vformat_to<arg_formatter<range>>(range(out), format_str, args);
}
template <typename OutputIt, typename... Args>
inline OutputIt vformat_to(
    OutputIt out, wstring_view format_str,
    typename format_args_t<OutputIt, wchar_t>::type args) {
  typedef output_range<OutputIt, wchar_t> range;
  return vformat_to<arg_formatter<range>>(range(out), format_str, args);
}

/**
 \rst
 Formats arguments, writes the result to the output iterator ``out`` and returns
 the iterator past the end of the output range.

 **Example**::

   std::vector<char> out;
   fmt::format_to(std::back_inserter(out), "{}", 42);
 \endrst
 */
template <typename OutputIt, typename... Args>
inline OutputIt format_to(OutputIt out, string_view format_str,
                          const Args & ... args) {
  return vformat_to(out, format_str,
      make_format_args<typename format_context_t<OutputIt>::type>(args...));
}

template <typename OutputIt>
struct format_to_n_result {
  /** Iterator past the end of the output range. */
  OutputIt out;
  /** Total (not truncated) output size. */
  std::size_t size;
};

template <typename OutputIt>
using format_to_n_context = typename fmt::format_context_t<
  fmt::internal::truncating_iterator<OutputIt>>::type;

template <typename OutputIt>
using format_to_n_args = fmt::basic_format_args<format_to_n_context<OutputIt>>;

template <typename OutputIt, typename ...Args>
inline format_arg_store<format_to_n_context<OutputIt>, Args...>
    make_format_to_n_args(const Args & ... args) {
  return format_arg_store<format_to_n_context<OutputIt>, Args...>(args...);
}

template <typename OutputIt, typename... Args>
inline format_to_n_result<OutputIt> vformat_to_n(
    OutputIt out, std::size_t n, string_view format_str,
    format_to_n_args<OutputIt> args) {
  typedef internal::truncating_iterator<OutputIt> It;
  auto it = vformat_to(It(out, n), format_str, args);
  return {it.base(), it.count()};
}

/**
 \rst
 Formats arguments, writes up to ``n`` characters of the result to the output
 iterator ``out`` and returns the total output size and the iterator past the
 end of the output range.
 \endrst
 */
template <typename OutputIt, typename... Args>
inline format_to_n_result<OutputIt> format_to_n(
    OutputIt out, std::size_t n, string_view format_str, const Args &... args) {
  return vformat_to_n<OutputIt>(
    out, n, format_str, make_format_to_n_args<OutputIt>(args...));
}
template <typename OutputIt, typename... Args>
inline format_to_n_result<OutputIt> format_to_n(
    OutputIt out, std::size_t n, wstring_view format_str,
    const Args &... args) {
  typedef internal::truncating_iterator<OutputIt> It;
  auto it = vformat_to(It(out, n), format_str,
      make_format_args<typename format_context_t<It, wchar_t>::type>(args...));
  return {it.base(), it.count()};
}

template <typename Char>
inline std::basic_string<Char> internal::vformat(
    basic_string_view<Char> format_str,
    basic_format_args<typename buffer_context<Char>::type> args) {
  basic_memory_buffer<Char> buffer;
  vformat_to(buffer, format_str, args);
  return fmt::to_string(buffer);
}

template <typename String, typename... Args>
inline typename std::enable_if<internal::is_compile_string<String>::value>::type
    print(String format_str, const Args & ... args) {
  internal::check_format_string<Args...>(format_str);
  return vprint(format_str.data(), make_format_args(args...));
}

/**
  Returns the number of characters in the output of
  ``format(format_str, args...)``.
 */
template <typename... Args>
inline std::size_t formatted_size(string_view format_str,
                                  const Args & ... args) {
  auto it = format_to(internal::counting_iterator<char>(), format_str, args...);
  return it.count();
}

#if FMT_USE_USER_DEFINED_LITERALS
namespace internal {

# if FMT_UDL_TEMPLATE
template <typename Char, Char... CHARS>
class udl_formatter {
 public:
  template <typename... Args>
  std::basic_string<Char> operator()(const Args &... args) const {
    FMT_CONSTEXPR_DECL Char s[] = {CHARS..., '\0'};
    FMT_CONSTEXPR_DECL bool invalid_format =
        check_format_string<Char, error_handler, Args...>(
          basic_string_view<Char>(s, sizeof...(CHARS)));
    (void)invalid_format;
    return format(s, args...);
  }
};
# else
template <typename Char>
struct udl_formatter {
  const Char *str;

  template <typename... Args>
  auto operator()(Args && ... args) const
                  -> decltype(format(str, std::forward<Args>(args)...)) {
    return format(str, std::forward<Args>(args)...);
  }
};
# endif // FMT_UDL_TEMPLATE

template <typename Char>
struct udl_arg {
  const Char *str;

  template <typename T>
  named_arg<T, Char> operator=(T &&value) const {
    return {str, std::forward<T>(value)};
  }
};

} // namespace internal

inline namespace literals {

# if FMT_UDL_TEMPLATE
template <typename Char, Char... CHARS>
FMT_CONSTEXPR internal::udl_formatter<Char, CHARS...> operator""_format() {
  return {};
}
# else
/**
  \rst
  User-defined literal equivalent of :func:`fmt::format`.

  **Example**::

    using namespace fmt::literals;
    std::string message = "The answer is {}"_format(42);
  \endrst
 */
inline internal::udl_formatter<char>
operator"" _format(const char *s, std::size_t) { return {s}; }
inline internal::udl_formatter<wchar_t>
operator"" _format(const wchar_t *s, std::size_t) { return {s}; }
# endif // FMT_UDL_TEMPLATE

/**
  \rst
  User-defined literal equivalent of :func:`fmt::arg`.

  **Example**::

    using namespace fmt::literals;
    fmt::print("Elapsed time: {s:.2f} seconds", "s"_a=1.23);
  \endrst
 */
inline internal::udl_arg<char>
operator"" _a(const char *s, std::size_t) { return {s}; }
inline internal::udl_arg<wchar_t>
operator"" _a(const wchar_t *s, std::size_t) { return {s}; }
} // inline namespace literals
#endif // FMT_USE_USER_DEFINED_LITERALS
FMT_END_NAMESPACE

#define FMT_STRING(s) [] { \
    typedef typename std::decay<decltype(s)>::type pointer; \
    struct S : fmt::compile_string { \
      static FMT_CONSTEXPR pointer data() { return s; } \
      static FMT_CONSTEXPR size_t size() { return sizeof(s); } \
      explicit operator fmt::string_view() const { return s; } \
    }; \
    return S{}; \
  }()

#if defined(FMT_STRING_ALIAS) && FMT_STRING_ALIAS
/**
  \rst
  Constructs a compile-time format string. This macro is disabled by default to
  prevent potential name collisions. To enable it define ``FMT_STRING_ALIAS`` to
  1 before including ``fmt/format.h``.

  **Example**::

    #define FMT_STRING_ALIAS 1
    #include <fmt/format.h>
    // A compile-time error because 'd' is an invalid specifier for strings.
    std::string s = format(fmt("{:d}"), "foo");
  \endrst
 */
# define fmt(s) FMT_STRING(s)
#endif

#ifdef FMT_HEADER_ONLY
# define FMT_FUNC inline
# include "format-inl.h"
#else
# define FMT_FUNC
#endif

// Restore warnings.
#if FMT_GCC_VERSION >= 406 || FMT_CLANG_VERSION
# pragma GCC diagnostic pop
#endif

#endif  // FMT_FORMAT_H_
