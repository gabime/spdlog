// Formatting library for C++ - the core API
//
// Copyright (c) 2012 - present, Victor Zverovich
// All rights reserved.
//
// For the license information refer to format.h.

#ifndef FMT_CORE_H_
#define FMT_CORE_H_

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <string>
#include <type_traits>

// The fmt library version in the form major * 10000 + minor * 100 + patch.
#define FMT_VERSION 50100

#ifdef __has_feature
#define FMT_HAS_FEATURE(x) __has_feature(x)
#else
#define FMT_HAS_FEATURE(x) 0
#endif

#ifdef __has_include
#define FMT_HAS_INCLUDE(x) __has_include(x)
#else
#define FMT_HAS_INCLUDE(x) 0
#endif

#ifdef __has_cpp_attribute
#define FMT_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define FMT_HAS_CPP_ATTRIBUTE(x) 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define FMT_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#else
#define FMT_GCC_VERSION 0
#endif

#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
#define FMT_HAS_GXX_CXX11 FMT_GCC_VERSION
#else
#define FMT_HAS_GXX_CXX11 0
#endif

#ifdef _MSC_VER
#define FMT_MSC_VER _MSC_VER
#else
#define FMT_MSC_VER 0
#endif

// Check if relaxed c++14 constexpr is supported.
// GCC doesn't allow throw in constexpr until version 6 (bug 67371).
#ifndef FMT_USE_CONSTEXPR
#define FMT_USE_CONSTEXPR                                                                                                                  \
    (FMT_HAS_FEATURE(cxx_relaxed_constexpr) || FMT_MSC_VER >= 1910 || (FMT_GCC_VERSION >= 600 && __cplusplus >= 201402L))
#endif
#if FMT_USE_CONSTEXPR
#define FMT_CONSTEXPR constexpr
#define FMT_CONSTEXPR_DECL constexpr
#else
#define FMT_CONSTEXPR inline
#define FMT_CONSTEXPR_DECL
#endif

#ifndef FMT_OVERRIDE
#if FMT_HAS_FEATURE(cxx_override) || (FMT_GCC_VERSION >= 408 && FMT_HAS_GXX_CXX11) || FMT_MSC_VER >= 1900
#define FMT_OVERRIDE override
#else
#define FMT_OVERRIDE
#endif
#endif

#if FMT_HAS_FEATURE(cxx_explicit_conversions) || FMT_MSC_VER >= 1800
#define FMT_EXPLICIT explicit
#else
#define FMT_EXPLICIT
#endif

#ifndef FMT_NULL
#if FMT_HAS_FEATURE(cxx_nullptr) || (FMT_GCC_VERSION >= 408 && FMT_HAS_GXX_CXX11) || FMT_MSC_VER >= 1600
#define FMT_NULL nullptr
#define FMT_USE_NULLPTR 1
#else
#define FMT_NULL NULL
#endif
#endif

#ifndef FMT_USE_NULLPTR
#define FMT_USE_NULLPTR 0
#endif

#if FMT_HAS_CPP_ATTRIBUTE(noreturn)
#define FMT_NORETURN [[noreturn]]
#else
#define FMT_NORETURN
#endif

// Check if exceptions are disabled.
#if defined(__GNUC__) && !defined(__EXCEPTIONS)
#define FMT_EXCEPTIONS 0
#elif FMT_MSC_VER && !_HAS_EXCEPTIONS
#define FMT_EXCEPTIONS 0
#endif
#ifndef FMT_EXCEPTIONS
#define FMT_EXCEPTIONS 1
#endif

// Define FMT_USE_NOEXCEPT to make fmt use noexcept (C++11 feature).
#ifndef FMT_USE_NOEXCEPT
#define FMT_USE_NOEXCEPT 0
#endif

#if FMT_USE_NOEXCEPT || FMT_HAS_FEATURE(cxx_noexcept) || (FMT_GCC_VERSION >= 408 && FMT_HAS_GXX_CXX11) || FMT_MSC_VER >= 1900
#define FMT_DETECTED_NOEXCEPT noexcept
#else
#define FMT_DETECTED_NOEXCEPT throw()
#endif

#ifndef FMT_NOEXCEPT
#if FMT_EXCEPTIONS
#define FMT_NOEXCEPT FMT_DETECTED_NOEXCEPT
#else
#define FMT_NOEXCEPT
#endif
#endif

// This is needed because GCC still uses throw() in its headers when exceptions
// are disabled.
#if FMT_GCC_VERSION
#define FMT_DTOR_NOEXCEPT FMT_DETECTED_NOEXCEPT
#else
#define FMT_DTOR_NOEXCEPT FMT_NOEXCEPT
#endif

#ifndef FMT_BEGIN_NAMESPACE
#if FMT_HAS_FEATURE(cxx_inline_namespaces) || FMT_GCC_VERSION >= 404 || FMT_MSC_VER >= 1900
#define FMT_INLINE_NAMESPACE inline namespace
#define FMT_END_NAMESPACE                                                                                                                  \
    }                                                                                                                                      \
    }
#else
#define FMT_INLINE_NAMESPACE namespace
#define FMT_END_NAMESPACE                                                                                                                  \
    }                                                                                                                                      \
    using namespace v5;                                                                                                                    \
    }
#endif
#define FMT_BEGIN_NAMESPACE                                                                                                                \
    namespace fmt {                                                                                                                        \
    FMT_INLINE_NAMESPACE v5                                                                                                                \
    {
#endif

#if !defined(FMT_HEADER_ONLY) && defined(_WIN32)
#ifdef FMT_EXPORT
#define FMT_API __declspec(dllexport)
#elif defined(FMT_SHARED)
#define FMT_API __declspec(dllimport)
#endif
#endif
#ifndef FMT_API
#define FMT_API
#endif

#ifndef FMT_ASSERT
#define FMT_ASSERT(condition, message) assert((condition) && message)
#endif

#define FMT_DELETED = delete

// A macro to disallow the copy construction and assignment.
#define FMT_DISALLOW_COPY_AND_ASSIGN(Type)                                                                                                 \
    Type(const Type &) FMT_DELETED;                                                                                                        \
    void operator=(const Type &) FMT_DELETED

// libc++ supports string_view in pre-c++17.
#if (FMT_HAS_INCLUDE(<string_view>) && (__cplusplus > 201402L || defined(_LIBCPP_VERSION))) ||                                             \
    (defined(_MSVC_LANG) && _MSVC_LANG > 201402L && _MSC_VER >= 1910)
#include <string_view>
#define FMT_USE_STD_STRING_VIEW
#elif (FMT_HAS_INCLUDE(<experimental / string_view>) && __cplusplus >= 201402L)
#include <experimental/string_view>
#define FMT_USE_EXPERIMENTAL_STRING_VIEW
#endif

// std::result_of is defined in <functional> in gcc 4.4.
#if FMT_GCC_VERSION && FMT_GCC_VERSION <= 404
#include <functional>
#endif

FMT_BEGIN_NAMESPACE

namespace internal {

// An implementation of declval for pre-C++11 compilers such as gcc 4.
template<typename T>
typename std::add_rvalue_reference<T>::type declval() FMT_NOEXCEPT;

// Casts nonnegative integer to unsigned.
template<typename Int>
FMT_CONSTEXPR typename std::make_unsigned<Int>::type to_unsigned(Int value)
{
    FMT_ASSERT(value >= 0, "negative value");
    return static_cast<typename std::make_unsigned<Int>::type>(value);
}

} // namespace internal

/**
  An implementation of ``std::basic_string_view`` for pre-C++17. It provides a
  subset of the API. ``fmt::basic_string_view`` is used for format strings even
  if ``std::string_view`` is available to prevent issues when a library is
  compiled with a different ``-std`` option than the client code (which is not
  recommended).
 */
template<typename Char>
class basic_string_view
{
private:
    const Char *data_;
    size_t size_;

public:
    typedef Char char_type;
    typedef const Char *iterator;

    // Standard basic_string_view type.
#if defined(FMT_USE_STD_STRING_VIEW)
    typedef std::basic_string_view<Char> type;
#elif defined(FMT_USE_EXPERIMENTAL_STRING_VIEW)
    typedef std::experimental::basic_string_view<Char> type;
#else
    struct type
    {
        const char *data() const
        {
            return FMT_NULL;
        }
        size_t size() const
        {
            return 0;
        }
    };
#endif

    FMT_CONSTEXPR basic_string_view() FMT_NOEXCEPT : data_(FMT_NULL), size_(0) {}

    /** Constructs a string reference object from a C string and a size. */
    FMT_CONSTEXPR basic_string_view(const Char *s, size_t count) FMT_NOEXCEPT : data_(s), size_(count) {}

    /**
      \rst
      Constructs a string reference object from a C string computing
      the size with ``std::char_traits<Char>::length``.
      \endrst
     */
    basic_string_view(const Char *s)
        : data_(s)
        , size_(std::char_traits<Char>::length(s))
    {
    }

    /** Constructs a string reference from a ``std::basic_string`` object. */
    template<typename Alloc>
    FMT_CONSTEXPR basic_string_view(const std::basic_string<Char, Alloc> &s) FMT_NOEXCEPT : data_(s.c_str()), size_(s.size())
    {
    }

    FMT_CONSTEXPR basic_string_view(type s) FMT_NOEXCEPT : data_(s.data()), size_(s.size()) {}

    /** Returns a pointer to the string data. */
    const Char *data() const
    {
        return data_;
    }

    /** Returns the string size. */
    FMT_CONSTEXPR size_t size() const
    {
        return size_;
    }

    FMT_CONSTEXPR iterator begin() const
    {
        return data_;
    }
    FMT_CONSTEXPR iterator end() const
    {
        return data_ + size_;
    }

    FMT_CONSTEXPR void remove_prefix(size_t n)
    {
        data_ += n;
        size_ -= n;
    }

    // Lexicographically compare this string reference to other.
    int compare(basic_string_view other) const
    {
        size_t str_size = size_ < other.size_ ? size_ : other.size_;
        int result = std::char_traits<Char>::compare(data_, other.data_, str_size);
        if (result == 0)
            result = size_ == other.size_ ? 0 : (size_ < other.size_ ? -1 : 1);
        return result;
    }

    friend bool operator==(basic_string_view lhs, basic_string_view rhs)
    {
        return lhs.compare(rhs) == 0;
    }
    friend bool operator!=(basic_string_view lhs, basic_string_view rhs)
    {
        return lhs.compare(rhs) != 0;
    }
    friend bool operator<(basic_string_view lhs, basic_string_view rhs)
    {
        return lhs.compare(rhs) < 0;
    }
    friend bool operator<=(basic_string_view lhs, basic_string_view rhs)
    {
        return lhs.compare(rhs) <= 0;
    }
    friend bool operator>(basic_string_view lhs, basic_string_view rhs)
    {
        return lhs.compare(rhs) > 0;
    }
    friend bool operator>=(basic_string_view lhs, basic_string_view rhs)
    {
        return lhs.compare(rhs) >= 0;
    }
};

typedef basic_string_view<char> string_view;
typedef basic_string_view<wchar_t> wstring_view;

template<typename Context>
class basic_format_arg;

template<typename Context>
class basic_format_args;

// A formatter for objects of type T.
template<typename T, typename Char = char, typename Enable = void>
struct formatter;

namespace internal {

/** A contiguous memory buffer with an optional growing ability. */
template<typename T>
class basic_buffer
{
private:
    FMT_DISALLOW_COPY_AND_ASSIGN(basic_buffer);

    T *ptr_;
    std::size_t size_;
    std::size_t capacity_;

protected:
    basic_buffer(T *p = FMT_NULL, std::size_t sz = 0, std::size_t cap = 0) FMT_NOEXCEPT : ptr_(p), size_(sz), capacity_(cap) {}

    /** Sets the buffer data and capacity. */
    void set(T *buf_data, std::size_t buf_capacity) FMT_NOEXCEPT
    {
        ptr_ = buf_data;
        capacity_ = buf_capacity;
    }

    /** Increases the buffer capacity to hold at least *capacity* elements. */
    virtual void grow(std::size_t capacity) = 0;

public:
    typedef T value_type;
    typedef const T &const_reference;

    virtual ~basic_buffer() {}

    T *begin() FMT_NOEXCEPT
    {
        return ptr_;
    }
    T *end() FMT_NOEXCEPT
    {
        return ptr_ + size_;
    }

    /** Returns the size of this buffer. */
    std::size_t size() const FMT_NOEXCEPT
    {
        return size_;
    }

    /** Returns the capacity of this buffer. */
    std::size_t capacity() const FMT_NOEXCEPT
    {
        return capacity_;
    }

    /** Returns a pointer to the buffer data. */
    T *data() FMT_NOEXCEPT
    {
        return ptr_;
    }

    /** Returns a pointer to the buffer data. */
    const T *data() const FMT_NOEXCEPT
    {
        return ptr_;
    }

    /**
      Resizes the buffer. If T is a POD type new elements may not be initialized.
     */
    void resize(std::size_t new_size)
    {
        reserve(new_size);
        size_ = new_size;
    }

    /** Reserves space to store at least *capacity* elements. */
    void reserve(std::size_t new_capacity)
    {
        if (new_capacity > capacity_)
            grow(new_capacity);
    }

    void push_back(const T &value)
    {
        reserve(size_ + 1);
        ptr_[size_++] = value;
    }

    /** Appends data to the end of the buffer. */
    template<typename U>
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

typedef basic_buffer<char> buffer;
typedef basic_buffer<wchar_t> wbuffer;

// A container-backed buffer.
template<typename Container>
class container_buffer : public basic_buffer<typename Container::value_type>
{
private:
    Container &container_;

protected:
    void grow(std::size_t capacity) FMT_OVERRIDE
    {
        container_.resize(capacity);
        this->set(&container_[0], capacity);
    }

public:
    explicit container_buffer(Container &c)
        : basic_buffer<typename Container::value_type>(&c[0], c.size(), c.size())
        , container_(c)
    {
    }
};

struct error_handler
{
    FMT_CONSTEXPR error_handler() {}
    FMT_CONSTEXPR error_handler(const error_handler &) {}

    // This function is intentionally not constexpr to give a compile-time error.
    FMT_API void on_error(const char *message);
};

// Formatting of wide characters and strings into a narrow output is disallowed:
//   fmt::format("{}", L"test"); // error
// To fix this, use a wide format string:
//   fmt::format(L"{}", L"test");
template<typename Char>
inline void require_wchar()
{
    static_assert(std::is_same<wchar_t, Char>::value, "formatting of wide characters into a narrow output is disallowed");
}

template<typename Char>
struct named_arg_base;

template<typename T, typename Char>
struct named_arg;

template<typename T>
struct is_named_arg : std::false_type
{
};

template<typename T, typename Char>
struct is_named_arg<named_arg<T, Char>> : std::true_type
{
};

enum type
{
    none_type,
    named_arg_type,
    // Integer types should go first,
    int_type,
    uint_type,
    long_long_type,
    ulong_long_type,
    bool_type,
    char_type,
    last_integer_type = char_type,
    // followed by floating-point types.
    double_type,
    long_double_type,
    last_numeric_type = long_double_type,
    cstring_type,
    string_type,
    pointer_type,
    custom_type
};

FMT_CONSTEXPR bool is_integral(type t)
{
    FMT_ASSERT(t != internal::named_arg_type, "invalid argument type");
    return t > internal::none_type && t <= internal::last_integer_type;
}

FMT_CONSTEXPR bool is_arithmetic(type t)
{
    FMT_ASSERT(t != internal::named_arg_type, "invalid argument type");
    return t > internal::none_type && t <= internal::last_numeric_type;
}

template<typename T, typename Char, bool ENABLE = true>
struct convert_to_int
{
    enum
    {
        value = !std::is_arithmetic<T>::value && std::is_convertible<T, int>::value
    };
};

template<typename Char>
struct string_value
{
    const Char *value;
    std::size_t size;
};

template<typename Context>
struct custom_value
{
    const void *value;
    void (*format)(const void *arg, Context &ctx);
};

// A formatting argument value.
template<typename Context>
class value
{
public:
    typedef typename Context::char_type char_type;

    union
    {
        int int_value;
        unsigned uint_value;
        long long long_long_value;
        unsigned long long ulong_long_value;
        double double_value;
        long double long_double_value;
        const void *pointer;
        string_value<char_type> string;
        string_value<signed char> sstring;
        string_value<unsigned char> ustring;
        custom_value<Context> custom;
    };

    FMT_CONSTEXPR value(int val = 0)
        : int_value(val)
    {
    }
    value(unsigned val)
    {
        uint_value = val;
    }
    value(long long val)
    {
        long_long_value = val;
    }
    value(unsigned long long val)
    {
        ulong_long_value = val;
    }
    value(double val)
    {
        double_value = val;
    }
    value(long double val)
    {
        long_double_value = val;
    }
    value(const char_type *val)
    {
        string.value = val;
    }
    value(const signed char *val)
    {
        static_assert(std::is_same<char, char_type>::value, "incompatible string types");
        sstring.value = val;
    }
    value(const unsigned char *val)
    {
        static_assert(std::is_same<char, char_type>::value, "incompatible string types");
        ustring.value = val;
    }
    value(basic_string_view<char_type> val)
    {
        string.value = val.data();
        string.size = val.size();
    }
    value(const void *val)
    {
        pointer = val;
    }

    template<typename T>
    explicit value(const T &val)
    {
        custom.value = &val;
        custom.format = &format_custom_arg<T>;
    }

    const named_arg_base<char_type> &as_named_arg()
    {
        return *static_cast<const named_arg_base<char_type> *>(pointer);
    }

private:
    // Formats an argument of a custom type, such as a user-defined class.
    template<typename T>
    static void format_custom_arg(const void *arg, Context &ctx)
    {
        // Get the formatter type through the context to allow different contexts
        // have different extension points, e.g. `formatter<T>` for `format` and
        // `printf_formatter<T>` for `printf`.
        typename Context::template formatter_type<T>::type f;
        auto &&parse_ctx = ctx.parse_context();
        parse_ctx.advance_to(f.parse(parse_ctx));
        ctx.advance_to(f.format(*static_cast<const T *>(arg), ctx));
    }
};

template<typename Context, type TYPE>
struct typed_value : value<Context>
{
    static const type type_tag = TYPE;

    template<typename T>
    FMT_CONSTEXPR typed_value(const T &val)
        : value<Context>(val)
    {
    }
};

template<typename Context, typename T>
FMT_CONSTEXPR basic_format_arg<Context> make_arg(const T &value);

#define FMT_MAKE_VALUE(TAG, ArgType, ValueType)                                                                                            \
    template<typename C>                                                                                                                   \
    FMT_CONSTEXPR typed_value<C, TAG> make_value(ArgType val)                                                                              \
    {                                                                                                                                      \
        return static_cast<ValueType>(val);                                                                                                \
    }

#define FMT_MAKE_VALUE_SAME(TAG, Type)                                                                                                     \
    template<typename C>                                                                                                                   \
    FMT_CONSTEXPR typed_value<C, TAG> make_value(Type val)                                                                                 \
    {                                                                                                                                      \
        return val;                                                                                                                        \
    }

FMT_MAKE_VALUE(bool_type, bool, int)
FMT_MAKE_VALUE(int_type, short, int)
FMT_MAKE_VALUE(uint_type, unsigned short, unsigned)
FMT_MAKE_VALUE_SAME(int_type, int)
FMT_MAKE_VALUE_SAME(uint_type, unsigned)

// To minimize the number of types we need to deal with, long is translated
// either to int or to long long depending on its size.
typedef std::conditional<sizeof(long) == sizeof(int), int, long long>::type long_type;
FMT_MAKE_VALUE((sizeof(long) == sizeof(int) ? int_type : long_long_type), long, long_type)
typedef std::conditional<sizeof(unsigned long) == sizeof(unsigned), unsigned, unsigned long long>::type ulong_type;
FMT_MAKE_VALUE((sizeof(unsigned long) == sizeof(unsigned) ? uint_type : ulong_long_type), unsigned long, ulong_type)

FMT_MAKE_VALUE_SAME(long_long_type, long long)
FMT_MAKE_VALUE_SAME(ulong_long_type, unsigned long long)
FMT_MAKE_VALUE(int_type, signed char, int)
FMT_MAKE_VALUE(uint_type, unsigned char, unsigned)
FMT_MAKE_VALUE(char_type, char, int)

#if !defined(_MSC_VER) || defined(_NATIVE_WCHAR_T_DEFINED)
template<typename C>
inline typed_value<C, char_type> make_value(wchar_t val)
{
    require_wchar<typename C::char_type>();
    return static_cast<int>(val);
}
#endif

FMT_MAKE_VALUE(double_type, float, double)
FMT_MAKE_VALUE_SAME(double_type, double)
FMT_MAKE_VALUE_SAME(long_double_type, long double)

// Formatting of wide strings into a narrow buffer and multibyte strings
// into a wide buffer is disallowed (https://github.com/fmtlib/fmt/pull/606).
FMT_MAKE_VALUE(cstring_type, typename C::char_type *, const typename C::char_type *)
FMT_MAKE_VALUE(cstring_type, const typename C::char_type *, const typename C::char_type *)

FMT_MAKE_VALUE(cstring_type, signed char *, const signed char *)
FMT_MAKE_VALUE_SAME(cstring_type, const signed char *)
FMT_MAKE_VALUE(cstring_type, unsigned char *, const unsigned char *)
FMT_MAKE_VALUE_SAME(cstring_type, const unsigned char *)
FMT_MAKE_VALUE_SAME(string_type, basic_string_view<typename C::char_type>)
FMT_MAKE_VALUE(string_type, typename basic_string_view<typename C::char_type>::type, basic_string_view<typename C::char_type>)
FMT_MAKE_VALUE(string_type, const std::basic_string<typename C::char_type> &, basic_string_view<typename C::char_type>)
FMT_MAKE_VALUE(pointer_type, void *, const void *)
FMT_MAKE_VALUE_SAME(pointer_type, const void *)

#if FMT_USE_NULLPTR
FMT_MAKE_VALUE(pointer_type, std::nullptr_t, const void *)
#endif

// Formatting of arbitrary pointers is disallowed. If you want to output a
// pointer cast it to "void *" or "const void *". In particular, this forbids
// formatting of "[const] volatile char *" which is printed as bool by
// iostreams.
template<typename C, typename T>
typename std::enable_if<!std::is_same<T, typename C::char_type>::value>::type make_value(const T *)
{
    static_assert(!sizeof(T), "formatting of non-void pointers is disallowed");
}

template<typename C, typename T>
inline typename std::enable_if<std::is_enum<T>::value && convert_to_int<T, typename C::char_type>::value, typed_value<C, int_type>>::type
make_value(const T &val)
{
    return static_cast<int>(val);
}

template<typename C, typename T, typename Char = typename C::char_type>
inline typename std::enable_if<!convert_to_int<T, Char>::value && !std::is_convertible<T, basic_string_view<Char>>::value,
    // Implicit conversion to std::string is not handled here because it's
    // unsafe: https://github.com/fmtlib/fmt/issues/729
    typed_value<C, custom_type>>::type
make_value(const T &val)
{
    return val;
}

template<typename C, typename T>
typed_value<C, named_arg_type> make_value(const named_arg<T, typename C::char_type> &val)
{
    basic_format_arg<C> arg = make_arg<C>(val.value);
    std::memcpy(val.data, &arg, sizeof(arg));
    return static_cast<const void *>(&val);
}

// Maximum number of arguments with packed types.
enum
{
    max_packed_args = 15
};

template<typename Context>
class arg_map;

template<typename>
struct result_of;

template<typename F, typename... Args>
struct result_of<F(Args...)>
{
    // A workaround for gcc 4.4 that doesn't allow F to be a reference.
    typedef typename std::result_of<typename std::remove_reference<F>::type(Args...)>::type type;
};
} // namespace internal

// A formatting argument. It is a trivially copyable/constructible type to
// allow storage in basic_memory_buffer.
template<typename Context>
class basic_format_arg
{
private:
    internal::value<Context> value_;
    internal::type type_;

    template<typename ContextType, typename T>
    friend FMT_CONSTEXPR basic_format_arg<ContextType> internal::make_arg(const T &value);

    template<typename Visitor, typename Ctx>
    friend FMT_CONSTEXPR typename internal::result_of<Visitor(int)>::type visit(Visitor &&vis, basic_format_arg<Ctx> arg);

    friend class basic_format_args<Context>;
    friend class internal::arg_map<Context>;

    typedef typename Context::char_type char_type;

public:
    class handle
    {
    public:
        explicit handle(internal::custom_value<Context> custom)
            : custom_(custom)
        {
        }

        void format(Context &ctx) const
        {
            custom_.format(custom_.value, ctx);
        }

    private:
        internal::custom_value<Context> custom_;
    };

    FMT_CONSTEXPR basic_format_arg()
        : type_(internal::none_type)
    {
    }

    FMT_EXPLICIT operator bool() const FMT_NOEXCEPT
    {
        return type_ != internal::none_type;
    }

    internal::type type() const
    {
        return type_;
    }

    bool is_integral() const
    {
        return internal::is_integral(type_);
    }
    bool is_arithmetic() const
    {
        return internal::is_arithmetic(type_);
    }
};

// Parsing context consisting of a format string range being parsed and an
// argument counter for automatic indexing.
template<typename Char, typename ErrorHandler = internal::error_handler>
class basic_parse_context : private ErrorHandler
{
private:
    basic_string_view<Char> format_str_;
    int next_arg_id_;

public:
    typedef Char char_type;
    typedef typename basic_string_view<Char>::iterator iterator;

    explicit FMT_CONSTEXPR basic_parse_context(basic_string_view<Char> format_str, ErrorHandler eh = ErrorHandler())
        : ErrorHandler(eh)
        , format_str_(format_str)
        , next_arg_id_(0)
    {
    }

    // Returns an iterator to the beginning of the format string range being
    // parsed.
    FMT_CONSTEXPR iterator begin() const FMT_NOEXCEPT
    {
        return format_str_.begin();
    }

    // Returns an iterator past the end of the format string range being parsed.
    FMT_CONSTEXPR iterator end() const FMT_NOEXCEPT
    {
        return format_str_.end();
    }

    // Advances the begin iterator to ``it``.
    FMT_CONSTEXPR void advance_to(iterator it)
    {
        format_str_.remove_prefix(internal::to_unsigned(it - begin()));
    }

    // Returns the next argument index.
    FMT_CONSTEXPR unsigned next_arg_id();

    FMT_CONSTEXPR bool check_arg_id(unsigned)
    {
        if (next_arg_id_ > 0)
        {
            on_error("cannot switch from automatic to manual argument indexing");
            return false;
        }
        next_arg_id_ = -1;
        return true;
    }
    void check_arg_id(basic_string_view<Char>) {}

    FMT_CONSTEXPR void on_error(const char *message)
    {
        ErrorHandler::on_error(message);
    }

    FMT_CONSTEXPR ErrorHandler error_handler() const
    {
        return *this;
    }
};

typedef basic_parse_context<char> parse_context;
typedef basic_parse_context<wchar_t> wparse_context;

namespace internal {
// A map from argument names to their values for named arguments.
template<typename Context>
class arg_map
{
private:
    FMT_DISALLOW_COPY_AND_ASSIGN(arg_map);

    typedef typename Context::char_type char_type;

    struct entry
    {
        basic_string_view<char_type> name;
        basic_format_arg<Context> arg;
    };

    entry *map_;
    unsigned size_;

    void push_back(value<Context> val)
    {
        const internal::named_arg_base<char_type> &named = val.as_named_arg();
        map_[size_] = entry{named.name, named.template deserialize<Context>()};
        ++size_;
    }

public:
    arg_map()
        : map_(FMT_NULL)
        , size_(0)
    {
    }
    void init(const basic_format_args<Context> &args);
    ~arg_map()
    {
        delete[] map_;
    }

    basic_format_arg<Context> find(basic_string_view<char_type> name) const
    {
        // The list is unsorted, so just return the first matching name.
        for (entry *it = map_, *end = map_ + size_; it != end; ++it)
        {
            if (it->name == name)
                return it->arg;
        }
        return basic_format_arg<Context>();
    }
};

template<typename OutputIt, typename Context, typename Char>
class context_base
{
public:
    typedef OutputIt iterator;

private:
    basic_parse_context<Char> parse_context_;
    iterator out_;
    basic_format_args<Context> args_;

protected:
    typedef Char char_type;
    typedef basic_format_arg<Context> format_arg;

    context_base(OutputIt out, basic_string_view<char_type> format_str, basic_format_args<Context> ctx_args)
        : parse_context_(format_str)
        , out_(out)
        , args_(ctx_args)
    {
    }

    // Returns the argument with specified index.
    format_arg do_get_arg(unsigned arg_id)
    {
        format_arg arg = args_.get(arg_id);
        if (!arg)
            parse_context_.on_error("argument index out of range");
        return arg;
    }

    // Checks if manual indexing is used and returns the argument with
    // specified index.
    format_arg get_arg(unsigned arg_id)
    {
        return this->parse_context().check_arg_id(arg_id) ? this->do_get_arg(arg_id) : format_arg();
    }

public:
    basic_parse_context<char_type> &parse_context()
    {
        return parse_context_;
    }

    internal::error_handler error_handler()
    {
        return parse_context_.error_handler();
    }

    void on_error(const char *message)
    {
        parse_context_.on_error(message);
    }

    // Returns an iterator to the beginning of the output range.
    iterator out()
    {
        return out_;
    }
    iterator begin()
    {
        return out_;
    } // deprecated

    // Advances the begin iterator to ``it``.
    void advance_to(iterator it)
    {
        out_ = it;
    }

    basic_format_args<Context> args() const
    {
        return args_;
    }
};

// Extracts a reference to the container from back_insert_iterator.
template<typename Container>
inline Container &get_container(std::back_insert_iterator<Container> it)
{
    typedef std::back_insert_iterator<Container> bi_iterator;
    struct accessor : bi_iterator
    {
        accessor(bi_iterator iter)
            : bi_iterator(iter)
        {
        }
        using bi_iterator::container;
    };
    return *accessor(it).container;
}
} // namespace internal

// Formatting context.
template<typename OutputIt, typename Char>
class basic_format_context : public internal::context_base<OutputIt, basic_format_context<OutputIt, Char>, Char>
{
public:
    /** The character type for the output. */
    typedef Char char_type;

    // using formatter_type = formatter<T, char_type>;
    template<typename T>
    struct formatter_type
    {
        typedef formatter<T, char_type> type;
    };

private:
    internal::arg_map<basic_format_context> map_;

    FMT_DISALLOW_COPY_AND_ASSIGN(basic_format_context);

    typedef internal::context_base<OutputIt, basic_format_context, Char> base;
    typedef typename base::format_arg format_arg;
    using base::get_arg;

public:
    using typename base::iterator;

    /**
     Constructs a ``basic_format_context`` object. References to the arguments are
     stored in the object so make sure they have appropriate lifetimes.
     */
    basic_format_context(OutputIt out, basic_string_view<char_type> format_str, basic_format_args<basic_format_context> ctx_args)
        : base(out, format_str, ctx_args)
    {
    }

    format_arg next_arg()
    {
        return this->do_get_arg(this->parse_context().next_arg_id());
    }
    format_arg get_arg(unsigned arg_id)
    {
        return this->do_get_arg(arg_id);
    }

    // Checks if manual indexing is used and returns the argument with the
    // specified name.
    format_arg get_arg(basic_string_view<char_type> name);
};

template<typename Char>
struct buffer_context
{
    typedef basic_format_context<std::back_insert_iterator<internal::basic_buffer<Char>>, Char> type;
};
typedef buffer_context<char>::type format_context;
typedef buffer_context<wchar_t>::type wformat_context;

namespace internal {
template<typename Context, typename T>
struct get_type
{
    typedef decltype(make_value<Context>(declval<typename std::decay<T>::type &>())) value_type;
    static const type value = value_type::type_tag;
};

template<typename Context>
FMT_CONSTEXPR unsigned long long get_types()
{
    return 0;
}

template<typename Context, typename Arg, typename... Args>
FMT_CONSTEXPR unsigned long long get_types()
{
    return get_type<Context, Arg>::value | (get_types<Context, Args...>() << 4);
}

template<typename Context, typename T>
FMT_CONSTEXPR basic_format_arg<Context> make_arg(const T &value)
{
    basic_format_arg<Context> arg;
    arg.type_ = get_type<Context, T>::value;
    arg.value_ = make_value<Context>(value);
    return arg;
}

template<bool IS_PACKED, typename Context, typename T>
inline typename std::enable_if<IS_PACKED, value<Context>>::type make_arg(const T &value)
{
    return make_value<Context>(value);
}

template<bool IS_PACKED, typename Context, typename T>
inline typename std::enable_if<!IS_PACKED, basic_format_arg<Context>>::type make_arg(const T &value)
{
    return make_arg<Context>(value);
}
} // namespace internal

/**
  \rst
  An array of references to arguments. It can be implicitly converted into
  `~fmt::basic_format_args` for passing into type-erased formatting functions
  such as `~fmt::vformat`.
  \endrst
 */
template<typename Context, typename... Args>
class format_arg_store
{
private:
    static const size_t NUM_ARGS = sizeof...(Args);

    // Packed is a macro on MinGW so use IS_PACKED instead.
    static const bool IS_PACKED = NUM_ARGS < internal::max_packed_args;

    typedef typename std::conditional<IS_PACKED, internal::value<Context>, basic_format_arg<Context>>::type value_type;

    // If the arguments are not packed, add one more element to mark the end.
    static const size_t DATA_SIZE = NUM_ARGS + (IS_PACKED && NUM_ARGS != 0 ? 0 : 1);
    value_type data_[DATA_SIZE];

    friend class basic_format_args<Context>;

    static FMT_CONSTEXPR long long get_types()
    {
        return IS_PACKED ? static_cast<long long>(internal::get_types<Context, Args...>()) : -static_cast<long long>(NUM_ARGS);
    }

public:
#if FMT_USE_CONSTEXPR
    static constexpr long long TYPES = get_types();
#else
    static const long long TYPES;
#endif

#if (FMT_GCC_VERSION && FMT_GCC_VERSION <= 405) || (FMT_MSC_VER && FMT_MSC_VER <= 1800)
    // Workaround array initialization issues in gcc <= 4.5 and MSVC <= 2013.
    format_arg_store(const Args &... args)
    {
        value_type init[DATA_SIZE] = {internal::make_arg<IS_PACKED, Context>(args)...};
        std::memcpy(data_, init, sizeof(init));
    }
#else
    format_arg_store(const Args &... args)
        : data_{internal::make_arg<IS_PACKED, Context>(args)...}
    {
    }
#endif
};

#if !FMT_USE_CONSTEXPR
template<typename Context, typename... Args>
const long long format_arg_store<Context, Args...>::TYPES = get_types();
#endif

/**
  \rst
  Constructs an `~fmt::format_arg_store` object that contains references to
  arguments and can be implicitly converted to `~fmt::format_args`. `Context` can
  be omitted in which case it defaults to `~fmt::context`.
  \endrst
 */
template<typename Context, typename... Args>
inline format_arg_store<Context, Args...> make_format_args(const Args &... args)
{
    return format_arg_store<Context, Args...>(args...);
}

template<typename... Args>
inline format_arg_store<format_context, Args...> make_format_args(const Args &... args)
{
    return format_arg_store<format_context, Args...>(args...);
}

/** Formatting arguments. */
template<typename Context>
class basic_format_args
{
public:
    typedef unsigned size_type;
    typedef basic_format_arg<Context> format_arg;

private:
    // To reduce compiled code size per formatting function call, types of first
    // max_packed_args arguments are passed in the types_ field.
    unsigned long long types_;
    union
    {
        // If the number of arguments is less than max_packed_args, the argument
        // values are stored in values_, otherwise they are stored in args_.
        // This is done to reduce compiled code size as storing larger objects
        // may require more code (at least on x86-64) even if the same amount of
        // data is actually copied to stack. It saves ~10% on the bloat test.
        const internal::value<Context> *values_;
        const format_arg *args_;
    };

    typename internal::type type(unsigned index) const
    {
        unsigned shift = index * 4;
        unsigned long long mask = 0xf;
        return static_cast<typename internal::type>((types_ & (mask << shift)) >> shift);
    }

    friend class internal::arg_map<Context>;

    void set_data(const internal::value<Context> *values)
    {
        values_ = values;
    }
    void set_data(const format_arg *args)
    {
        args_ = args;
    }

    format_arg do_get(size_type index) const
    {
        long long signed_types = static_cast<long long>(types_);
        if (signed_types < 0)
        {
            unsigned long long num_args = static_cast<unsigned long long>(-signed_types);
            return index < num_args ? args_[index] : format_arg();
        }
        format_arg arg;
        if (index > internal::max_packed_args)
            return arg;
        arg.type_ = type(index);
        if (arg.type_ == internal::none_type)
            return arg;
        internal::value<Context> &val = arg.value_;
        val = values_[index];
        return arg;
    }

public:
    basic_format_args()
        : types_(0)
    {
    }

    /**
     \rst
     Constructs a `basic_format_args` object from `~fmt::format_arg_store`.
     \endrst
     */
    template<typename... Args>
    basic_format_args(const format_arg_store<Context, Args...> &store)
        : types_(static_cast<unsigned long long>(store.TYPES))
    {
        set_data(store.data_);
    }

    /** Returns the argument at specified index. */
    format_arg get(size_type index) const
    {
        format_arg arg = do_get(index);
        return arg.type_ == internal::named_arg_type ? arg.value_.as_named_arg().template deserialize<Context>() : arg;
    }

    unsigned max_size() const
    {
        long long signed_types = static_cast<long long>(types_);
        return static_cast<unsigned>(signed_types < 0 ? -signed_types : static_cast<long long>(internal::max_packed_args));
    }
};

/** An alias to ``basic_format_args<context>``. */
// It is a separate type rather than a typedef to make symbols readable.
struct format_args : basic_format_args<format_context>
{
    template<typename... Args>
    format_args(Args &&... arg)
        : basic_format_args<format_context>(std::forward<Args>(arg)...)
    {
    }
};
struct wformat_args : basic_format_args<wformat_context>
{
    template<typename... Args>
    wformat_args(Args &&... arg)
        : basic_format_args<wformat_context>(std::forward<Args>(arg)...)
    {
    }
};

namespace internal {
template<typename Char>
struct named_arg_base
{
    basic_string_view<Char> name;

    // Serialized value<context>.
    mutable char data[sizeof(basic_format_arg<format_context>)];

    named_arg_base(basic_string_view<Char> nm)
        : name(nm)
    {
    }

    template<typename Context>
    basic_format_arg<Context> deserialize() const
    {
        basic_format_arg<Context> arg;
        std::memcpy(&arg, data, sizeof(basic_format_arg<Context>));
        return arg;
    }
};

template<typename T, typename Char>
struct named_arg : named_arg_base<Char>
{
    const T &value;

    named_arg(basic_string_view<Char> name, const T &val)
        : named_arg_base<Char>(name)
        , value(val)
    {
    }
};
} // namespace internal

/**
  \rst
  Returns a named argument to be used in a formatting function.

  **Example**::

    fmt::print("Elapsed time: {s:.2f} seconds", fmt::arg("s", 1.23));
  \endrst
 */
template<typename T>
inline internal::named_arg<T, char> arg(string_view name, const T &arg)
{
    return internal::named_arg<T, char>(name, arg);
}

template<typename T>
inline internal::named_arg<T, wchar_t> arg(wstring_view name, const T &arg)
{
    return internal::named_arg<T, wchar_t>(name, arg);
}

// This function template is deleted intentionally to disable nested named
// arguments as in ``format("{}", arg("a", arg("b", 42)))``.
template<typename S, typename T, typename Char>
void arg(S, internal::named_arg<T, Char>) FMT_DELETED;

#ifndef FMT_EXTENDED_COLORS
// color and (v)print_colored are deprecated.
enum color
{
    black,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white
};
FMT_API void vprint_colored(color c, string_view format, format_args args);
FMT_API void vprint_colored(color c, wstring_view format, wformat_args args);
template<typename... Args>
inline void print_colored(color c, string_view format_str, const Args &... args)
{
    vprint_colored(c, format_str, make_format_args(args...));
}
template<typename... Args>
inline void print_colored(color c, wstring_view format_str, const Args &... args)
{
    vprint_colored(c, format_str, make_format_args<wformat_context>(args...));
}
#endif

format_context::iterator vformat_to(internal::buffer &buf, string_view format_str, format_args args);
wformat_context::iterator vformat_to(internal::wbuffer &buf, wstring_view format_str, wformat_args args);

template<typename Container>
struct is_contiguous : std::false_type
{
};

template<typename Char>
struct is_contiguous<std::basic_string<Char>> : std::true_type
{
};

template<typename Char>
struct is_contiguous<internal::basic_buffer<Char>> : std::true_type
{
};

/** Formats a string and writes the output to ``out``. */
template<typename Container>
typename std::enable_if<is_contiguous<Container>::value, std::back_insert_iterator<Container>>::type vformat_to(
    std::back_insert_iterator<Container> out, string_view format_str, format_args args)
{
    auto &container = internal::get_container(out);
    internal::container_buffer<Container> buf(container);
    vformat_to(buf, format_str, args);
    return std::back_inserter(container);
}

template<typename Container>
typename std::enable_if<is_contiguous<Container>::value, std::back_insert_iterator<Container>>::type vformat_to(
    std::back_insert_iterator<Container> out, wstring_view format_str, wformat_args args)
{
    auto &container = internal::get_container(out);
    internal::container_buffer<Container> buf(container);
    vformat_to(buf, format_str, args);
    return std::back_inserter(container);
}

std::string vformat(string_view format_str, format_args args);
std::wstring vformat(wstring_view format_str, wformat_args args);

/**
  \rst
  Formats arguments and returns the result as a string.

  **Example**::

    #include <fmt/core.h>
    std::string message = fmt::format("The answer is {}", 42);
  \endrst
*/
template<typename... Args>
inline std::string format(string_view format_str, const Args &... args)
{
    // This should be just
    // return vformat(format_str, make_format_args(args...));
    // but gcc has trouble optimizing the latter, so break it down.
    format_arg_store<format_context, Args...> as{args...};
    return vformat(format_str, as);
}
template<typename... Args>
inline std::wstring format(wstring_view format_str, const Args &... args)
{
    format_arg_store<wformat_context, Args...> as{args...};
    return vformat(format_str, as);
}

FMT_API void vprint(std::FILE *f, string_view format_str, format_args args);
FMT_API void vprint(std::FILE *f, wstring_view format_str, wformat_args args);

/**
  \rst
  Prints formatted data to the file *f*.

  **Example**::

    fmt::print(stderr, "Don't {}!", "panic");
  \endrst
 */
template<typename... Args>
inline void print(std::FILE *f, string_view format_str, const Args &... args)
{
    format_arg_store<format_context, Args...> as(args...);
    vprint(f, format_str, as);
}
/**
  Prints formatted data to the file *f* which should be in wide-oriented mode set
  via ``fwide(f, 1)`` or ``_setmode(_fileno(f), _O_U8TEXT)`` on Windows.
 */
template<typename... Args>
inline void print(std::FILE *f, wstring_view format_str, const Args &... args)
{
    format_arg_store<wformat_context, Args...> as(args...);
    vprint(f, format_str, as);
}

FMT_API void vprint(string_view format_str, format_args args);
FMT_API void vprint(wstring_view format_str, wformat_args args);

/**
  \rst
  Prints formatted data to ``stdout``.

  **Example**::

    fmt::print("Elapsed time: {0:.2f} seconds", 1.23);
  \endrst
 */
template<typename... Args>
inline void print(string_view format_str, const Args &... args)
{
    format_arg_store<format_context, Args...> as{args...};
    vprint(format_str, as);
}

template<typename... Args>
inline void print(wstring_view format_str, const Args &... args)
{
    format_arg_store<wformat_context, Args...> as(args...);
    vprint(format_str, as);
}
FMT_END_NAMESPACE

#endif // FMT_CORE_H_
