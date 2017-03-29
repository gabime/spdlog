/*
 Formatting library for C++

 Copyright (c) 2012 - 2016, Victor Zverovich
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

#include <cassert>
#include <clocale>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

#ifdef _SECURE_SCL
# define FMT_SECURE_SCL _SECURE_SCL
#else
# define FMT_SECURE_SCL 0
#endif

#if FMT_SECURE_SCL
# include <iterator>
#endif

#ifdef _MSC_VER
# define FMT_MSC_VER _MSC_VER
#else
# define FMT_MSC_VER 0
#endif

#if FMT_MSC_VER && FMT_MSC_VER <= 1500
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
typedef __int64          intmax_t;
#else
#include <stdint.h>
#endif

#if !defined(FMT_HEADER_ONLY) && defined(_WIN32)
# ifdef FMT_EXPORT
#  define FMT_API __declspec(dllexport)
# elif defined(FMT_SHARED)
#  define FMT_API __declspec(dllimport)
# endif
#endif
#ifndef FMT_API
# define FMT_API
#endif

#ifdef __GNUC__
# define FMT_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
# define FMT_GCC_EXTENSION __extension__
# if FMT_GCC_VERSION >= 406
#  pragma GCC diagnostic push
// Disable the warning about "long long" which is sometimes reported even
// when using __extension__.
#  pragma GCC diagnostic ignored "-Wlong-long"
// Disable the warning about declaration shadowing because it affects too
// many valid cases.
#  pragma GCC diagnostic ignored "-Wshadow"
// Disable the warning about implicit conversions that may change the sign of
// an integer; silencing it otherwise would require many explicit casts.
#  pragma GCC diagnostic ignored "-Wsign-conversion"
# endif
# if __cplusplus >= 201103L || defined __GXX_EXPERIMENTAL_CXX0X__
#  define FMT_HAS_GXX_CXX11 1
# endif
#else
# define FMT_GCC_EXTENSION
#endif

#if defined(__INTEL_COMPILER)
# define FMT_ICC_VERSION __INTEL_COMPILER
#elif defined(__ICL)
# define FMT_ICC_VERSION __ICL
#endif

#if defined(__clang__) && !defined(FMT_ICC_VERSION)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
# pragma clang diagnostic ignored "-Wpadded"
#endif

#ifdef __GNUC_LIBSTD__
# define FMT_GNUC_LIBSTD_VERSION (__GNUC_LIBSTD__ * 100 + __GNUC_LIBSTD_MINOR__)
#endif

#ifdef __has_feature
# define FMT_HAS_FEATURE(x) __has_feature(x)
#else
# define FMT_HAS_FEATURE(x) 0
#endif

#ifdef __has_builtin
# define FMT_HAS_BUILTIN(x) __has_builtin(x)
#else
# define FMT_HAS_BUILTIN(x) 0
#endif

#ifdef __has_cpp_attribute
# define FMT_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
# define FMT_HAS_CPP_ATTRIBUTE(x) 0
#endif

#ifndef FMT_USE_VARIADIC_TEMPLATES
// Variadic templates are available in GCC since version 4.4
// (http://gcc.gnu.org/projects/cxx0x.html) and in Visual C++
// since version 2013.
# define FMT_USE_VARIADIC_TEMPLATES \
   (FMT_HAS_FEATURE(cxx_variadic_templates) || \
       (FMT_GCC_VERSION >= 404 && FMT_HAS_GXX_CXX11) || FMT_MSC_VER >= 1800)
#endif

#ifndef FMT_USE_RVALUE_REFERENCES
// Don't use rvalue references when compiling with clang and an old libstdc++
// as the latter doesn't provide std::move.
# if defined(FMT_GNUC_LIBSTD_VERSION) && FMT_GNUC_LIBSTD_VERSION <= 402
#  define FMT_USE_RVALUE_REFERENCES 0
# else
#  define FMT_USE_RVALUE_REFERENCES \
    (FMT_HAS_FEATURE(cxx_rvalue_references) || \
        (FMT_GCC_VERSION >= 403 && FMT_HAS_GXX_CXX11) || FMT_MSC_VER >= 1600)
# endif
#endif

#if FMT_USE_RVALUE_REFERENCES
# include <utility>  // for std::move
#endif

// Check if exceptions are disabled.
#if defined(__GNUC__) && !defined(__EXCEPTIONS)
# define FMT_EXCEPTIONS 0
#endif
#if FMT_MSC_VER && !_HAS_EXCEPTIONS
# define FMT_EXCEPTIONS 0
#endif
#ifndef FMT_EXCEPTIONS
# define FMT_EXCEPTIONS 1
#endif

#ifndef FMT_THROW
# if FMT_EXCEPTIONS
#  define FMT_THROW(x) throw x
# else
#  define FMT_THROW(x) assert(false)
# endif
#endif

// Define FMT_USE_NOEXCEPT to make fmt use noexcept (C++11 feature).
#ifndef FMT_USE_NOEXCEPT
# define FMT_USE_NOEXCEPT 0
#endif

#ifndef FMT_NOEXCEPT
# if FMT_EXCEPTIONS
#  if FMT_USE_NOEXCEPT || FMT_HAS_FEATURE(cxx_noexcept) || \
    (FMT_GCC_VERSION >= 408 && FMT_HAS_GXX_CXX11) || \
    FMT_MSC_VER >= 1900
#   define FMT_NOEXCEPT noexcept
#  else
#   define FMT_NOEXCEPT throw()
#  endif
# else
#  define FMT_NOEXCEPT
# endif
#endif

#ifndef FMT_OVERRIDE
# if FMT_USE_OVERRIDE || FMT_HAS_FEATURE(cxx_override) || \
   (FMT_GCC_VERSION >= 408 && FMT_HAS_GXX_CXX11) || \
   FMT_MSC_VER >= 1900
#  define FMT_OVERRIDE override
# else
#  define FMT_OVERRIDE
# endif
#endif


// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#ifndef FMT_USE_DELETED_FUNCTIONS
# define FMT_USE_DELETED_FUNCTIONS 0
#endif

#if FMT_USE_DELETED_FUNCTIONS || FMT_HAS_FEATURE(cxx_deleted_functions) || \
  (FMT_GCC_VERSION >= 404 && FMT_HAS_GXX_CXX11) || FMT_MSC_VER >= 1800
# define FMT_DELETED_OR_UNDEFINED  = delete
# define FMT_DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = delete; \
    TypeName& operator=(const TypeName&) = delete
#else
# define FMT_DELETED_OR_UNDEFINED
# define FMT_DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&); \
    TypeName& operator=(const TypeName&)
#endif

#ifndef FMT_USE_USER_DEFINED_LITERALS
// All compilers which support UDLs also support variadic templates. This
// makes the fmt::literals implementation easier. However, an explicit check
// for variadic templates is added here just in case.
// For Intel's compiler both it and the system gcc/msc must support UDLs.
# define FMT_USE_USER_DEFINED_LITERALS \
   FMT_USE_VARIADIC_TEMPLATES && FMT_USE_RVALUE_REFERENCES && \
   (FMT_HAS_FEATURE(cxx_user_literals) || \
     (FMT_GCC_VERSION >= 407 && FMT_HAS_GXX_CXX11) || FMT_MSC_VER >= 1900) && \
   (!defined(FMT_ICC_VERSION) || FMT_ICC_VERSION >= 1500)
#endif

#ifndef FMT_ASSERT
# define FMT_ASSERT(condition, message) assert((condition) && message)
#endif

#if FMT_GCC_VERSION >= 400 || FMT_HAS_BUILTIN(__builtin_clz)
# define FMT_BUILTIN_CLZ(n) __builtin_clz(n)
#endif

#if FMT_GCC_VERSION >= 400 || FMT_HAS_BUILTIN(__builtin_clzll)
# define FMT_BUILTIN_CLZLL(n) __builtin_clzll(n)
#endif

// Some compilers masquerade as both MSVC and GCC-likes or
// otherwise support __builtin_clz and __builtin_clzll, so
// only define FMT_BUILTIN_CLZ using the MSVC intrinsics
// if the clz and clzll builtins are not available.
#if FMT_MSC_VER && !defined(FMT_BUILTIN_CLZLL)
# include <intrin.h>  // _BitScanReverse, _BitScanReverse64

namespace fmt
{
namespace internal
{
# pragma intrinsic(_BitScanReverse)
inline uint32_t clz(uint32_t x)
{
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

# ifdef _WIN64
#  pragma intrinsic(_BitScanReverse64)
# endif

inline uint32_t clzll(uint64_t x)
{
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
}
#endif

namespace fmt
{
namespace internal
{
struct DummyInt
{
    int data[2];
    operator int() const
    {
        return 0;
    }
};
typedef std::numeric_limits<fmt::internal::DummyInt> FPUtil;

// Dummy implementations of system functions such as signbit and ecvt called
// if the latter are not available.
inline DummyInt signbit(...)
{
    return DummyInt();
}
inline DummyInt _ecvt_s(...)
{
    return DummyInt();
}
inline DummyInt isinf(...)
{
    return DummyInt();
}
inline DummyInt _finite(...)
{
    return DummyInt();
}
inline DummyInt isnan(...)
{
    return DummyInt();
}
inline DummyInt _isnan(...)
{
    return DummyInt();
}

// A helper function to suppress bogus "conditional expression is constant"
// warnings.
template <typename T>
inline T const_check(T value)
{
    return value;
}
}
}  // namespace fmt

namespace std
{
// Standard permits specialization of std::numeric_limits. This specialization
// is used to resolve ambiguity between isinf and std::isinf in glibc:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=48891
// and the same for isnan and signbit.
template <>
class numeric_limits<fmt::internal::DummyInt> :
    public std::numeric_limits<int>
{
public:
    // Portable version of isinf.
    template <typename T>
    static bool isinfinity(T x)
    {
        using namespace fmt::internal;
        // The resolution "priority" is:
        // isinf macro > std::isinf > ::isinf > fmt::internal::isinf
        if (const_check(sizeof(isinf(x)) == sizeof(bool) ||
                        sizeof(isinf(x)) == sizeof(int)))
        {
            return isinf(x) != 0;
        }
        return !_finite(static_cast<double>(x));
    }

    // Portable version of isnan.
    template <typename T>
    static bool isnotanumber(T x)
    {
        using namespace fmt::internal;
        if (const_check(sizeof(isnan(x)) == sizeof(bool) ||
                        sizeof(isnan(x)) == sizeof(int)))
        {
            return isnan(x) != 0;
        }
        return _isnan(static_cast<double>(x)) != 0;
    }

    // Portable version of signbit.
    static bool isnegative(double x)
    {
        using namespace fmt::internal;
        if (const_check(sizeof(signbit(x)) == sizeof(int)))
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

namespace fmt
{

// Fix the warning about long long on older versions of GCC
// that don't support the diagnostic pragma.
FMT_GCC_EXTENSION typedef long long LongLong;
FMT_GCC_EXTENSION typedef unsigned long long ULongLong;

#if FMT_USE_RVALUE_REFERENCES
using std::move;
#endif

template <typename Char>
class BasicWriter;

typedef BasicWriter<char> Writer;
typedef BasicWriter<wchar_t> WWriter;

template <typename Char>
class ArgFormatter;

template <typename CharType,
          typename ArgFormatter = fmt::ArgFormatter<CharType> >
class BasicFormatter;

/**
  \rst
  A string reference. It can be constructed from a C string or ``std::string``.

  You can use one of the following typedefs for common character types:

  +------------+-------------------------+
  | Type       | Definition              |
  +============+=========================+
  | StringRef  | BasicStringRef<char>    |
  +------------+-------------------------+
  | WStringRef | BasicStringRef<wchar_t> |
  +------------+-------------------------+

  This class is most useful as a parameter type to allow passing
  different types of strings to a function, for example::

    template <typename... Args>
    std::string format(StringRef format_str, const Args & ... args);

    format("{}", 42);
    format(std::string("{}"), 42);
  \endrst
 */
template <typename Char>
class BasicStringRef
{
private:
    const Char *data_;
    std::size_t size_;

public:
    /** Constructs a string reference object from a C string and a size. */
    BasicStringRef(const Char *s, std::size_t size) : data_(s), size_(size) {}

    /**
      \rst
      Constructs a string reference object from a C string computing
      the size with ``std::char_traits<Char>::length``.
      \endrst
     */
    BasicStringRef(const Char *s)
        : data_(s), size_(std::char_traits<Char>::length(s)) {}

    /**
      \rst
      Constructs a string reference from an ``std::string`` object.
      \endrst
     */
    BasicStringRef(const std::basic_string<Char> &s)
        : data_(s.c_str()), size_(s.size()) {}

    /**
      \rst
      Converts a string reference to an ``std::string`` object.
      \endrst
     */
    std::basic_string<Char> to_string() const
    {
        return std::basic_string<Char>(data_, size_);
    }

    /** Returns a pointer to the string data. */
    const Char *data() const
    {
        return data_;
    }

    /** Returns the string size. */
    std::size_t size() const
    {
        return size_;
    }

    // Lexicographically compare this string reference to other.
    int compare(BasicStringRef other) const
    {
        std::size_t size = size_ < other.size_ ? size_ : other.size_;
        int result = std::char_traits<Char>::compare(data_, other.data_, size);
        if (result == 0)
            result = size_ == other.size_ ? 0 : (size_ < other.size_ ? -1 : 1);
        return result;
    }

    friend bool operator==(BasicStringRef lhs, BasicStringRef rhs)
    {
        return lhs.compare(rhs) == 0;
    }
    friend bool operator!=(BasicStringRef lhs, BasicStringRef rhs)
    {
        return lhs.compare(rhs) != 0;
    }
    friend bool operator<(BasicStringRef lhs, BasicStringRef rhs)
    {
        return lhs.compare(rhs) < 0;
    }
    friend bool operator<=(BasicStringRef lhs, BasicStringRef rhs)
    {
        return lhs.compare(rhs) <= 0;
    }
    friend bool operator>(BasicStringRef lhs, BasicStringRef rhs)
    {
        return lhs.compare(rhs) > 0;
    }
    friend bool operator>=(BasicStringRef lhs, BasicStringRef rhs)
    {
        return lhs.compare(rhs) >= 0;
    }
};

typedef BasicStringRef<char> StringRef;
typedef BasicStringRef<wchar_t> WStringRef;

/**
  \rst
  A reference to a null terminated string. It can be constructed from a C
  string or ``std::string``.

  You can use one of the following typedefs for common character types:

  +-------------+--------------------------+
  | Type        | Definition               |
  +=============+==========================+
  | CStringRef  | BasicCStringRef<char>    |
  +-------------+--------------------------+
  | WCStringRef | BasicCStringRef<wchar_t> |
  +-------------+--------------------------+

  This class is most useful as a parameter type to allow passing
  different types of strings to a function, for example::

    template <typename... Args>
    std::string format(CStringRef format_str, const Args & ... args);

    format("{}", 42);
    format(std::string("{}"), 42);
  \endrst
 */
template <typename Char>
class BasicCStringRef
{
private:
    const Char *data_;

public:
    /** Constructs a string reference object from a C string. */
    BasicCStringRef(const Char *s) : data_(s) {}

    /**
      \rst
      Constructs a string reference from an ``std::string`` object.
      \endrst
     */
    BasicCStringRef(const std::basic_string<Char> &s) : data_(s.c_str()) {}

    /** Returns the pointer to a C string. */
    const Char *c_str() const
    {
        return data_;
    }
};

typedef BasicCStringRef<char> CStringRef;
typedef BasicCStringRef<wchar_t> WCStringRef;

/** A formatting error such as invalid format string. */
class FormatError : public std::runtime_error
{
public:
    explicit FormatError(CStringRef message)
        : std::runtime_error(message.c_str()) {}
    ~FormatError() throw();
};

namespace internal
{

// MakeUnsigned<T>::Type gives an unsigned type corresponding to integer type T.
template <typename T>
struct MakeUnsigned
{
    typedef T Type;
};

#define FMT_SPECIALIZE_MAKE_UNSIGNED(T, U) \
  template <> \
  struct MakeUnsigned<T> { typedef U Type; }

FMT_SPECIALIZE_MAKE_UNSIGNED(char, unsigned char);
FMT_SPECIALIZE_MAKE_UNSIGNED(signed char, unsigned char);
FMT_SPECIALIZE_MAKE_UNSIGNED(short, unsigned short);
FMT_SPECIALIZE_MAKE_UNSIGNED(int, unsigned);
FMT_SPECIALIZE_MAKE_UNSIGNED(long, unsigned long);
FMT_SPECIALIZE_MAKE_UNSIGNED(LongLong, ULongLong);

// Casts nonnegative integer to unsigned.
template <typename Int>
inline typename MakeUnsigned<Int>::Type to_unsigned(Int value)
{
    FMT_ASSERT(value >= 0, "negative value");
    return static_cast<typename MakeUnsigned<Int>::Type>(value);
}

// The number of characters to store in the MemoryBuffer object itself
// to avoid dynamic memory allocation.
enum { INLINE_BUFFER_SIZE = 500 };

#if FMT_SECURE_SCL
// Use checked iterator to avoid warnings on MSVC.
template <typename T>
inline stdext::checked_array_iterator<T*> make_ptr(T *ptr, std::size_t size)
{
    return stdext::checked_array_iterator<T*>(ptr, size);
}
#else
template <typename T>
inline T *make_ptr(T *ptr, std::size_t)
{
    return ptr;
}
#endif
}  // namespace internal

/**
  \rst
  A buffer supporting a subset of ``std::vector``'s operations.
  \endrst
 */
template <typename T>
class Buffer
{
private:
    FMT_DISALLOW_COPY_AND_ASSIGN(Buffer);

protected:
    T *ptr_;
    std::size_t size_;
    std::size_t capacity_;

    Buffer(T *ptr = 0, std::size_t capacity = 0)
        : ptr_(ptr), size_(0), capacity_(capacity) {}

    /**
      \rst
      Increases the buffer capacity to hold at least *size* elements updating
      ``ptr_`` and ``capacity_``.
      \endrst
     */
    virtual void grow(std::size_t size) = 0;

public:
    virtual ~Buffer() {}

    /** Returns the size of this buffer. */
    std::size_t size() const
    {
        return size_;
    }

    /** Returns the capacity of this buffer. */
    std::size_t capacity() const
    {
        return capacity_;
    }

    /**
      Resizes the buffer. If T is a POD type new elements may not be initialized.
     */
    void resize(std::size_t new_size)
    {
        if (new_size > capacity_)
            grow(new_size);
        size_ = new_size;
    }

    /**
      \rst
      Reserves space to store at least *capacity* elements.
      \endrst
     */
    void reserve(std::size_t capacity)
    {
        if (capacity > capacity_)
            grow(capacity);
    }

    void clear() FMT_NOEXCEPT { size_ = 0; }

    void push_back(const T &value)
    {
        if (size_ == capacity_)
            grow(size_ + 1);
        ptr_[size_++] = value;
    }

    /** Appends data to the end of the buffer. */
    template <typename U>
    void append(const U *begin, const U *end);

    T &operator[](std::size_t index)
    {
        return ptr_[index];
    }
    const T &operator[](std::size_t index) const
    {
        return ptr_[index];
    }
};

template <typename T>
template <typename U>
void Buffer<T>::append(const U *begin, const U *end)
{
    std::size_t new_size = size_ + internal::to_unsigned(end - begin);
    if (new_size > capacity_)
        grow(new_size);
    std::uninitialized_copy(begin, end,
                            internal::make_ptr(ptr_, capacity_) + size_);
    size_ = new_size;
}

namespace internal
{

// A memory buffer for trivially copyable/constructible types with the first
// SIZE elements stored in the object itself.
template <typename T, std::size_t SIZE, typename Allocator = std::allocator<T> >
class MemoryBuffer : private Allocator, public Buffer<T>
{
private:
    T data_[SIZE];

    // Deallocate memory allocated by the buffer.
    void deallocate()
    {
        if (this->ptr_ != data_) Allocator::deallocate(this->ptr_, this->capacity_);
    }

protected:
    void grow(std::size_t size) FMT_OVERRIDE;

public:
    explicit MemoryBuffer(const Allocator &alloc = Allocator())
        : Allocator(alloc), Buffer<T>(data_, SIZE) {}
    ~MemoryBuffer()
    {
        deallocate();
    }

#if FMT_USE_RVALUE_REFERENCES
private:
    // Move data from other to this buffer.
    void move(MemoryBuffer &other)
    {
        Allocator &this_alloc = *this, &other_alloc = other;
        this_alloc = std::move(other_alloc);
        this->size_ = other.size_;
        this->capacity_ = other.capacity_;
        if (other.ptr_ == other.data_)
        {
            this->ptr_ = data_;
            std::uninitialized_copy(other.data_, other.data_ + this->size_,
                                    make_ptr(data_, this->capacity_));
        }
        else
        {
            this->ptr_ = other.ptr_;
            // Set pointer to the inline array so that delete is not called
            // when deallocating.
            other.ptr_ = other.data_;
        }
    }

public:
    MemoryBuffer(MemoryBuffer &&other)
    {
        move(other);
    }

    MemoryBuffer &operator=(MemoryBuffer &&other)
    {
        assert(this != &other);
        deallocate();
        move(other);
        return *this;
    }
#endif

    // Returns a copy of the allocator associated with this buffer.
    Allocator get_allocator() const
    {
        return *this;
    }
};

template <typename T, std::size_t SIZE, typename Allocator>
void MemoryBuffer<T, SIZE, Allocator>::grow(std::size_t size)
{
    std::size_t new_capacity = this->capacity_ + this->capacity_ / 2;
    if (size > new_capacity)
        new_capacity = size;
    T *new_ptr = this->allocate(new_capacity);
    // The following code doesn't throw, so the raw pointer above doesn't leak.
    std::uninitialized_copy(this->ptr_, this->ptr_ + this->size_,
                            make_ptr(new_ptr, new_capacity));
    std::size_t old_capacity = this->capacity_;
    T *old_ptr = this->ptr_;
    this->capacity_ = new_capacity;
    this->ptr_ = new_ptr;
    // deallocate may throw (at least in principle), but it doesn't matter since
    // the buffer already uses the new storage and will deallocate it in case
    // of exception.
    if (old_ptr != data_)
        Allocator::deallocate(old_ptr, old_capacity);
}

// A fixed-size buffer.
template <typename Char>
class FixedBuffer : public fmt::Buffer<Char>
{
public:
    FixedBuffer(Char *array, std::size_t size) : fmt::Buffer<Char>(array, size) {}

protected:
    FMT_API void grow(std::size_t size);
};

template <typename Char>
class BasicCharTraits
{
public:
#if FMT_SECURE_SCL
    typedef stdext::checked_array_iterator<Char*> CharPtr;
#else
    typedef Char *CharPtr;
#endif
    static Char cast(int value)
    {
        return static_cast<Char>(value);
    }
};

template <typename Char>
class CharTraits;

template <>
class CharTraits<char> : public BasicCharTraits<char>
{
private:
    // Conversion from wchar_t to char is not allowed.
    static char convert(wchar_t);

public:
    static char convert(char value)
    {
        return value;
    }

    // Formats a floating-point number.
    template <typename T>
    FMT_API static int format_float(char *buffer, std::size_t size,
                                    const char *format, unsigned width, int precision, T value);
};

template <>
class CharTraits<wchar_t> : public BasicCharTraits<wchar_t>
{
public:
    static wchar_t convert(char value)
    {
        return value;
    }
    static wchar_t convert(wchar_t value)
    {
        return value;
    }

    template <typename T>
    FMT_API static int format_float(wchar_t *buffer, std::size_t size,
                                    const wchar_t *format, unsigned width, int precision, T value);
};

// Checks if a number is negative - used to avoid warnings.
template <bool IsSigned>
struct SignChecker
{
    template <typename T>
    static bool is_negative(T value)
    {
        return value < 0;
    }
};

template <>
struct SignChecker<false>
{
    template <typename T>
    static bool is_negative(T)
    {
        return false;
    }
};

// Returns true if value is negative, false otherwise.
// Same as (value < 0) but doesn't produce warnings if T is an unsigned type.
template <typename T>
inline bool is_negative(T value)
{
    return SignChecker<std::numeric_limits<T>::is_signed>::is_negative(value);
}

// Selects uint32_t if FitsIn32Bits is true, uint64_t otherwise.
template <bool FitsIn32Bits>
struct TypeSelector
{
    typedef uint32_t Type;
};

template <>
struct TypeSelector<false>
{
    typedef uint64_t Type;
};

template <typename T>
struct IntTraits
{
    // Smallest of uint32_t and uint64_t that is large enough to represent
    // all values of T.
    typedef typename
    TypeSelector<std::numeric_limits<T>::digits <= 32>::Type MainType;
};

FMT_API void report_unknown_type(char code, const char *type);

// Static data is placed in this class template to allow header-only
// configuration.
template <typename T = void>
struct FMT_API BasicData
{
    static const uint32_t POWERS_OF_10_32[];
    static const uint64_t POWERS_OF_10_64[];
    static const char DIGITS[];
};

#ifndef FMT_USE_EXTERN_TEMPLATES
// Clang doesn't have a feature check for extern templates so we check
// for variadic templates which were introduced in the same version.
# define FMT_USE_EXTERN_TEMPLATES (__clang__ && FMT_USE_VARIADIC_TEMPLATES)
#endif

#if FMT_USE_EXTERN_TEMPLATES && !defined(FMT_HEADER_ONLY)
extern template struct BasicData<void>;
#endif

typedef BasicData<> Data;

#ifdef FMT_BUILTIN_CLZLL
// Returns the number of decimal digits in n. Leading zeros are not counted
// except for n == 0 in which case count_digits returns 1.
inline unsigned count_digits(uint64_t n)
{
    // Based on http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
    // and the benchmark https://github.com/localvoid/cxx-benchmark-count-digits.
    int t = (64 - FMT_BUILTIN_CLZLL(n | 1)) * 1233 >> 12;
    return to_unsigned(t) - (n < Data::POWERS_OF_10_64[t]) + 1;
}
#else
// Fallback version of count_digits used when __builtin_clz is not available.
inline unsigned count_digits(uint64_t n)
{
    unsigned count = 1;
    for (;;)
    {
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

#ifdef FMT_BUILTIN_CLZ
// Optional version of count_digits for better performance on 32-bit platforms.
inline unsigned count_digits(uint32_t n)
{
    int t = (32 - FMT_BUILTIN_CLZ(n | 1)) * 1233 >> 12;
    return to_unsigned(t) - (n < Data::POWERS_OF_10_32[t]) + 1;
}
#endif

// A functor that doesn't add a thousands separator.
struct NoThousandsSep
{
    template <typename Char>
    void operator()(Char *) {}
};

// A functor that adds a thousands separator.
class ThousandsSep
{
private:
    fmt::StringRef sep_;

    // Index of a decimal digit with the least significant digit having index 0.
    unsigned digit_index_;

public:
    explicit ThousandsSep(fmt::StringRef sep) : sep_(sep), digit_index_(0) {}

    template <typename Char>
    void operator()(Char *&buffer)
    {
        if (++digit_index_ % 3 != 0)
            return;
        buffer -= sep_.size();
        std::uninitialized_copy(sep_.data(), sep_.data() + sep_.size(),
                                internal::make_ptr(buffer, sep_.size()));
    }
};

// Formats a decimal unsigned integer value writing into buffer.
// thousands_sep is a functor that is called after writing each char to
// add a thousands separator if necessary.
template <typename UInt, typename Char, typename ThousandsSep>
inline void format_decimal(Char *buffer, UInt value, unsigned num_digits,
                           ThousandsSep thousands_sep)
{
    buffer += num_digits;
    while (value >= 100)
    {
        // Integer division is slow so do it for a group of two digits instead
        // of for every digit. The idea comes from the talk by Alexandrescu
        // "Three Optimization Tips for C++". See speed-test for a comparison.
        unsigned index = static_cast<unsigned>((value % 100) * 2);
        value /= 100;
        *--buffer = Data::DIGITS[index + 1];
        thousands_sep(buffer);
        *--buffer = Data::DIGITS[index];
        thousands_sep(buffer);
    }
    if (value < 10)
    {
        *--buffer = static_cast<char>('0' + value);
        return;
    }
    unsigned index = static_cast<unsigned>(value * 2);
    *--buffer = Data::DIGITS[index + 1];
    thousands_sep(buffer);
    *--buffer = Data::DIGITS[index];
}

template <typename UInt, typename Char>
inline void format_decimal(Char *buffer, UInt value, unsigned num_digits)
{
    return format_decimal(buffer, value, num_digits, NoThousandsSep());
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
class UTF8ToUTF16
{
private:
    MemoryBuffer<wchar_t, INLINE_BUFFER_SIZE> buffer_;

public:
    FMT_API explicit UTF8ToUTF16(StringRef s);
    operator WStringRef() const
    {
        return WStringRef(&buffer_[0], size());
    }
    size_t size() const
    {
        return buffer_.size() - 1;
    }
    const wchar_t *c_str() const
    {
        return &buffer_[0];
    }
    std::wstring str() const
    {
        return std::wstring(&buffer_[0], size());
    }
};

// A converter from UTF-16 to UTF-8.
// It is only provided for Windows since other systems support UTF-8 natively.
class UTF16ToUTF8
{
private:
    MemoryBuffer<char, INLINE_BUFFER_SIZE> buffer_;

public:
    UTF16ToUTF8() {}
    FMT_API explicit UTF16ToUTF8(WStringRef s);
    operator StringRef() const
    {
        return StringRef(&buffer_[0], size());
    }
    size_t size() const
    {
        return buffer_.size() - 1;
    }
    const char *c_str() const
    {
        return &buffer_[0];
    }
    std::string str() const
    {
        return std::string(&buffer_[0], size());
    }

    // Performs conversion returning a system error code instead of
    // throwing exception on conversion error. This method may still throw
    // in case of memory allocation error.
    FMT_API int convert(WStringRef s);
};

FMT_API void format_windows_error(fmt::Writer &out, int error_code,
                                  fmt::StringRef message) FMT_NOEXCEPT;
#endif

FMT_API void format_system_error(fmt::Writer &out, int error_code,
                                 fmt::StringRef message) FMT_NOEXCEPT;

// A formatting argument value.
struct Value
{
    template <typename Char>
    struct StringValue
    {
        const Char *value;
        std::size_t size;
    };

    typedef void (*FormatFunc)(
        void *formatter, const void *arg, void *format_str_ptr);

    struct CustomValue
    {
        const void *value;
        FormatFunc format;
    };

    union
    {
        int int_value;
        unsigned uint_value;
        LongLong long_long_value;
        ULongLong ulong_long_value;
        double double_value;
        long double long_double_value;
        const void *pointer;
        StringValue<char> string;
        StringValue<signed char> sstring;
        StringValue<unsigned char> ustring;
        StringValue<wchar_t> wstring;
        CustomValue custom;
    };

    enum Type
    {
        NONE, NAMED_ARG,
        // Integer types should go first,
        INT, UINT, LONG_LONG, ULONG_LONG, BOOL, CHAR, LAST_INTEGER_TYPE = CHAR,
        // followed by floating-point types.
        DOUBLE, LONG_DOUBLE, LAST_NUMERIC_TYPE = LONG_DOUBLE,
        CSTRING, STRING, WSTRING, POINTER, CUSTOM
    };
};

// A formatting argument. It is a trivially copyable/constructible type to
// allow storage in internal::MemoryBuffer.
struct Arg : Value
{
    Type type;
};

template <typename Char>
struct NamedArg;

template <typename T = void>
struct Null {};

// A helper class template to enable or disable overloads taking wide
// characters and strings in MakeValue.
template <typename T, typename Char>
struct WCharHelper
{
    typedef Null<T> Supported;
    typedef T Unsupported;
};

template <typename T>
struct WCharHelper<T, wchar_t>
{
    typedef T Supported;
    typedef Null<T> Unsupported;
};

typedef char Yes[1];
typedef char No[2];

template <typename T>
T &get();

// These are non-members to workaround an overload resolution bug in bcc32.
Yes &convert(fmt::ULongLong);
No &convert(...);

template<typename T, bool ENABLE_CONVERSION>
struct ConvertToIntImpl
{
    enum { value = ENABLE_CONVERSION };
};

template<typename T, bool ENABLE_CONVERSION>
struct ConvertToIntImpl2
{
    enum { value = false };
};

template<typename T>
struct ConvertToIntImpl2<T, true>
{
    enum
    {
        // Don't convert numeric types.
        value = ConvertToIntImpl<T, !std::numeric_limits<T>::is_specialized>::value
    };
};

template<typename T>
struct ConvertToInt
{
    enum { enable_conversion = sizeof(convert(get<T>())) == sizeof(Yes) };
    enum { value = ConvertToIntImpl2<T, enable_conversion>::value };
};

#define FMT_DISABLE_CONVERSION_TO_INT(Type) \
  template <> \
  struct ConvertToInt<Type> {  enum { value = 0 }; }

// Silence warnings about convering float to int.
FMT_DISABLE_CONVERSION_TO_INT(float);
FMT_DISABLE_CONVERSION_TO_INT(double);
FMT_DISABLE_CONVERSION_TO_INT(long double);

template<bool B, class T = void>
struct EnableIf {};

template<class T>
struct EnableIf<true, T>
{
    typedef T type;
};

template<bool B, class T, class F>
struct Conditional
{
    typedef T type;
};

template<class T, class F>
struct Conditional<false, T, F>
{
    typedef F type;
};

// For bcc32 which doesn't understand ! in template arguments.
template<bool>
struct Not
{
    enum { value = 0 };
};

template<>
struct Not<false>
{
    enum { value = 1 };
};

template<typename T, T> struct LConvCheck
{
    LConvCheck(int) {}
};

// Returns the thousands separator for the current locale.
// We check if ``lconv`` contains ``thousands_sep`` because on Android
// ``lconv`` is stubbed as an empty struct.
template <typename LConv>
inline StringRef thousands_sep(
    LConv *lc, LConvCheck<char *LConv::*, &LConv::thousands_sep> = 0)
{
    return lc->thousands_sep;
}

inline fmt::StringRef thousands_sep(...)
{
    return "";
}

// Makes an Arg object from any type.
template <typename Formatter>
class MakeValue : public Arg
{
public:
    typedef typename Formatter::Char Char;

private:
    // The following two methods are private to disallow formatting of
    // arbitrary pointers. If you want to output a pointer cast it to
    // "void *" or "const void *". In particular, this forbids formatting
    // of "[const] volatile char *" which is printed as bool by iostreams.
    // Do not implement!
    template <typename T>
    MakeValue(const T *value);
    template <typename T>
    MakeValue(T *value);

    // The following methods are private to disallow formatting of wide
    // characters and strings into narrow strings as in
    //   fmt::format("{}", L"test");
    // To fix this, use a wide format string: fmt::format(L"{}", L"test").
#if !FMT_MSC_VER || defined(_NATIVE_WCHAR_T_DEFINED)
    MakeValue(typename WCharHelper<wchar_t, Char>::Unsupported);
#endif
    MakeValue(typename WCharHelper<wchar_t *, Char>::Unsupported);
    MakeValue(typename WCharHelper<const wchar_t *, Char>::Unsupported);
    MakeValue(typename WCharHelper<const std::wstring &, Char>::Unsupported);
    MakeValue(typename WCharHelper<WStringRef, Char>::Unsupported);

    void set_string(StringRef str)
    {
        string.value = str.data();
        string.size = str.size();
    }

    void set_string(WStringRef str)
    {
        wstring.value = str.data();
        wstring.size = str.size();
    }

    // Formats an argument of a custom type, such as a user-defined class.
    template <typename T>
    static void format_custom_arg(
        void *formatter, const void *arg, void *format_str_ptr)
    {
        format(*static_cast<Formatter*>(formatter),
               *static_cast<const Char**>(format_str_ptr),
               *static_cast<const T*>(arg));
    }

public:
    MakeValue() {}

#define FMT_MAKE_VALUE_(Type, field, TYPE, rhs) \
  MakeValue(Type value) { field = rhs; } \
  static uint64_t type(Type) { return Arg::TYPE; }

#define FMT_MAKE_VALUE(Type, field, TYPE) \
  FMT_MAKE_VALUE_(Type, field, TYPE, value)

    FMT_MAKE_VALUE(bool, int_value, BOOL)
    FMT_MAKE_VALUE(short, int_value, INT)
    FMT_MAKE_VALUE(unsigned short, uint_value, UINT)
    FMT_MAKE_VALUE(int, int_value, INT)
    FMT_MAKE_VALUE(unsigned, uint_value, UINT)

    MakeValue(long value)
    {
        // To minimize the number of types we need to deal with, long is
        // translated either to int or to long long depending on its size.
        if (const_check(sizeof(long) == sizeof(int)))
            int_value = static_cast<int>(value);
        else
            long_long_value = value;
    }
    static uint64_t type(long)
    {
        return sizeof(long) == sizeof(int) ? Arg::INT : Arg::LONG_LONG;
    }

    MakeValue(unsigned long value)
    {
        if (const_check(sizeof(unsigned long) == sizeof(unsigned)))
            uint_value = static_cast<unsigned>(value);
        else
            ulong_long_value = value;
    }
    static uint64_t type(unsigned long)
    {
        return sizeof(unsigned long) == sizeof(unsigned) ?
               Arg::UINT : Arg::ULONG_LONG;
    }

    FMT_MAKE_VALUE(LongLong, long_long_value, LONG_LONG)
    FMT_MAKE_VALUE(ULongLong, ulong_long_value, ULONG_LONG)
    FMT_MAKE_VALUE(float, double_value, DOUBLE)
    FMT_MAKE_VALUE(double, double_value, DOUBLE)
    FMT_MAKE_VALUE(long double, long_double_value, LONG_DOUBLE)
    FMT_MAKE_VALUE(signed char, int_value, INT)
    FMT_MAKE_VALUE(unsigned char, uint_value, UINT)
    FMT_MAKE_VALUE(char, int_value, CHAR)

#if !defined(_MSC_VER) || defined(_NATIVE_WCHAR_T_DEFINED)
    MakeValue(typename WCharHelper<wchar_t, Char>::Supported value)
    {
        int_value = value;
    }
    static uint64_t type(wchar_t)
    {
        return Arg::CHAR;
    }
#endif

#define FMT_MAKE_STR_VALUE(Type, TYPE) \
  MakeValue(Type value) { set_string(value); } \
  static uint64_t type(Type) { return Arg::TYPE; }

    FMT_MAKE_VALUE(char *, string.value, CSTRING)
    FMT_MAKE_VALUE(const char *, string.value, CSTRING)
    FMT_MAKE_VALUE(signed char *, sstring.value, CSTRING)
    FMT_MAKE_VALUE(const signed char *, sstring.value, CSTRING)
    FMT_MAKE_VALUE(unsigned char *, ustring.value, CSTRING)
    FMT_MAKE_VALUE(const unsigned char *, ustring.value, CSTRING)
    FMT_MAKE_STR_VALUE(const std::string &, STRING)
    FMT_MAKE_STR_VALUE(StringRef, STRING)
    FMT_MAKE_VALUE_(CStringRef, string.value, CSTRING, value.c_str())

#define FMT_MAKE_WSTR_VALUE(Type, TYPE) \
  MakeValue(typename WCharHelper<Type, Char>::Supported value) { \
    set_string(value); \
  } \
  static uint64_t type(Type) { return Arg::TYPE; }

    FMT_MAKE_WSTR_VALUE(wchar_t *, WSTRING)
    FMT_MAKE_WSTR_VALUE(const wchar_t *, WSTRING)
    FMT_MAKE_WSTR_VALUE(const std::wstring &, WSTRING)
    FMT_MAKE_WSTR_VALUE(WStringRef, WSTRING)

    FMT_MAKE_VALUE(void *, pointer, POINTER)
    FMT_MAKE_VALUE(const void *, pointer, POINTER)

    template <typename T>
    MakeValue(const T &value,
              typename EnableIf<Not<
              ConvertToInt<T>::value>::value, int>::type = 0)
    {
        custom.value = &value;
        custom.format = &format_custom_arg<T>;
    }

    template <typename T>
    MakeValue(const T &value,
              typename EnableIf<ConvertToInt<T>::value, int>::type = 0)
    {
        int_value = value;
    }

    template <typename T>
    static uint64_t type(const T &)
    {
        return ConvertToInt<T>::value ? Arg::INT : Arg::CUSTOM;
    }

    // Additional template param `Char_` is needed here because make_type always
    // uses char.
    template <typename Char_>
    MakeValue(const NamedArg<Char_> &value)
    {
        pointer = &value;
    }

    template <typename Char_>
    static uint64_t type(const NamedArg<Char_> &)
    {
        return Arg::NAMED_ARG;
    }
};

template <typename Formatter>
class MakeArg : public Arg
{
public:
    MakeArg()
    {
        type = Arg::NONE;
    }

    template <typename T>
    MakeArg(const T &value)
        : Arg(MakeValue<Formatter>(value))
    {
        type = static_cast<Arg::Type>(MakeValue<Formatter>::type(value));
    }
};

template <typename Char>
struct NamedArg : Arg
{
    BasicStringRef<Char> name;

    template <typename T>
    NamedArg(BasicStringRef<Char> argname, const T &value)
        : Arg(MakeArg< BasicFormatter<Char> >(value)), name(argname) {}
};

class RuntimeError : public std::runtime_error
{
protected:
    RuntimeError() : std::runtime_error("") {}
    ~RuntimeError() throw();
};

template <typename Char>
class PrintfArgFormatter;

template <typename Char>
class ArgMap;
}  // namespace internal

/** An argument list. */
class ArgList
{
private:
    // To reduce compiled code size per formatting function call, types of first
    // MAX_PACKED_ARGS arguments are passed in the types_ field.
    uint64_t types_;
    union
    {
        // If the number of arguments is less than MAX_PACKED_ARGS, the argument
        // values are stored in values_, otherwise they are stored in args_.
        // This is done to reduce compiled code size as storing larger objects
        // may require more code (at least on x86-64) even if the same amount of
        // data is actually copied to stack. It saves ~10% on the bloat test.
        const internal::Value *values_;
        const internal::Arg *args_;
    };

    internal::Arg::Type type(unsigned index) const
    {
        unsigned shift = index * 4;
        uint64_t mask = 0xf;
        return static_cast<internal::Arg::Type>(
                   (types_ & (mask << shift)) >> shift);
    }

    template <typename Char>
    friend class internal::ArgMap;

public:
    // Maximum number of arguments with packed types.
    enum { MAX_PACKED_ARGS = 16 };

    ArgList() : types_(0) {}

    ArgList(ULongLong types, const internal::Value *values)
        : types_(types), values_(values) {}
    ArgList(ULongLong types, const internal::Arg *args)
        : types_(types), args_(args) {}

    /** Returns the argument at specified index. */
    internal::Arg operator[](unsigned index) const
    {
        using internal::Arg;
        Arg arg;
        bool use_values = type(MAX_PACKED_ARGS - 1) == Arg::NONE;
        if (index < MAX_PACKED_ARGS)
        {
            Arg::Type arg_type = type(index);
            internal::Value &val = arg;
            if (arg_type != Arg::NONE)
                val = use_values ? values_[index] : args_[index];
            arg.type = arg_type;
            return arg;
        }
        if (use_values)
        {
            // The index is greater than the number of arguments that can be stored
            // in values, so return a "none" argument.
            arg.type = Arg::NONE;
            return arg;
        }
        for (unsigned i = MAX_PACKED_ARGS; i <= index; ++i)
        {
            if (args_[i].type == Arg::NONE)
                return args_[i];
        }
        return args_[index];
    }
};

#define FMT_DISPATCH(call) static_cast<Impl*>(this)->call

/**
  \rst
  An argument visitor based on the `curiously recurring template pattern
  <http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern>`_.

  To use `~fmt::ArgVisitor` define a subclass that implements some or all of the
  visit methods with the same signatures as the methods in `~fmt::ArgVisitor`,
  for example, `~fmt::ArgVisitor::visit_int()`.
  Pass the subclass as the *Impl* template parameter. Then calling
  `~fmt::ArgVisitor::visit` for some argument will dispatch to a visit method
  specific to the argument type. For example, if the argument type is
  ``double`` then the `~fmt::ArgVisitor::visit_double()` method of a subclass
  will be called. If the subclass doesn't contain a method with this signature,
  then a corresponding method of `~fmt::ArgVisitor` will be called.

  **Example**::

    class MyArgVisitor : public fmt::ArgVisitor<MyArgVisitor, void> {
     public:
      void visit_int(int value) { fmt::print("{}", value); }
      void visit_double(double value) { fmt::print("{}", value ); }
    };
  \endrst
 */
template <typename Impl, typename Result>
class ArgVisitor
{
private:
    typedef internal::Arg Arg;

public:
    void report_unhandled_arg() {}

    Result visit_unhandled_arg()
    {
        FMT_DISPATCH(report_unhandled_arg());
        return Result();
    }

    /** Visits an ``int`` argument. **/
    Result visit_int(int value)
    {
        return FMT_DISPATCH(visit_any_int(value));
    }

    /** Visits a ``long long`` argument. **/
    Result visit_long_long(LongLong value)
    {
        return FMT_DISPATCH(visit_any_int(value));
    }

    /** Visits an ``unsigned`` argument. **/
    Result visit_uint(unsigned value)
    {
        return FMT_DISPATCH(visit_any_int(value));
    }

    /** Visits an ``unsigned long long`` argument. **/
    Result visit_ulong_long(ULongLong value)
    {
        return FMT_DISPATCH(visit_any_int(value));
    }

    /** Visits a ``bool`` argument. **/
    Result visit_bool(bool value)
    {
        return FMT_DISPATCH(visit_any_int(value));
    }

    /** Visits a ``char`` or ``wchar_t`` argument. **/
    Result visit_char(int value)
    {
        return FMT_DISPATCH(visit_any_int(value));
    }

    /** Visits an argument of any integral type. **/
    template <typename T>
    Result visit_any_int(T)
    {
        return FMT_DISPATCH(visit_unhandled_arg());
    }

    /** Visits a ``double`` argument. **/
    Result visit_double(double value)
    {
        return FMT_DISPATCH(visit_any_double(value));
    }

    /** Visits a ``long double`` argument. **/
    Result visit_long_double(long double value)
    {
        return FMT_DISPATCH(visit_any_double(value));
    }

    /** Visits a ``double`` or ``long double`` argument. **/
    template <typename T>
    Result visit_any_double(T)
    {
        return FMT_DISPATCH(visit_unhandled_arg());
    }

    /** Visits a null-terminated C string (``const char *``) argument. **/
    Result visit_cstring(const char *)
    {
        return FMT_DISPATCH(visit_unhandled_arg());
    }

    /** Visits a string argument. **/
    Result visit_string(Arg::StringValue<char>)
    {
        return FMT_DISPATCH(visit_unhandled_arg());
    }

    /** Visits a wide string argument. **/
    Result visit_wstring(Arg::StringValue<wchar_t>)
    {
        return FMT_DISPATCH(visit_unhandled_arg());
    }

    /** Visits a pointer argument. **/
    Result visit_pointer(const void *)
    {
        return FMT_DISPATCH(visit_unhandled_arg());
    }

    /** Visits an argument of a custom (user-defined) type. **/
    Result visit_custom(Arg::CustomValue)
    {
        return FMT_DISPATCH(visit_unhandled_arg());
    }

    /**
      \rst
      Visits an argument dispatching to the appropriate visit method based on
      the argument type. For example, if the argument type is ``double`` then
      the `~fmt::ArgVisitor::visit_double()` method of the *Impl* class will be
      called.
      \endrst
     */
    Result visit(const Arg &arg)
    {
        switch (arg.type)
        {
        case Arg::NONE:
        case Arg::NAMED_ARG:
            FMT_ASSERT(false, "invalid argument type");
            break;
        case Arg::INT:
            return FMT_DISPATCH(visit_int(arg.int_value));
        case Arg::UINT:
            return FMT_DISPATCH(visit_uint(arg.uint_value));
        case Arg::LONG_LONG:
            return FMT_DISPATCH(visit_long_long(arg.long_long_value));
        case Arg::ULONG_LONG:
            return FMT_DISPATCH(visit_ulong_long(arg.ulong_long_value));
        case Arg::BOOL:
            return FMT_DISPATCH(visit_bool(arg.int_value != 0));
        case Arg::CHAR:
            return FMT_DISPATCH(visit_char(arg.int_value));
        case Arg::DOUBLE:
            return FMT_DISPATCH(visit_double(arg.double_value));
        case Arg::LONG_DOUBLE:
            return FMT_DISPATCH(visit_long_double(arg.long_double_value));
        case Arg::CSTRING:
            return FMT_DISPATCH(visit_cstring(arg.string.value));
        case Arg::STRING:
            return FMT_DISPATCH(visit_string(arg.string));
        case Arg::WSTRING:
            return FMT_DISPATCH(visit_wstring(arg.wstring));
        case Arg::POINTER:
            return FMT_DISPATCH(visit_pointer(arg.pointer));
        case Arg::CUSTOM:
            return FMT_DISPATCH(visit_custom(arg.custom));
        }
        return Result();
    }
};

enum Alignment
{
    ALIGN_DEFAULT, ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER, ALIGN_NUMERIC
};

// Flags.
enum
{
    SIGN_FLAG = 1, PLUS_FLAG = 2, MINUS_FLAG = 4, HASH_FLAG = 8,
    CHAR_FLAG = 0x10  // Argument has char type - used in error reporting.
};

// An empty format specifier.
struct EmptySpec {};

// A type specifier.
template <char TYPE>
struct TypeSpec : EmptySpec
{
    Alignment align() const
    {
        return ALIGN_DEFAULT;
    }
    unsigned width() const
    {
        return 0;
    }
    int precision() const
    {
        return -1;
    }
    bool flag(unsigned) const
    {
        return false;
    }
    char type() const
    {
        return TYPE;
    }
    char fill() const
    {
        return ' ';
    }
};

// A width specifier.
struct WidthSpec
{
    unsigned width_;
    // Fill is always wchar_t and cast to char if necessary to avoid having
    // two specialization of WidthSpec and its subclasses.
    wchar_t fill_;

    WidthSpec(unsigned width, wchar_t fill) : width_(width), fill_(fill) {}

    unsigned width() const
    {
        return width_;
    }
    wchar_t fill() const
    {
        return fill_;
    }
};

// An alignment specifier.
struct AlignSpec : WidthSpec
{
    Alignment align_;

    AlignSpec(unsigned width, wchar_t fill, Alignment align = ALIGN_DEFAULT)
        : WidthSpec(width, fill), align_(align) {}

    Alignment align() const
    {
        return align_;
    }

    int precision() const
    {
        return -1;
    }
};

// An alignment and type specifier.
template <char TYPE>
struct AlignTypeSpec : AlignSpec
{
    AlignTypeSpec(unsigned width, wchar_t fill) : AlignSpec(width, fill) {}

    bool flag(unsigned) const
    {
        return false;
    }
    char type() const
    {
        return TYPE;
    }
};

// A full format specifier.
struct FormatSpec : AlignSpec
{
    unsigned flags_;
    int precision_;
    char type_;

    FormatSpec(
        unsigned width = 0, char type = 0, wchar_t fill = ' ')
        : AlignSpec(width, fill), flags_(0), precision_(-1), type_(type) {}

    bool flag(unsigned f) const
    {
        return (flags_ & f) != 0;
    }
    int precision() const
    {
        return precision_;
    }
    char type() const
    {
        return type_;
    }
};

// An integer format specifier.
template <typename T, typename SpecT = TypeSpec<0>, typename Char = char>
class IntFormatSpec : public SpecT
{
private:
    T value_;

public:
    IntFormatSpec(T val, const SpecT &spec = SpecT())
        : SpecT(spec), value_(val) {}

    T value() const
    {
        return value_;
    }
};

// A string format specifier.
template <typename Char>
class StrFormatSpec : public AlignSpec
{
private:
    const Char *str_;

public:
    template <typename FillChar>
    StrFormatSpec(const Char *str, unsigned width, FillChar fill)
        : AlignSpec(width, fill), str_(str)
    {
        internal::CharTraits<Char>::convert(FillChar());
    }

    const Char *str() const
    {
        return str_;
    }
};

/**
  Returns an integer format specifier to format the value in base 2.
 */
IntFormatSpec<int, TypeSpec<'b'> > bin(int value);

/**
  Returns an integer format specifier to format the value in base 8.
 */
IntFormatSpec<int, TypeSpec<'o'> > oct(int value);

/**
  Returns an integer format specifier to format the value in base 16 using
  lower-case letters for the digits above 9.
 */
IntFormatSpec<int, TypeSpec<'x'> > hex(int value);

/**
  Returns an integer formatter format specifier to format in base 16 using
  upper-case letters for the digits above 9.
 */
IntFormatSpec<int, TypeSpec<'X'> > hexu(int value);

/**
  \rst
  Returns an integer format specifier to pad the formatted argument with the
  fill character to the specified width using the default (right) numeric
  alignment.

  **Example**::

    MemoryWriter out;
    out << pad(hex(0xcafe), 8, '0');
    // out.str() == "0000cafe"

  \endrst
 */
template <char TYPE_CODE, typename Char>
IntFormatSpec<int, AlignTypeSpec<TYPE_CODE>, Char> pad(
    int value, unsigned width, Char fill = ' ');

#define FMT_DEFINE_INT_FORMATTERS(TYPE) \
inline IntFormatSpec<TYPE, TypeSpec<'b'> > bin(TYPE value) { \
  return IntFormatSpec<TYPE, TypeSpec<'b'> >(value, TypeSpec<'b'>()); \
} \
 \
inline IntFormatSpec<TYPE, TypeSpec<'o'> > oct(TYPE value) { \
  return IntFormatSpec<TYPE, TypeSpec<'o'> >(value, TypeSpec<'o'>()); \
} \
 \
inline IntFormatSpec<TYPE, TypeSpec<'x'> > hex(TYPE value) { \
  return IntFormatSpec<TYPE, TypeSpec<'x'> >(value, TypeSpec<'x'>()); \
} \
 \
inline IntFormatSpec<TYPE, TypeSpec<'X'> > hexu(TYPE value) { \
  return IntFormatSpec<TYPE, TypeSpec<'X'> >(value, TypeSpec<'X'>()); \
} \
 \
template <char TYPE_CODE> \
inline IntFormatSpec<TYPE, AlignTypeSpec<TYPE_CODE> > pad( \
    IntFormatSpec<TYPE, TypeSpec<TYPE_CODE> > f, unsigned width) { \
  return IntFormatSpec<TYPE, AlignTypeSpec<TYPE_CODE> >( \
      f.value(), AlignTypeSpec<TYPE_CODE>(width, ' ')); \
} \
 \
/* For compatibility with older compilers we provide two overloads for pad, */ \
/* one that takes a fill character and one that doesn't. In the future this */ \
/* can be replaced with one overload making the template argument Char      */ \
/* default to char (C++11). */ \
template <char TYPE_CODE, typename Char> \
inline IntFormatSpec<TYPE, AlignTypeSpec<TYPE_CODE>, Char> pad( \
    IntFormatSpec<TYPE, TypeSpec<TYPE_CODE>, Char> f, \
    unsigned width, Char fill) { \
  return IntFormatSpec<TYPE, AlignTypeSpec<TYPE_CODE>, Char>( \
      f.value(), AlignTypeSpec<TYPE_CODE>(width, fill)); \
} \
 \
inline IntFormatSpec<TYPE, AlignTypeSpec<0> > pad( \
    TYPE value, unsigned width) { \
  return IntFormatSpec<TYPE, AlignTypeSpec<0> >( \
      value, AlignTypeSpec<0>(width, ' ')); \
} \
 \
template <typename Char> \
inline IntFormatSpec<TYPE, AlignTypeSpec<0>, Char> pad( \
   TYPE value, unsigned width, Char fill) { \
 return IntFormatSpec<TYPE, AlignTypeSpec<0>, Char>( \
     value, AlignTypeSpec<0>(width, fill)); \
}

FMT_DEFINE_INT_FORMATTERS(int)
FMT_DEFINE_INT_FORMATTERS(long)
FMT_DEFINE_INT_FORMATTERS(unsigned)
FMT_DEFINE_INT_FORMATTERS(unsigned long)
FMT_DEFINE_INT_FORMATTERS(LongLong)
FMT_DEFINE_INT_FORMATTERS(ULongLong)

/**
  \rst
  Returns a string formatter that pads the formatted argument with the fill
  character to the specified width using the default (left) string alignment.

  **Example**::

    std::string s = str(MemoryWriter() << pad("abc", 8));
    // s == "abc     "

  \endrst
 */
template <typename Char>
inline StrFormatSpec<Char> pad(
    const Char *str, unsigned width, Char fill = ' ')
{
    return StrFormatSpec<Char>(str, width, fill);
}

inline StrFormatSpec<wchar_t> pad(
    const wchar_t *str, unsigned width, char fill = ' ')
{
    return StrFormatSpec<wchar_t>(str, width, fill);
}

namespace internal
{

template <typename Char>
class ArgMap
{
private:
    typedef std::vector<
    std::pair<fmt::BasicStringRef<Char>, internal::Arg> > MapType;
    typedef typename MapType::value_type Pair;

    MapType map_;

public:
    FMT_API void init(const ArgList &args);

    const internal::Arg* find(const fmt::BasicStringRef<Char> &name) const
    {
        // The list is unsorted, so just return the first matching name.
        for (typename MapType::const_iterator it = map_.begin(), end = map_.end();
                it != end; ++it)
        {
            if (it->first == name)
                return &it->second;
        }
        return 0;
    }
};

template <typename Impl, typename Char>
class ArgFormatterBase : public ArgVisitor<Impl, void>
{
private:
    BasicWriter<Char> &writer_;
    FormatSpec &spec_;

    FMT_DISALLOW_COPY_AND_ASSIGN(ArgFormatterBase);

    void write_pointer(const void *p)
    {
        spec_.flags_ = HASH_FLAG;
        spec_.type_ = 'x';
        writer_.write_int(reinterpret_cast<uintptr_t>(p), spec_);
    }

protected:
    BasicWriter<Char> &writer()
    {
        return writer_;
    }
    FormatSpec &spec()
    {
        return spec_;
    }

    void write(bool value)
    {
        const char *str_value = value ? "true" : "false";
        Arg::StringValue<char> str = { str_value, std::strlen(str_value) };
        writer_.write_str(str, spec_);
    }

    void write(const char *value)
    {
        Arg::StringValue<char> str = {value, value != 0 ? std::strlen(value) : 0};
        writer_.write_str(str, spec_);
    }

public:
    ArgFormatterBase(BasicWriter<Char> &w, FormatSpec &s)
        : writer_(w), spec_(s) {}

    template <typename T>
    void visit_any_int(T value)
    {
        writer_.write_int(value, spec_);
    }

    template <typename T>
    void visit_any_double(T value)
    {
        writer_.write_double(value, spec_);
    }

    void visit_bool(bool value)
    {
        if (spec_.type_)
            return visit_any_int(value);
        write(value);
    }

    void visit_char(int value)
    {
        if (spec_.type_ && spec_.type_ != 'c')
        {
            spec_.flags_ |= CHAR_FLAG;
            writer_.write_int(value, spec_);
            return;
        }
        if (spec_.align_ == ALIGN_NUMERIC || spec_.flags_ != 0)
            FMT_THROW(FormatError("invalid format specifier for char"));
        typedef typename BasicWriter<Char>::CharPtr CharPtr;
        Char fill = internal::CharTraits<Char>::cast(spec_.fill());
        CharPtr out = CharPtr();
        const unsigned CHAR_SIZE = 1;
        if (spec_.width_ > CHAR_SIZE)
        {
            out = writer_.grow_buffer(spec_.width_);
            if (spec_.align_ == ALIGN_RIGHT)
            {
                std::uninitialized_fill_n(out, spec_.width_ - CHAR_SIZE, fill);
                out += spec_.width_ - CHAR_SIZE;
            }
            else if (spec_.align_ == ALIGN_CENTER)
            {
                out = writer_.fill_padding(out, spec_.width_,
                                           internal::const_check(CHAR_SIZE), fill);
            }
            else
            {
                std::uninitialized_fill_n(out + CHAR_SIZE,
                                          spec_.width_ - CHAR_SIZE, fill);
            }
        }
        else
        {
            out = writer_.grow_buffer(CHAR_SIZE);
        }
        *out = internal::CharTraits<Char>::cast(value);
    }

    void visit_cstring(const char *value)
    {
        if (spec_.type_ == 'p')
            return write_pointer(value);
        write(value);
    }

    void visit_string(Arg::StringValue<char> value)
    {
        writer_.write_str(value, spec_);
    }

    using ArgVisitor<Impl, void>::visit_wstring;

    void visit_wstring(Arg::StringValue<Char> value)
    {
        writer_.write_str(value, spec_);
    }

    void visit_pointer(const void *value)
    {
        if (spec_.type_ && spec_.type_ != 'p')
            report_unknown_type(spec_.type_, "pointer");
        write_pointer(value);
    }
};

class FormatterBase
{
private:
    ArgList args_;
    int next_arg_index_;

    // Returns the argument with specified index.
    FMT_API Arg do_get_arg(unsigned arg_index, const char *&error);

protected:
    const ArgList &args() const
    {
        return args_;
    }

    explicit FormatterBase(const ArgList &args)
    {
        args_ = args;
        next_arg_index_ = 0;
    }

    // Returns the next argument.
    Arg next_arg(const char *&error)
    {
        if (next_arg_index_ >= 0)
            return do_get_arg(internal::to_unsigned(next_arg_index_++), error);
        error = "cannot switch from manual to automatic argument indexing";
        return Arg();
    }

    // Checks if manual indexing is used and returns the argument with
    // specified index.
    Arg get_arg(unsigned arg_index, const char *&error)
    {
        return check_no_auto_index(error) ? do_get_arg(arg_index, error) : Arg();
    }

    bool check_no_auto_index(const char *&error)
    {
        if (next_arg_index_ > 0)
        {
            error = "cannot switch from automatic to manual argument indexing";
            return false;
        }
        next_arg_index_ = -1;
        return true;
    }

    template <typename Char>
    void write(BasicWriter<Char> &w, const Char *start, const Char *end)
    {
        if (start != end)
            w << BasicStringRef<Char>(start, internal::to_unsigned(end - start));
    }
};

// A printf formatter.
template <typename Char>
class PrintfFormatter : private FormatterBase
{
private:
    void parse_flags(FormatSpec &spec, const Char *&s);

    // Returns the argument with specified index or, if arg_index is equal
    // to the maximum unsigned value, the next argument.
    Arg get_arg(const Char *s,
                unsigned arg_index = (std::numeric_limits<unsigned>::max)());

    // Parses argument index, flags and width and returns the argument index.
    unsigned parse_header(const Char *&s, FormatSpec &spec);

public:
    explicit PrintfFormatter(const ArgList &args) : FormatterBase(args) {}
    FMT_API void format(BasicWriter<Char> &writer,
                        BasicCStringRef<Char> format_str);
};
}  // namespace internal

/**
  \rst
  An argument formatter based on the `curiously recurring template pattern
  <http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern>`_.

  To use `~fmt::BasicArgFormatter` define a subclass that implements some or
  all of the visit methods with the same signatures as the methods in
  `~fmt::ArgVisitor`, for example, `~fmt::ArgVisitor::visit_int()`.
  Pass the subclass as the *Impl* template parameter. When a formatting
  function processes an argument, it will dispatch to a visit method
  specific to the argument type. For example, if the argument type is
  ``double`` then the `~fmt::ArgVisitor::visit_double()` method of a subclass
  will be called. If the subclass doesn't contain a method with this signature,
  then a corresponding method of `~fmt::BasicArgFormatter` or its superclass
  will be called.
  \endrst
 */
template <typename Impl, typename Char>
class BasicArgFormatter : public internal::ArgFormatterBase<Impl, Char>
{
private:
    BasicFormatter<Char, Impl> &formatter_;
    const Char *format_;

public:
    /**
      \rst
      Constructs an argument formatter object.
      *formatter* is a reference to the main formatter object, *spec* contains
      format specifier information for standard argument types, and *fmt* points
      to the part of the format string being parsed for custom argument types.
      \endrst
     */
    BasicArgFormatter(BasicFormatter<Char, Impl> &formatter,
                      FormatSpec &spec, const Char *fmt)
        : internal::ArgFormatterBase<Impl, Char>(formatter.writer(), spec),
          formatter_(formatter), format_(fmt) {}

    /** Formats argument of a custom (user-defined) type. */
    void visit_custom(internal::Arg::CustomValue c)
    {
        c.format(&formatter_, c.value, &format_);
    }
};

/** The default argument formatter. */
template <typename Char>
class ArgFormatter : public BasicArgFormatter<ArgFormatter<Char>, Char>
{
public:
    /** Constructs an argument formatter object. */
    ArgFormatter(BasicFormatter<Char> &formatter,
                 FormatSpec &spec, const Char *fmt)
        : BasicArgFormatter<ArgFormatter<Char>, Char>(formatter, spec, fmt) {}
};

/** This template formats data and writes the output to a writer. */
template <typename CharType, typename ArgFormatter>
class BasicFormatter : private internal::FormatterBase
{
public:
    /** The character type for the output. */
    typedef CharType Char;

private:
    BasicWriter<Char> &writer_;
    internal::ArgMap<Char> map_;

    FMT_DISALLOW_COPY_AND_ASSIGN(BasicFormatter);

    using internal::FormatterBase::get_arg;

    // Checks if manual indexing is used and returns the argument with
    // specified name.
    internal::Arg get_arg(BasicStringRef<Char> arg_name, const char *&error);

    // Parses argument index and returns corresponding argument.
    internal::Arg parse_arg_index(const Char *&s);

    // Parses argument name and returns corresponding argument.
    internal::Arg parse_arg_name(const Char *&s);

public:
    /**
     \rst
     Constructs a ``BasicFormatter`` object. References to the arguments and
     the writer are stored in the formatter object so make sure they have
     appropriate lifetimes.
     \endrst
     */
    BasicFormatter(const ArgList &args, BasicWriter<Char> &w)
        : internal::FormatterBase(args), writer_(w) {}

    /** Returns a reference to the writer associated with this formatter. */
    BasicWriter<Char> &writer()
    {
        return writer_;
    }

    /** Formats stored arguments and writes the output to the writer. */
    void format(BasicCStringRef<Char> format_str);

    // Formats a single argument and advances format_str, a format string pointer.
    const Char *format(const Char *&format_str, const internal::Arg &arg);
};

// Generates a comma-separated list with results of applying f to
// numbers 0..n-1.
# define FMT_GEN(n, f) FMT_GEN##n(f)
# define FMT_GEN1(f)  f(0)
# define FMT_GEN2(f)  FMT_GEN1(f),  f(1)
# define FMT_GEN3(f)  FMT_GEN2(f),  f(2)
# define FMT_GEN4(f)  FMT_GEN3(f),  f(3)
# define FMT_GEN5(f)  FMT_GEN4(f),  f(4)
# define FMT_GEN6(f)  FMT_GEN5(f),  f(5)
# define FMT_GEN7(f)  FMT_GEN6(f),  f(6)
# define FMT_GEN8(f)  FMT_GEN7(f),  f(7)
# define FMT_GEN9(f)  FMT_GEN8(f),  f(8)
# define FMT_GEN10(f) FMT_GEN9(f),  f(9)
# define FMT_GEN11(f) FMT_GEN10(f), f(10)
# define FMT_GEN12(f) FMT_GEN11(f), f(11)
# define FMT_GEN13(f) FMT_GEN12(f), f(12)
# define FMT_GEN14(f) FMT_GEN13(f), f(13)
# define FMT_GEN15(f) FMT_GEN14(f), f(14)

namespace internal
{
inline uint64_t make_type()
{
    return 0;
}

template <typename T>
inline uint64_t make_type(const T &arg)
{
    return MakeValue< BasicFormatter<char> >::type(arg);
}

template <unsigned N, bool/*IsPacked*/= (N < ArgList::MAX_PACKED_ARGS)>
          struct ArgArray;

template <unsigned N>
struct ArgArray<N, true/*IsPacked*/>
{
    typedef Value Type[N > 0 ? N : 1];

template <typename Formatter, typename T>
static Value make(const T &value)
{
#ifdef __clang__
    Value result = MakeValue<Formatter>(value);
    // Workaround a bug in Apple LLVM version 4.2 (clang-425.0.28) of clang:
    // https://github.com/fmtlib/fmt/issues/276
    (void)result.custom.format;
    return result;
#else
    return MakeValue<Formatter>(value);
#endif
}
         };

template <unsigned N>
struct ArgArray<N, false/*IsPacked*/>
{
    typedef Arg Type[N + 1]; // +1 for the list end Arg::NONE

    template <typename Formatter, typename T>
    static Arg make(const T &value)
    {
        return MakeArg<Formatter>(value);
    }
};

#if FMT_USE_VARIADIC_TEMPLATES
template <typename Arg, typename... Args>
inline uint64_t make_type(const Arg &first, const Args & ... tail)
{
    return make_type(first) | (make_type(tail...) << 4);
}

#else

struct ArgType
{
    uint64_t type;

    ArgType() : type(0) {}

    template <typename T>
    ArgType(const T &arg) : type(make_type(arg)) {}
};

# define FMT_ARG_TYPE_DEFAULT(n) ArgType t##n = ArgType()

inline uint64_t make_type(FMT_GEN15(FMT_ARG_TYPE_DEFAULT))
{
    return t0.type | (t1.type << 4) | (t2.type << 8) | (t3.type << 12) |
           (t4.type << 16) | (t5.type << 20) | (t6.type << 24) | (t7.type << 28) |
           (t8.type << 32) | (t9.type << 36) | (t10.type << 40) | (t11.type << 44) |
           (t12.type << 48) | (t13.type << 52) | (t14.type << 56);
}
#endif
}  // namespace internal

# define FMT_MAKE_TEMPLATE_ARG(n) typename T##n
# define FMT_MAKE_ARG_TYPE(n) T##n
# define FMT_MAKE_ARG(n) const T##n &v##n
# define FMT_ASSIGN_char(n) \
  arr[n] = fmt::internal::MakeValue< fmt::BasicFormatter<char> >(v##n)
# define FMT_ASSIGN_wchar_t(n) \
  arr[n] = fmt::internal::MakeValue< fmt::BasicFormatter<wchar_t> >(v##n)

#if FMT_USE_VARIADIC_TEMPLATES
// Defines a variadic function returning void.
# define FMT_VARIADIC_VOID(func, arg_type) \
  template <typename... Args> \
  void func(arg_type arg0, const Args & ... args) { \
    typedef fmt::internal::ArgArray<sizeof...(Args)> ArgArray; \
    typename ArgArray::Type array{ \
      ArgArray::template make<fmt::BasicFormatter<Char> >(args)...}; \
    func(arg0, fmt::ArgList(fmt::internal::make_type(args...), array)); \
  }

// Defines a variadic constructor.
# define FMT_VARIADIC_CTOR(ctor, func, arg0_type, arg1_type) \
  template <typename... Args> \
  ctor(arg0_type arg0, arg1_type arg1, const Args & ... args) { \
    typedef fmt::internal::ArgArray<sizeof...(Args)> ArgArray; \
    typename ArgArray::Type array{ \
      ArgArray::template make<fmt::BasicFormatter<Char> >(args)...}; \
    func(arg0, arg1, fmt::ArgList(fmt::internal::make_type(args...), array)); \
  }

#else

# define FMT_MAKE_REF(n) \
  fmt::internal::MakeValue< fmt::BasicFormatter<Char> >(v##n)
# define FMT_MAKE_REF2(n) v##n

// Defines a wrapper for a function taking one argument of type arg_type
// and n additional arguments of arbitrary types.
# define FMT_WRAP1(func, arg_type, n) \
  template <FMT_GEN(n, FMT_MAKE_TEMPLATE_ARG)> \
  inline void func(arg_type arg1, FMT_GEN(n, FMT_MAKE_ARG)) { \
    const fmt::internal::ArgArray<n>::Type array = {FMT_GEN(n, FMT_MAKE_REF)}; \
    func(arg1, fmt::ArgList( \
      fmt::internal::make_type(FMT_GEN(n, FMT_MAKE_REF2)), array)); \
  }

// Emulates a variadic function returning void on a pre-C++11 compiler.
# define FMT_VARIADIC_VOID(func, arg_type) \
  inline void func(arg_type arg) { func(arg, fmt::ArgList()); } \
  FMT_WRAP1(func, arg_type, 1) FMT_WRAP1(func, arg_type, 2) \
  FMT_WRAP1(func, arg_type, 3) FMT_WRAP1(func, arg_type, 4) \
  FMT_WRAP1(func, arg_type, 5) FMT_WRAP1(func, arg_type, 6) \
  FMT_WRAP1(func, arg_type, 7) FMT_WRAP1(func, arg_type, 8) \
  FMT_WRAP1(func, arg_type, 9) FMT_WRAP1(func, arg_type, 10)

# define FMT_CTOR(ctor, func, arg0_type, arg1_type, n) \
  template <FMT_GEN(n, FMT_MAKE_TEMPLATE_ARG)> \
  ctor(arg0_type arg0, arg1_type arg1, FMT_GEN(n, FMT_MAKE_ARG)) { \
    const fmt::internal::ArgArray<n>::Type array = {FMT_GEN(n, FMT_MAKE_REF)}; \
    func(arg0, arg1, fmt::ArgList( \
      fmt::internal::make_type(FMT_GEN(n, FMT_MAKE_REF2)), array)); \
  }

// Emulates a variadic constructor on a pre-C++11 compiler.
# define FMT_VARIADIC_CTOR(ctor, func, arg0_type, arg1_type) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 1) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 2) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 3) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 4) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 5) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 6) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 7) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 8) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 9) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 10)
#endif

// Generates a comma-separated list with results of applying f to pairs
// (argument, index).
#define FMT_FOR_EACH1(f, x0) f(x0, 0)
#define FMT_FOR_EACH2(f, x0, x1) \
  FMT_FOR_EACH1(f, x0), f(x1, 1)
#define FMT_FOR_EACH3(f, x0, x1, x2) \
  FMT_FOR_EACH2(f, x0 ,x1), f(x2, 2)
#define FMT_FOR_EACH4(f, x0, x1, x2, x3) \
  FMT_FOR_EACH3(f, x0, x1, x2), f(x3, 3)
#define FMT_FOR_EACH5(f, x0, x1, x2, x3, x4) \
  FMT_FOR_EACH4(f, x0, x1, x2, x3), f(x4, 4)
#define FMT_FOR_EACH6(f, x0, x1, x2, x3, x4, x5) \
  FMT_FOR_EACH5(f, x0, x1, x2, x3, x4), f(x5, 5)
#define FMT_FOR_EACH7(f, x0, x1, x2, x3, x4, x5, x6) \
  FMT_FOR_EACH6(f, x0, x1, x2, x3, x4, x5), f(x6, 6)
#define FMT_FOR_EACH8(f, x0, x1, x2, x3, x4, x5, x6, x7) \
  FMT_FOR_EACH7(f, x0, x1, x2, x3, x4, x5, x6), f(x7, 7)
#define FMT_FOR_EACH9(f, x0, x1, x2, x3, x4, x5, x6, x7, x8) \
  FMT_FOR_EACH8(f, x0, x1, x2, x3, x4, x5, x6, x7), f(x8, 8)
#define FMT_FOR_EACH10(f, x0, x1, x2, x3, x4, x5, x6, x7, x8, x9) \
  FMT_FOR_EACH9(f, x0, x1, x2, x3, x4, x5, x6, x7, x8), f(x9, 9)

/**
 An error returned by an operating system or a language runtime,
 for example a file opening error.
*/
class SystemError : public internal::RuntimeError
{
private:
    void init(int err_code, CStringRef format_str, ArgList args);

protected:
    int error_code_;

    typedef char Char;  // For FMT_VARIADIC_CTOR.

    SystemError() {}

public:
    /**
     \rst
     Constructs a :class:`fmt::SystemError` object with the description
     of the form

     .. parsed-literal::
       *<message>*: *<system-message>*

     where *<message>* is the formatted message and *<system-message>* is
     the system message corresponding to the error code.
     *error_code* is a system error code as given by ``errno``.
     If *error_code* is not a valid error code such as -1, the system message
     may look like "Unknown error -1" and is platform-dependent.

     **Example**::

       // This throws a SystemError with the description
       //   cannot open file 'madeup': No such file or directory
       // or similar (system message may vary).
       const char *filename = "madeup";
       std::FILE *file = std::fopen(filename, "r");
       if (!file)
         throw fmt::SystemError(errno, "cannot open file '{}'", filename);
     \endrst
    */
    SystemError(int error_code, CStringRef message)
    {
        init(error_code, message, ArgList());
    }
    FMT_VARIADIC_CTOR(SystemError, init, int, CStringRef)

    ~SystemError() throw();

    int error_code() const
    {
        return error_code_;
    }
};

/**
  \rst
  This template provides operations for formatting and writing data into
  a character stream. The output is stored in a buffer provided by a subclass
  such as :class:`fmt::BasicMemoryWriter`.

  You can use one of the following typedefs for common character types:

  +---------+----------------------+
  | Type    | Definition           |
  +=========+======================+
  | Writer  | BasicWriter<char>    |
  +---------+----------------------+
  | WWriter | BasicWriter<wchar_t> |
  +---------+----------------------+

  \endrst
 */
template <typename Char>
class BasicWriter
{
private:
    // Output buffer.
    Buffer<Char> &buffer_;

    FMT_DISALLOW_COPY_AND_ASSIGN(BasicWriter);

    typedef typename internal::CharTraits<Char>::CharPtr CharPtr;

#if FMT_SECURE_SCL
    // Returns pointer value.
    static Char *get(CharPtr p)
    {
        return p.base();
    }
#else
    static Char *get(Char *p)
    {
        return p;
    }
#endif

    // Fills the padding around the content and returns the pointer to the
    // content area.
    static CharPtr fill_padding(CharPtr buffer,
                                unsigned total_size, std::size_t content_size, wchar_t fill);

    // Grows the buffer by n characters and returns a pointer to the newly
    // allocated area.
    CharPtr grow_buffer(std::size_t n)
    {
        std::size_t size = buffer_.size();
        buffer_.resize(size + n);
        return internal::make_ptr(&buffer_[size], n);
    }

    // Writes an unsigned decimal integer.
    template <typename UInt>
    Char *write_unsigned_decimal(UInt value, unsigned prefix_size = 0)
    {
        unsigned num_digits = internal::count_digits(value);
        Char *ptr = get(grow_buffer(prefix_size + num_digits));
        internal::format_decimal(ptr + prefix_size, value, num_digits);
        return ptr;
    }

    // Writes a decimal integer.
    template <typename Int>
    void write_decimal(Int value)
    {
        typedef typename internal::IntTraits<Int>::MainType MainType;
        MainType abs_value = static_cast<MainType>(value);
        if (internal::is_negative(value))
        {
            abs_value = 0 - abs_value;
            *write_unsigned_decimal(abs_value, 1) = '-';
        }
        else
        {
            write_unsigned_decimal(abs_value, 0);
        }
    }

    // Prepare a buffer for integer formatting.
    CharPtr prepare_int_buffer(unsigned num_digits,
                               const EmptySpec &, const char *prefix, unsigned prefix_size)
    {
        unsigned size = prefix_size + num_digits;
        CharPtr p = grow_buffer(size);
        std::uninitialized_copy(prefix, prefix + prefix_size, p);
        return p + size - 1;
    }

    template <typename Spec>
    CharPtr prepare_int_buffer(unsigned num_digits,
                               const Spec &spec, const char *prefix, unsigned prefix_size);

    // Formats an integer.
    template <typename T, typename Spec>
    void write_int(T value, Spec spec);

    // Formats a floating-point number (double or long double).
    template <typename T>
    void write_double(T value, const FormatSpec &spec);

    // Writes a formatted string.
    template <typename StrChar>
    CharPtr write_str(const StrChar *s, std::size_t size, const AlignSpec &spec);

    template <typename StrChar>
    void write_str(const internal::Arg::StringValue<StrChar> &str,
                   const FormatSpec &spec);

    // This following methods are private to disallow writing wide characters
    // and strings to a char stream. If you want to print a wide string as a
    // pointer as std::ostream does, cast it to const void*.
    // Do not implement!
    void operator<<(typename internal::WCharHelper<wchar_t, Char>::Unsupported);
    void operator<<(
        typename internal::WCharHelper<const wchar_t *, Char>::Unsupported);

    // Appends floating-point length specifier to the format string.
    // The second argument is only used for overload resolution.
    void append_float_length(Char *&format_ptr, long double)
    {
        *format_ptr++ = 'L';
    }

    template<typename T>
    void append_float_length(Char *&, T) {}

    template <typename Impl, typename Char_>
    friend class internal::ArgFormatterBase;

    friend class internal::PrintfArgFormatter<Char>;

protected:
    /**
      Constructs a ``BasicWriter`` object.
     */
    explicit BasicWriter(Buffer<Char> &b) : buffer_(b) {}

public:
    /**
      \rst
      Destroys a ``BasicWriter`` object.
      \endrst
     */
    virtual ~BasicWriter() {}

    /**
      Returns the total number of characters written.
     */
    std::size_t size() const
    {
        return buffer_.size();
    }

    /**
      Returns a pointer to the output buffer content. No terminating null
      character is appended.
     */
    const Char *data() const FMT_NOEXCEPT
    {
        return &buffer_[0];
    }

    /**
      Returns a pointer to the output buffer content with terminating null
      character appended.
     */
    const Char *c_str() const
    {
        std::size_t size = buffer_.size();
        buffer_.reserve(size + 1);
        buffer_[size] = '\0';
        return &buffer_[0];
    }

    /**
      \rst
      Returns the content of the output buffer as an `std::string`.
      \endrst
     */
    std::basic_string<Char> str() const
    {
        return std::basic_string<Char>(&buffer_[0], buffer_.size());
    }

    /**
      \rst
      Writes formatted data.

      *args* is an argument list representing arbitrary arguments.

      **Example**::

         MemoryWriter out;
         out.write("Current point:\n");
         out.write("({:+f}, {:+f})", -3.14, 3.14);

      This will write the following output to the ``out`` object:

      .. code-block:: none

         Current point:
         (-3.140000, +3.140000)

      The output can be accessed using :func:`data()`, :func:`c_str` or
      :func:`str` methods.

      See also :ref:`syntax`.
      \endrst
     */
    void write(BasicCStringRef<Char> format, ArgList args)
    {
        BasicFormatter<Char>(args, *this).format(format);
    }
    FMT_VARIADIC_VOID(write, BasicCStringRef<Char>)

    BasicWriter &operator<<(int value)
    {
        write_decimal(value);
        return *this;
    }
    BasicWriter &operator<<(unsigned value)
    {
        return *this << IntFormatSpec<unsigned>(value);
    }
    BasicWriter &operator<<(long value)
    {
        write_decimal(value);
        return *this;
    }
    BasicWriter &operator<<(unsigned long value)
    {
        return *this << IntFormatSpec<unsigned long>(value);
    }
    BasicWriter &operator<<(LongLong value)
    {
        write_decimal(value);
        return *this;
    }

    /**
      \rst
      Formats *value* and writes it to the stream.
      \endrst
     */
    BasicWriter &operator<<(ULongLong value)
    {
        return *this << IntFormatSpec<ULongLong>(value);
    }

    BasicWriter &operator<<(double value)
    {
        write_double(value, FormatSpec());
        return *this;
    }

    /**
      \rst
      Formats *value* using the general format for floating-point numbers
      (``'g'``) and writes it to the stream.
      \endrst
     */
    BasicWriter &operator<<(long double value)
    {
        write_double(value, FormatSpec());
        return *this;
    }

    /**
      Writes a character to the stream.
     */
    BasicWriter &operator<<(char value)
    {
        buffer_.push_back(value);
        return *this;
    }

    BasicWriter &operator<<(
        typename internal::WCharHelper<wchar_t, Char>::Supported value)
    {
        buffer_.push_back(value);
        return *this;
    }

    /**
      \rst
      Writes *value* to the stream.
      \endrst
     */
    BasicWriter &operator<<(fmt::BasicStringRef<Char> value)
    {
        const Char *str = value.data();
        buffer_.append(str, str + value.size());
        return *this;
    }

    BasicWriter &operator<<(
        typename internal::WCharHelper<StringRef, Char>::Supported value)
    {
        const char *str = value.data();
        buffer_.append(str, str + value.size());
        return *this;
    }

    template <typename T, typename Spec, typename FillChar>
    BasicWriter &operator<<(IntFormatSpec<T, Spec, FillChar> spec)
    {
        internal::CharTraits<Char>::convert(FillChar());
        write_int(spec.value(), spec);
        return *this;
    }

    template <typename StrChar>
    BasicWriter &operator<<(const StrFormatSpec<StrChar> &spec)
    {
        const StrChar *s = spec.str();
        write_str(s, std::char_traits<Char>::length(s), spec);
        return *this;
    }

    void clear() FMT_NOEXCEPT { buffer_.clear(); }

    Buffer<Char> &buffer() FMT_NOEXCEPT { return buffer_; }
};

template <typename Char>
template <typename StrChar>
typename BasicWriter<Char>::CharPtr BasicWriter<Char>::write_str(
    const StrChar *s, std::size_t size, const AlignSpec &spec)
{
    CharPtr out = CharPtr();
    if (spec.width() > size)
    {
        out = grow_buffer(spec.width());
        Char fill = internal::CharTraits<Char>::cast(spec.fill());
        if (spec.align() == ALIGN_RIGHT)
        {
            std::uninitialized_fill_n(out, spec.width() - size, fill);
            out += spec.width() - size;
        }
        else if (spec.align() == ALIGN_CENTER)
        {
            out = fill_padding(out, spec.width(), size, fill);
        }
        else
        {
            std::uninitialized_fill_n(out + size, spec.width() - size, fill);
        }
    }
    else
    {
        out = grow_buffer(size);
    }
    std::uninitialized_copy(s, s + size, out);
    return out;
}

template <typename Char>
template <typename StrChar>
void BasicWriter<Char>::write_str(
    const internal::Arg::StringValue<StrChar> &s, const FormatSpec &spec)
{
    // Check if StrChar is convertible to Char.
    internal::CharTraits<Char>::convert(StrChar());
    if (spec.type_ && spec.type_ != 's')
        internal::report_unknown_type(spec.type_, "string");
    const StrChar *str_value = s.value;
    std::size_t str_size = s.size;
    if (str_size == 0)
    {
        if (!str_value)
        {
            FMT_THROW(FormatError("string pointer is null"));
        }
    }
    std::size_t precision = static_cast<std::size_t>(spec.precision_);
    if (spec.precision_ >= 0 && precision < str_size)
        str_size = precision;
    write_str(str_value, str_size, spec);
}

template <typename Char>
typename BasicWriter<Char>::CharPtr
BasicWriter<Char>::fill_padding(
    CharPtr buffer, unsigned total_size,
    std::size_t content_size, wchar_t fill)
{
    std::size_t padding = total_size - content_size;
    std::size_t left_padding = padding / 2;
    Char fill_char = internal::CharTraits<Char>::cast(fill);
    std::uninitialized_fill_n(buffer, left_padding, fill_char);
    buffer += left_padding;
    CharPtr content = buffer;
    std::uninitialized_fill_n(buffer + content_size,
                              padding - left_padding, fill_char);
    return content;
}

template <typename Char>
template <typename Spec>
typename BasicWriter<Char>::CharPtr
BasicWriter<Char>::prepare_int_buffer(
    unsigned num_digits, const Spec &spec,
    const char *prefix, unsigned prefix_size)
{
    unsigned width = spec.width();
    Alignment align = spec.align();
    Char fill = internal::CharTraits<Char>::cast(spec.fill());
    if (spec.precision() > static_cast<int>(num_digits))
    {
        // Octal prefix '0' is counted as a digit, so ignore it if precision
        // is specified.
        if (prefix_size > 0 && prefix[prefix_size - 1] == '0')
            --prefix_size;
        unsigned number_size =
            prefix_size + internal::to_unsigned(spec.precision());
        AlignSpec subspec(number_size, '0', ALIGN_NUMERIC);
        if (number_size >= width)
            return prepare_int_buffer(num_digits, subspec, prefix, prefix_size);
        buffer_.reserve(width);
        unsigned fill_size = width - number_size;
        if (align != ALIGN_LEFT)
        {
            CharPtr p = grow_buffer(fill_size);
            std::uninitialized_fill(p, p + fill_size, fill);
        }
        CharPtr result = prepare_int_buffer(
                             num_digits, subspec, prefix, prefix_size);
        if (align == ALIGN_LEFT)
        {
            CharPtr p = grow_buffer(fill_size);
            std::uninitialized_fill(p, p + fill_size, fill);
        }
        return result;
    }
    unsigned size = prefix_size + num_digits;
    if (width <= size)
    {
        CharPtr p = grow_buffer(size);
        std::uninitialized_copy(prefix, prefix + prefix_size, p);
        return p + size - 1;
    }
    CharPtr p = grow_buffer(width);
    CharPtr end = p + width;
    if (align == ALIGN_LEFT)
    {
        std::uninitialized_copy(prefix, prefix + prefix_size, p);
        p += size;
        std::uninitialized_fill(p, end, fill);
    }
    else if (align == ALIGN_CENTER)
    {
        p = fill_padding(p, width, size, fill);
        std::uninitialized_copy(prefix, prefix + prefix_size, p);
        p += size;
    }
    else
    {
        if (align == ALIGN_NUMERIC)
        {
            if (prefix_size != 0)
            {
                p = std::uninitialized_copy(prefix, prefix + prefix_size, p);
                size -= prefix_size;
            }
        }
        else
        {
            std::uninitialized_copy(prefix, prefix + prefix_size, end - size);
        }
        std::uninitialized_fill(p, end - size, fill);
        p = end;
    }
    return p - 1;
}

template <typename Char>
template <typename T, typename Spec>
void BasicWriter<Char>::write_int(T value, Spec spec)
{
    unsigned prefix_size = 0;
    typedef typename internal::IntTraits<T>::MainType UnsignedType;
    UnsignedType abs_value = static_cast<UnsignedType>(value);
    char prefix[4] = "";
    if (internal::is_negative(value))
    {
        prefix[0] = '-';
        ++prefix_size;
        abs_value = 0 - abs_value;
    }
    else if (spec.flag(SIGN_FLAG))
    {
        prefix[0] = spec.flag(PLUS_FLAG) ? '+' : ' ';
        ++prefix_size;
    }
    switch (spec.type())
    {
    case 0:
    case 'd':
    {
        unsigned num_digits = internal::count_digits(abs_value);
        CharPtr p = prepare_int_buffer(num_digits, spec, prefix, prefix_size) + 1;
        internal::format_decimal(get(p), abs_value, 0);
        break;
    }
    case 'x':
    case 'X':
    {
        UnsignedType n = abs_value;
        if (spec.flag(HASH_FLAG))
        {
            prefix[prefix_size++] = '0';
            prefix[prefix_size++] = spec.type();
        }
        unsigned num_digits = 0;
        do
        {
            ++num_digits;
        }
        while ((n >>= 4) != 0);
        Char *p = get(prepare_int_buffer(
                          num_digits, spec, prefix, prefix_size));
        n = abs_value;
        const char *digits = spec.type() == 'x' ?
                             "0123456789abcdef" : "0123456789ABCDEF";
        do
        {
            *p-- = digits[n & 0xf];
        }
        while ((n >>= 4) != 0);
        break;
    }
    case 'b':
    case 'B':
    {
        UnsignedType n = abs_value;
        if (spec.flag(HASH_FLAG))
        {
            prefix[prefix_size++] = '0';
            prefix[prefix_size++] = spec.type();
        }
        unsigned num_digits = 0;
        do
        {
            ++num_digits;
        }
        while ((n >>= 1) != 0);
        Char *p = get(prepare_int_buffer(num_digits, spec, prefix, prefix_size));
        n = abs_value;
        do
        {
            *p-- = static_cast<Char>('0' + (n & 1));
        }
        while ((n >>= 1) != 0);
        break;
    }
    case 'o':
    {
        UnsignedType n = abs_value;
        if (spec.flag(HASH_FLAG))
            prefix[prefix_size++] = '0';
        unsigned num_digits = 0;
        do
        {
            ++num_digits;
        }
        while ((n >>= 3) != 0);
        Char *p = get(prepare_int_buffer(num_digits, spec, prefix, prefix_size));
        n = abs_value;
        do
        {
            *p-- = static_cast<Char>('0' + (n & 7));
        }
        while ((n >>= 3) != 0);
        break;
    }
    case 'n':
    {
        unsigned num_digits = internal::count_digits(abs_value);
        fmt::StringRef sep = "";
#ifndef ANDROID
        sep = internal::thousands_sep(std::localeconv());
#endif
        unsigned size = static_cast<unsigned>(
                            num_digits + sep.size() * ((num_digits - 1) / 3));
        CharPtr p = prepare_int_buffer(size, spec, prefix, prefix_size) + 1;
        internal::format_decimal(get(p), abs_value, 0, internal::ThousandsSep(sep));
        break;
    }
    default:
        internal::report_unknown_type(
            spec.type(), spec.flag(CHAR_FLAG) ? "char" : "integer");
        break;
    }
}

template <typename Char>
template <typename T>
void BasicWriter<Char>::write_double(T value, const FormatSpec &spec)
{
    // Check type.
    char type = spec.type();
    bool upper = false;
    switch (type)
    {
    case 0:
        type = 'g';
        break;
    case 'e':
    case 'f':
    case 'g':
    case 'a':
        break;
    case 'F':
#if FMT_MSC_VER
        // MSVC's printf doesn't support 'F'.
        type = 'f';
#endif
    // Fall through.
    case 'E':
    case 'G':
    case 'A':
        upper = true;
        break;
    default:
        internal::report_unknown_type(type, "double");
        break;
    }

    char sign = 0;
    // Use isnegative instead of value < 0 because the latter is always
    // false for NaN.
    if (internal::FPUtil::isnegative(static_cast<double>(value)))
    {
        sign = '-';
        value = -value;
    }
    else if (spec.flag(SIGN_FLAG))
    {
        sign = spec.flag(PLUS_FLAG) ? '+' : ' ';
    }

    if (internal::FPUtil::isnotanumber(value))
    {
        // Format NaN ourselves because sprintf's output is not consistent
        // across platforms.
        std::size_t nan_size = 4;
        const char *nan = upper ? " NAN" : " nan";
        if (!sign)
        {
            --nan_size;
            ++nan;
        }
        CharPtr out = write_str(nan, nan_size, spec);
        if (sign)
            *out = sign;
        return;
    }

    if (internal::FPUtil::isinfinity(value))
    {
        // Format infinity ourselves because sprintf's output is not consistent
        // across platforms.
        std::size_t inf_size = 4;
        const char *inf = upper ? " INF" : " inf";
        if (!sign)
        {
            --inf_size;
            ++inf;
        }
        CharPtr out = write_str(inf, inf_size, spec);
        if (sign)
            *out = sign;
        return;
    }

    std::size_t offset = buffer_.size();
    unsigned width = spec.width();
    if (sign)
    {
        buffer_.reserve(buffer_.size() + (width > 1u ? width : 1u));
        if (width > 0)
            --width;
        ++offset;
    }

    // Build format string.
    enum { MAX_FORMAT_SIZE = 10}; // longest format: %#-*.*Lg
    Char format[MAX_FORMAT_SIZE];
    Char *format_ptr = format;
    *format_ptr++ = '%';
    unsigned width_for_sprintf = width;
    if (spec.flag(HASH_FLAG))
        *format_ptr++ = '#';
    if (spec.align() == ALIGN_CENTER)
    {
        width_for_sprintf = 0;
    }
    else
    {
        if (spec.align() == ALIGN_LEFT)
            *format_ptr++ = '-';
        if (width != 0)
            *format_ptr++ = '*';
    }
    if (spec.precision() >= 0)
    {
        *format_ptr++ = '.';
        *format_ptr++ = '*';
    }

    append_float_length(format_ptr, value);
    *format_ptr++ = type;
    *format_ptr = '\0';

    // Format using snprintf.
    Char fill = internal::CharTraits<Char>::cast(spec.fill());
    unsigned n = 0;
    Char *start = 0;
    for (;;)
    {
        std::size_t buffer_size = buffer_.capacity() - offset;
#if FMT_MSC_VER
        // MSVC's vsnprintf_s doesn't work with zero size, so reserve
        // space for at least one extra character to make the size non-zero.
        // Note that the buffer's capacity will increase by more than 1.
        if (buffer_size == 0)
        {
            buffer_.reserve(offset + 1);
            buffer_size = buffer_.capacity() - offset;
        }
#endif
        start = &buffer_[offset];
        int result = internal::CharTraits<Char>::format_float(
                         start, buffer_size, format, width_for_sprintf, spec.precision(), value);
        if (result >= 0)
        {
            n = internal::to_unsigned(result);
            if (offset + n < buffer_.capacity())
                break;  // The buffer is large enough - continue with formatting.
            buffer_.reserve(offset + n + 1);
        }
        else
        {
            // If result is negative we ask to increase the capacity by at least 1,
            // but as std::vector, the buffer grows exponentially.
            buffer_.reserve(buffer_.capacity() + 1);
        }
    }
    if (sign)
    {
        if ((spec.align() != ALIGN_RIGHT && spec.align() != ALIGN_DEFAULT) ||
                *start != ' ')
        {
            *(start - 1) = sign;
            sign = 0;
        }
        else
        {
            *(start - 1) = fill;
        }
        ++n;
    }
    if (spec.align() == ALIGN_CENTER && spec.width() > n)
    {
        width = spec.width();
        CharPtr p = grow_buffer(width);
        std::memmove(get(p) + (width - n) / 2, get(p), n * sizeof(Char));
        fill_padding(p, spec.width(), n, fill);
        return;
    }
    if (spec.fill() != ' ' || sign)
    {
        while (*start == ' ')
            *start++ = fill;
        if (sign)
            *(start - 1) = sign;
    }
    grow_buffer(n);
}

/**
  \rst
  This class template provides operations for formatting and writing data
  into a character stream. The output is stored in a memory buffer that grows
  dynamically.

  You can use one of the following typedefs for common character types
  and the standard allocator:

  +---------------+-----------------------------------------------------+
  | Type          | Definition                                          |
  +===============+=====================================================+
  | MemoryWriter  | BasicMemoryWriter<char, std::allocator<char>>       |
  +---------------+-----------------------------------------------------+
  | WMemoryWriter | BasicMemoryWriter<wchar_t, std::allocator<wchar_t>> |
  +---------------+-----------------------------------------------------+

  **Example**::

     MemoryWriter out;
     out << "The answer is " << 42 << "\n";
     out.write("({:+f}, {:+f})", -3.14, 3.14);

  This will write the following output to the ``out`` object:

  .. code-block:: none

     The answer is 42
     (-3.140000, +3.140000)

  The output can be converted to an ``std::string`` with ``out.str()`` or
  accessed as a C string with ``out.c_str()``.
  \endrst
 */
template <typename Char, typename Allocator = std::allocator<Char> >
class BasicMemoryWriter : public BasicWriter<Char>
{
private:
    internal::MemoryBuffer<Char, internal::INLINE_BUFFER_SIZE, Allocator> buffer_;

public:
    explicit BasicMemoryWriter(const Allocator& alloc = Allocator())
        : BasicWriter<Char>(buffer_), buffer_(alloc) {}

#if FMT_USE_RVALUE_REFERENCES
    /**
      \rst
      Constructs a :class:`fmt::BasicMemoryWriter` object moving the content
      of the other object to it.
      \endrst
     */
    BasicMemoryWriter(BasicMemoryWriter &&other)
        : BasicWriter<Char>(buffer_), buffer_(std::move(other.buffer_))
    {
    }

    /**
      \rst
      Moves the content of the other ``BasicMemoryWriter`` object to this one.
      \endrst
     */
    BasicMemoryWriter &operator=(BasicMemoryWriter &&other)
    {
        buffer_ = std::move(other.buffer_);
        return *this;
    }
#endif
};

typedef BasicMemoryWriter<char> MemoryWriter;
typedef BasicMemoryWriter<wchar_t> WMemoryWriter;

/**
  \rst
  This class template provides operations for formatting and writing data
  into a fixed-size array. For writing into a dynamically growing buffer
  use :class:`fmt::BasicMemoryWriter`.

  Any write method will throw ``std::runtime_error`` if the output doesn't fit
  into the array.

  You can use one of the following typedefs for common character types:

  +--------------+---------------------------+
  | Type         | Definition                |
  +==============+===========================+
  | ArrayWriter  | BasicArrayWriter<char>    |
  +--------------+---------------------------+
  | WArrayWriter | BasicArrayWriter<wchar_t> |
  +--------------+---------------------------+
  \endrst
 */
template <typename Char>
class BasicArrayWriter : public BasicWriter<Char>
{
private:
    internal::FixedBuffer<Char> buffer_;

public:
    /**
     \rst
     Constructs a :class:`fmt::BasicArrayWriter` object for *array* of the
     given size.
     \endrst
     */
    BasicArrayWriter(Char *array, std::size_t size)
        : BasicWriter<Char>(buffer_), buffer_(array, size) {}

    /**
     \rst
     Constructs a :class:`fmt::BasicArrayWriter` object for *array* of the
     size known at compile time.
     \endrst
     */
    template <std::size_t SIZE>
    explicit BasicArrayWriter(Char (&array)[SIZE])
        : BasicWriter<Char>(buffer_), buffer_(array, SIZE) {}
};

typedef BasicArrayWriter<char> ArrayWriter;
typedef BasicArrayWriter<wchar_t> WArrayWriter;

// Reports a system error without throwing an exception.
// Can be used to report errors from destructors.
FMT_API void report_system_error(int error_code,
                                 StringRef message) FMT_NOEXCEPT;

#if FMT_USE_WINDOWS_H

/** A Windows error. */
class WindowsError : public SystemError
{
private:
    FMT_API void init(int error_code, CStringRef format_str, ArgList args);

public:
    /**
     \rst
     Constructs a :class:`fmt::WindowsError` object with the description
     of the form

     .. parsed-literal::
       *<message>*: *<system-message>*

     where *<message>* is the formatted message and *<system-message>* is the
     system message corresponding to the error code.
     *error_code* is a Windows error code as given by ``GetLastError``.
     If *error_code* is not a valid error code such as -1, the system message
     will look like "error -1".

     **Example**::

       // This throws a WindowsError with the description
       //   cannot open file 'madeup': The system cannot find the file specified.
       // or similar (system message may vary).
       const char *filename = "madeup";
       LPOFSTRUCT of = LPOFSTRUCT();
       HFILE file = OpenFile(filename, &of, OF_READ);
       if (file == HFILE_ERROR) {
         throw fmt::WindowsError(GetLastError(),
                                 "cannot open file '{}'", filename);
       }
     \endrst
    */
    WindowsError(int error_code, CStringRef message)
    {
        init(error_code, message, ArgList());
    }
    FMT_VARIADIC_CTOR(WindowsError, init, int, CStringRef)
};

// Reports a Windows error without throwing an exception.
// Can be used to report errors from destructors.
FMT_API void report_windows_error(int error_code,
                                  StringRef message) FMT_NOEXCEPT;

#endif

enum Color { BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE };

/**
  Formats a string and prints it to stdout using ANSI escape sequences
  to specify color (experimental).
  Example:
    print_colored(fmt::RED, "Elapsed time: {0:.2f} seconds", 1.23);
 */
FMT_API void print_colored(Color c, CStringRef format, ArgList args);

/**
  \rst
  Formats arguments and returns the result as a string.

  **Example**::

    std::string message = format("The answer is {}", 42);
  \endrst
*/
inline std::string format(CStringRef format_str, ArgList args)
{
    MemoryWriter w;
    w.write(format_str, args);
    return w.str();
}

inline std::wstring format(WCStringRef format_str, ArgList args)
{
    WMemoryWriter w;
    w.write(format_str, args);
    return w.str();
}

/**
  \rst
  Prints formatted data to the file *f*.

  **Example**::

    print(stderr, "Don't {}!", "panic");
  \endrst
 */
FMT_API void print(std::FILE *f, CStringRef format_str, ArgList args);

/**
  \rst
  Prints formatted data to ``stdout``.

  **Example**::

    print("Elapsed time: {0:.2f} seconds", 1.23);
  \endrst
 */
FMT_API void print(CStringRef format_str, ArgList args);

template <typename Char>
void printf(BasicWriter<Char> &w, BasicCStringRef<Char> format, ArgList args)
{
    internal::PrintfFormatter<Char>(args).format(w, format);
}

/**
  \rst
  Formats arguments and returns the result as a string.

  **Example**::

    std::string message = fmt::sprintf("The answer is %d", 42);
  \endrst
*/
inline std::string sprintf(CStringRef format, ArgList args)
{
    MemoryWriter w;
    printf(w, format, args);
    return w.str();
}

inline std::wstring sprintf(WCStringRef format, ArgList args)
{
    WMemoryWriter w;
    printf(w, format, args);
    return w.str();
}

/**
  \rst
  Prints formatted data to the file *f*.

  **Example**::

    fmt::fprintf(stderr, "Don't %s!", "panic");
  \endrst
 */
FMT_API int fprintf(std::FILE *f, CStringRef format, ArgList args);

/**
  \rst
  Prints formatted data to ``stdout``.

  **Example**::

    fmt::printf("Elapsed time: %.2f seconds", 1.23);
  \endrst
 */
inline int printf(CStringRef format, ArgList args)
{
    return fprintf(stdout, format, args);
}

/**
  Fast integer formatter.
 */
class FormatInt
{
private:
    // Buffer should be large enough to hold all digits (digits10 + 1),
    // a sign and a null character.
    enum {BUFFER_SIZE = std::numeric_limits<ULongLong>::digits10 + 3};
    mutable char buffer_[BUFFER_SIZE];
    char *str_;

    // Formats value in reverse and returns the number of digits.
    char *format_decimal(ULongLong value)
    {
        char *buffer_end = buffer_ + BUFFER_SIZE - 1;
        while (value >= 100)
        {
            // Integer division is slow so do it for a group of two digits instead
            // of for every digit. The idea comes from the talk by Alexandrescu
            // "Three Optimization Tips for C++". See speed-test for a comparison.
            unsigned index = static_cast<unsigned>((value % 100) * 2);
            value /= 100;
            *--buffer_end = internal::Data::DIGITS[index + 1];
            *--buffer_end = internal::Data::DIGITS[index];
        }
        if (value < 10)
        {
            *--buffer_end = static_cast<char>('0' + value);
            return buffer_end;
        }
        unsigned index = static_cast<unsigned>(value * 2);
        *--buffer_end = internal::Data::DIGITS[index + 1];
        *--buffer_end = internal::Data::DIGITS[index];
        return buffer_end;
    }

    void FormatSigned(LongLong value)
    {
        ULongLong abs_value = static_cast<ULongLong>(value);
        bool negative = value < 0;
        if (negative)
            abs_value = 0 - abs_value;
        str_ = format_decimal(abs_value);
        if (negative)
            *--str_ = '-';
    }

public:
    explicit FormatInt(int value)
    {
        FormatSigned(value);
    }
    explicit FormatInt(long value)
    {
        FormatSigned(value);
    }
    explicit FormatInt(LongLong value)
    {
        FormatSigned(value);
    }
    explicit FormatInt(unsigned value) : str_(format_decimal(value)) {}
    explicit FormatInt(unsigned long value) : str_(format_decimal(value)) {}
    explicit FormatInt(ULongLong value) : str_(format_decimal(value)) {}

    /** Returns the number of characters written to the output buffer. */
    std::size_t size() const
    {
        return internal::to_unsigned(buffer_ - str_ + BUFFER_SIZE - 1);
    }

    /**
      Returns a pointer to the output buffer content. No terminating null
      character is appended.
     */
    const char *data() const
    {
        return str_;
    }

    /**
      Returns a pointer to the output buffer content with terminating null
      character appended.
     */
    const char *c_str() const
    {
        buffer_[BUFFER_SIZE - 1] = '\0';
        return str_;
    }

    /**
      \rst
      Returns the content of the output buffer as an ``std::string``.
      \endrst
     */
    std::string str() const
    {
        return std::string(str_, size());
    }
};

// Formats a decimal integer value writing into buffer and returns
// a pointer to the end of the formatted string. This function doesn't
// write a terminating null character.
template <typename T>
inline void format_decimal(char *&buffer, T value)
{
    typedef typename internal::IntTraits<T>::MainType MainType;
    MainType abs_value = static_cast<MainType>(value);
    if (internal::is_negative(value))
    {
        *buffer++ = '-';
        abs_value = 0 - abs_value;
    }
    if (abs_value < 100)
    {
        if (abs_value < 10)
        {
            *buffer++ = static_cast<char>('0' + abs_value);
            return;
        }
        unsigned index = static_cast<unsigned>(abs_value * 2);
        *buffer++ = internal::Data::DIGITS[index];
        *buffer++ = internal::Data::DIGITS[index + 1];
        return;
    }
    unsigned num_digits = internal::count_digits(abs_value);
    internal::format_decimal(buffer, abs_value, num_digits);
    buffer += num_digits;
}

/**
  \rst
  Returns a named argument for formatting functions.

  **Example**::

    print("Elapsed time: {s:.2f} seconds", arg("s", 1.23));

  \endrst
 */
template <typename T>
inline internal::NamedArg<char> arg(StringRef name, const T &arg)
{
    return internal::NamedArg<char>(name, arg);
}

template <typename T>
inline internal::NamedArg<wchar_t> arg(WStringRef name, const T &arg)
{
    return internal::NamedArg<wchar_t>(name, arg);
}

// The following two functions are deleted intentionally to disable
// nested named arguments as in ``format("{}", arg("a", arg("b", 42)))``.
template <typename Char>
void arg(StringRef, const internal::NamedArg<Char>&) FMT_DELETED_OR_UNDEFINED;
template <typename Char>
void arg(WStringRef, const internal::NamedArg<Char>&) FMT_DELETED_OR_UNDEFINED;
}

#if FMT_GCC_VERSION
// Use the system_header pragma to suppress warnings about variadic macros
// because suppressing -Wvariadic-macros with the diagnostic pragma doesn't
// work. It is used at the end because we want to suppress as little warnings
// as possible.
# pragma GCC system_header
#endif

// This is used to work around VC++ bugs in handling variadic macros.
#define FMT_EXPAND(args) args

// Returns the number of arguments.
// Based on https://groups.google.com/forum/#!topic/comp.std.c/d-6Mj5Lko_s.
#define FMT_NARG(...) FMT_NARG_(__VA_ARGS__, FMT_RSEQ_N())
#define FMT_NARG_(...) FMT_EXPAND(FMT_ARG_N(__VA_ARGS__))
#define FMT_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define FMT_RSEQ_N() 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define FMT_CONCAT(a, b) a##b
#define FMT_FOR_EACH_(N, f, ...) \
  FMT_EXPAND(FMT_CONCAT(FMT_FOR_EACH, N)(f, __VA_ARGS__))
#define FMT_FOR_EACH(f, ...) \
  FMT_EXPAND(FMT_FOR_EACH_(FMT_NARG(__VA_ARGS__), f, __VA_ARGS__))

#define FMT_ADD_ARG_NAME(type, index) type arg##index
#define FMT_GET_ARG_NAME(type, index) arg##index

#if FMT_USE_VARIADIC_TEMPLATES
# define FMT_VARIADIC_(Char, ReturnType, func, call, ...) \
  template <typename... Args> \
  ReturnType func(FMT_FOR_EACH(FMT_ADD_ARG_NAME, __VA_ARGS__), \
      const Args & ... args) { \
    typedef fmt::internal::ArgArray<sizeof...(Args)> ArgArray; \
    typename ArgArray::Type array{ \
      ArgArray::template make<fmt::BasicFormatter<Char> >(args)...}; \
    call(FMT_FOR_EACH(FMT_GET_ARG_NAME, __VA_ARGS__), \
      fmt::ArgList(fmt::internal::make_type(args...), array)); \
  }
#else
// Defines a wrapper for a function taking __VA_ARGS__ arguments
// and n additional arguments of arbitrary types.
# define FMT_WRAP(Char, ReturnType, func, call, n, ...) \
  template <FMT_GEN(n, FMT_MAKE_TEMPLATE_ARG)> \
  inline ReturnType func(FMT_FOR_EACH(FMT_ADD_ARG_NAME, __VA_ARGS__), \
      FMT_GEN(n, FMT_MAKE_ARG)) { \
    fmt::internal::ArgArray<n>::Type arr; \
    FMT_GEN(n, FMT_ASSIGN_##Char); \
    call(FMT_FOR_EACH(FMT_GET_ARG_NAME, __VA_ARGS__), fmt::ArgList( \
      fmt::internal::make_type(FMT_GEN(n, FMT_MAKE_REF2)), arr)); \
  }

# define FMT_VARIADIC_(Char, ReturnType, func, call, ...) \
  inline ReturnType func(FMT_FOR_EACH(FMT_ADD_ARG_NAME, __VA_ARGS__)) { \
    call(FMT_FOR_EACH(FMT_GET_ARG_NAME, __VA_ARGS__), fmt::ArgList()); \
  } \
  FMT_WRAP(Char, ReturnType, func, call, 1, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 2, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 3, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 4, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 5, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 6, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 7, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 8, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 9, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 10, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 11, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 12, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 13, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 14, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 15, __VA_ARGS__)
#endif  // FMT_USE_VARIADIC_TEMPLATES

/**
  \rst
  Defines a variadic function with the specified return type, function name
  and argument types passed as variable arguments to this macro.

  **Example**::

    void print_error(const char *file, int line, const char *format,
                     fmt::ArgList args) {
      fmt::print("{}: {}: ", file, line);
      fmt::print(format, args);
    }
    FMT_VARIADIC(void, print_error, const char *, int, const char *)

  ``FMT_VARIADIC`` is used for compatibility with legacy C++ compilers that
  don't implement variadic templates. You don't have to use this macro if
  you don't need legacy compiler support and can use variadic templates
  directly::

    template <typename... Args>
    void print_error(const char *file, int line, const char *format,
                     const Args & ... args) {
      fmt::print("{}: {}: ", file, line);
      fmt::print(format, args...);
    }
  \endrst
 */
#define FMT_VARIADIC(ReturnType, func, ...) \
  FMT_VARIADIC_(char, ReturnType, func, return func, __VA_ARGS__)

#define FMT_VARIADIC_W(ReturnType, func, ...) \
  FMT_VARIADIC_(wchar_t, ReturnType, func, return func, __VA_ARGS__)

#define FMT_CAPTURE_ARG_(id, index) ::fmt::arg(#id, id)

#define FMT_CAPTURE_ARG_W_(id, index) ::fmt::arg(L###id, id)

/**
  \rst
  Convenient macro to capture the arguments' names and values into several
  ``fmt::arg(name, value)``.

  **Example**::

    int x = 1, y = 2;
    print("point: ({x}, {y})", FMT_CAPTURE(x, y));
    // same as:
    // print("point: ({x}, {y})", arg("x", x), arg("y", y));

  \endrst
 */
#define FMT_CAPTURE(...) FMT_FOR_EACH(FMT_CAPTURE_ARG_, __VA_ARGS__)

#define FMT_CAPTURE_W(...) FMT_FOR_EACH(FMT_CAPTURE_ARG_W_, __VA_ARGS__)

namespace fmt
{
FMT_VARIADIC(std::string, format, CStringRef)
FMT_VARIADIC_W(std::wstring, format, WCStringRef)
FMT_VARIADIC(void, print, CStringRef)
FMT_VARIADIC(void, print, std::FILE *, CStringRef)

FMT_VARIADIC(void, print_colored, Color, CStringRef)
FMT_VARIADIC(std::string, sprintf, CStringRef)
FMT_VARIADIC_W(std::wstring, sprintf, WCStringRef)
FMT_VARIADIC(int, printf, CStringRef)
FMT_VARIADIC(int, fprintf, std::FILE *, CStringRef)

namespace internal
{
template <typename Char>
inline bool is_name_start(Char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || '_' == c;
}

// Parses an unsigned integer advancing s to the end of the parsed input.
// This function assumes that the first character of s is a digit.
template <typename Char>
unsigned parse_nonnegative_int(const Char *&s)
{
    assert('0' <= *s && *s <= '9');
    unsigned value = 0;
    do
    {
        unsigned new_value = value * 10 + (*s++ - '0');
        // Check if value wrapped around.
        if (new_value < value)
        {
            value = (std::numeric_limits<unsigned>::max)();
            break;
        }
        value = new_value;
    }
    while ('0' <= *s && *s <= '9');
    // Convert to unsigned to prevent a warning.
    unsigned max_int = (std::numeric_limits<int>::max)();
    if (value > max_int)
        FMT_THROW(FormatError("number is too big"));
    return value;
}

inline void require_numeric_argument(const Arg &arg, char spec)
{
    if (arg.type > Arg::LAST_NUMERIC_TYPE)
    {
        std::string message =
            fmt::format("format specifier '{}' requires numeric argument", spec);
        FMT_THROW(fmt::FormatError(message));
    }
}

template <typename Char>
void check_sign(const Char *&s, const Arg &arg)
{
    char sign = static_cast<char>(*s);
    require_numeric_argument(arg, sign);
    if (arg.type == Arg::UINT || arg.type == Arg::ULONG_LONG)
    {
        FMT_THROW(FormatError(fmt::format(
                                  "format specifier '{}' requires signed argument", sign)));
    }
    ++s;
}
}  // namespace internal

template <typename Char, typename AF>
inline internal::Arg BasicFormatter<Char, AF>::get_arg(
    BasicStringRef<Char> arg_name, const char *&error)
{
    if (check_no_auto_index(error))
    {
        map_.init(args());
        const internal::Arg *arg = map_.find(arg_name);
        if (arg)
            return *arg;
        error = "argument not found";
    }
    return internal::Arg();
}

template <typename Char, typename AF>
inline internal::Arg BasicFormatter<Char, AF>::parse_arg_index(const Char *&s)
{
    const char *error = 0;
    internal::Arg arg = *s < '0' || *s > '9' ?
                        next_arg(error) : get_arg(internal::parse_nonnegative_int(s), error);
    if (error)
    {
        FMT_THROW(FormatError(
                      *s != '}' && *s != ':' ? "invalid format string" : error));
    }
    return arg;
}

template <typename Char, typename AF>
inline internal::Arg BasicFormatter<Char, AF>::parse_arg_name(const Char *&s)
{
    assert(internal::is_name_start(*s));
    const Char *start = s;
    Char c;
    do
    {
        c = *++s;
    }
    while (internal::is_name_start(c) || ('0' <= c && c <= '9'));
    const char *error = 0;
    internal::Arg arg = get_arg(BasicStringRef<Char>(start, s - start), error);
    if (error)
        FMT_THROW(FormatError(error));
    return arg;
}

template <typename Char, typename ArgFormatter>
const Char *BasicFormatter<Char, ArgFormatter>::format(
    const Char *&format_str, const internal::Arg &arg)
{
    using internal::Arg;
    const Char *s = format_str;
    FormatSpec spec;
    if (*s == ':')
    {
        if (arg.type == Arg::CUSTOM)
        {
            arg.custom.format(this, arg.custom.value, &s);
            return s;
        }
        ++s;
        // Parse fill and alignment.
        if (Char c = *s)
        {
            const Char *p = s + 1;
            spec.align_ = ALIGN_DEFAULT;
            do
            {
                switch (*p)
                {
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
                if (spec.align_ != ALIGN_DEFAULT)
                {
                    if (p != s)
                    {
                        if (c == '}') break;
                        if (c == '{')
                            FMT_THROW(FormatError("invalid fill character '{'"));
                        s += 2;
                        spec.fill_ = c;
                    }
                    else ++s;
                    if (spec.align_ == ALIGN_NUMERIC)
                        require_numeric_argument(arg, '=');
                    break;
                }
            }
            while (--p >= s);
        }

        // Parse sign.
        switch (*s)
        {
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

        if (*s == '#')
        {
            require_numeric_argument(arg, '#');
            spec.flags_ |= HASH_FLAG;
            ++s;
        }

        // Parse zero flag.
        if (*s == '0')
        {
            require_numeric_argument(arg, '0');
            spec.align_ = ALIGN_NUMERIC;
            spec.fill_ = '0';
            ++s;
        }

        // Parse width.
        if ('0' <= *s && *s <= '9')
        {
            spec.width_ = internal::parse_nonnegative_int(s);
        }
        else if (*s == '{')
        {
            ++s;
            Arg width_arg = internal::is_name_start(*s) ?
                            parse_arg_name(s) : parse_arg_index(s);
            if (*s++ != '}')
                FMT_THROW(FormatError("invalid format string"));
            ULongLong value = 0;
            switch (width_arg.type)
            {
            case Arg::INT:
                if (width_arg.int_value < 0)
                    FMT_THROW(FormatError("negative width"));
                value = width_arg.int_value;
                break;
            case Arg::UINT:
                value = width_arg.uint_value;
                break;
            case Arg::LONG_LONG:
                if (width_arg.long_long_value < 0)
                    FMT_THROW(FormatError("negative width"));
                value = width_arg.long_long_value;
                break;
            case Arg::ULONG_LONG:
                value = width_arg.ulong_long_value;
                break;
            default:
                FMT_THROW(FormatError("width is not integer"));
            }
            if (value > (std::numeric_limits<int>::max)())
                FMT_THROW(FormatError("number is too big"));
            spec.width_ = static_cast<int>(value);
        }

        // Parse precision.
        if (*s == '.')
        {
            ++s;
            spec.precision_ = 0;
            if ('0' <= *s && *s <= '9')
            {
                spec.precision_ = internal::parse_nonnegative_int(s);
            }
            else if (*s == '{')
            {
                ++s;
                Arg precision_arg = internal::is_name_start(*s) ?
                                    parse_arg_name(s) : parse_arg_index(s);
                if (*s++ != '}')
                    FMT_THROW(FormatError("invalid format string"));
                ULongLong value = 0;
                switch (precision_arg.type)
                {
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
                if (value > (std::numeric_limits<int>::max)())
                    FMT_THROW(FormatError("number is too big"));
                spec.precision_ = static_cast<int>(value);
            }
            else
            {
                FMT_THROW(FormatError("missing precision specifier"));
            }
            if (arg.type <= Arg::LAST_INTEGER_TYPE || arg.type == Arg::POINTER)
            {
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

    // Format argument.
    ArgFormatter(*this, spec, s - 1).visit(arg);
    return s;
}

template <typename Char, typename AF>
void BasicFormatter<Char, AF>::format(BasicCStringRef<Char> format_str)
{
    const Char *s = format_str.c_str();
    const Char *start = s;
    while (*s)
    {
        Char c = *s++;
        if (c != '{' && c != '}') continue;
        if (*s == c)
        {
            write(writer_, start, s);
            start = ++s;
            continue;
        }
        if (c == '}')
            FMT_THROW(FormatError("unmatched '}' in format string"));
        write(writer_, start, s - 1);
        internal::Arg arg = internal::is_name_start(*s) ?
                            parse_arg_name(s) : parse_arg_index(s);
        start = s = format(s, arg);
    }
    write(writer_, start, s);
}
}  // namespace fmt

#if FMT_USE_USER_DEFINED_LITERALS
namespace fmt
{
namespace internal
{

template <typename Char>
struct UdlFormat
{
    const Char *str;

    template <typename... Args>
    auto operator()(Args && ... args) const
    -> decltype(format(str, std::forward<Args>(args)...))
    {
        return format(str, std::forward<Args>(args)...);
    }
};

template <typename Char>
struct UdlArg
{
    const Char *str;

    template <typename T>
    NamedArg<Char> operator=(T &&value) const
    {
        return {str, std::forward<T>(value)};
    }
};

} // namespace internal

inline namespace literals
{

/**
  \rst
  C++11 literal equivalent of :func:`fmt::format`.

  **Example**::

    using namespace fmt::literals;
    std::string message = "The answer is {}"_format(42);
  \endrst
 */
inline internal::UdlFormat<char>
operator"" _format(const char *s, std::size_t)
{
    return {s};
}
inline internal::UdlFormat<wchar_t>
operator"" _format(const wchar_t *s, std::size_t)
{
    return {s};
}

/**
  \rst
  C++11 literal equivalent of :func:`fmt::arg`.

  **Example**::

    using namespace fmt::literals;
    print("Elapsed time: {s:.2f} seconds", "s"_a=1.23);
  \endrst
 */
inline internal::UdlArg<char>
operator"" _a(const char *s, std::size_t)
{
    return {s};
}
inline internal::UdlArg<wchar_t>
operator"" _a(const wchar_t *s, std::size_t)
{
    return {s};
}

} // inline namespace literals
} // namespace fmt
#endif // FMT_USE_USER_DEFINED_LITERALS

// Restore warnings.
#if FMT_GCC_VERSION >= 406
# pragma GCC diagnostic pop
#endif

#if defined(__clang__) && !defined(FMT_ICC_VERSION)
# pragma clang diagnostic pop
#endif

#ifdef FMT_HEADER_ONLY
# define FMT_FUNC inline
# include "format.cc"
#else
# define FMT_FUNC
#endif

#endif  // FMT_FORMAT_H_
