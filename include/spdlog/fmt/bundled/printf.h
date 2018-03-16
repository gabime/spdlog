/*
 Formatting library for C++

 Copyright (c) 2012 - 2016, Victor Zverovich
 All rights reserved.

 For the license information refer to format.h.
 */

#ifndef FMT_PRINTF_H_
#define FMT_PRINTF_H_

#include <algorithm> // std::fill_n
#include <limits>    // std::numeric_limits

#include "ostream.h"

namespace fmt {
namespace internal {

// Checks if a value fits in int - used to avoid warnings about comparing
// signed and unsigned integers.
template<bool IsSigned>
struct IntChecker
{
    template<typename T>
    static bool fits_in_int(T value)
    {
        unsigned max = std::numeric_limits<int>::max();
        return value <= max;
    }
    static bool fits_in_int(bool)
    {
        return true;
    }
};

template<>
struct IntChecker<true>
{
    template<typename T>
    static bool fits_in_int(T value)
    {
        return value >= std::numeric_limits<int>::min() && value <= std::numeric_limits<int>::max();
    }
    static bool fits_in_int(int)
    {
        return true;
    }
};

class PrecisionHandler : public ArgVisitor<PrecisionHandler, int>
{
public:
    void report_unhandled_arg()
    {
        FMT_THROW(FormatError("precision is not integer"));
    }

    template<typename T>
    int visit_any_int(T value)
    {
        if (!IntChecker<std::numeric_limits<T>::is_signed>::fits_in_int(value))
            FMT_THROW(FormatError("number is too big"));
        return static_cast<int>(value);
    }
};

// IsZeroInt::visit(arg) returns true iff arg is a zero integer.
class IsZeroInt : public ArgVisitor<IsZeroInt, bool>
{
public:
    template<typename T>
    bool visit_any_int(T value)
    {
        return value == 0;
    }
};

// returns the default type for format specific "%s"
class DefaultType : public ArgVisitor<DefaultType, char>
{
public:
    char visit_char(int)
    {
        return 'c';
    }

    char visit_bool(bool)
    {
        return 's';
    }

    char visit_pointer(const void *)
    {
        return 'p';
    }

    template<typename T>
    char visit_any_int(T)
    {
        return 'd';
    }

    template<typename T>
    char visit_any_double(T)
    {
        return 'g';
    }

    char visit_unhandled_arg()
    {
        return 's';
    }
};

template<typename T, typename U>
struct is_same
{
    enum
    {
        value = 0
    };
};

template<typename T>
struct is_same<T, T>
{
    enum
    {
        value = 1
    };
};

// An argument visitor that converts an integer argument to T for printf,
// if T is an integral type. If T is void, the argument is converted to
// corresponding signed or unsigned type depending on the type specifier:
// 'd' and 'i' - signed, other - unsigned)
template<typename T = void>
class ArgConverter : public ArgVisitor<ArgConverter<T>, void>
{
private:
    internal::Arg &arg_;
    wchar_t type_;

    FMT_DISALLOW_COPY_AND_ASSIGN(ArgConverter);

public:
    ArgConverter(internal::Arg &arg, wchar_t type)
        : arg_(arg)
        , type_(type)
    {
    }

    void visit_bool(bool value)
    {
        if (type_ != 's')
            visit_any_int(value);
    }

    void visit_char(int value)
    {
        if (type_ != 's')
            visit_any_int(value);
    }

    template<typename U>
    void visit_any_int(U value)
    {
        bool is_signed = type_ == 'd' || type_ == 'i';
        if (type_ == 's')
        {
            is_signed = std::numeric_limits<U>::is_signed;
        }

        using internal::Arg;
        typedef typename internal::Conditional<is_same<T, void>::value, U, T>::type TargetType;
        if (const_check(sizeof(TargetType) <= sizeof(int)))
        {
            // Extra casts are used to silence warnings.
            if (is_signed)
            {
                arg_.type = Arg::INT;
                arg_.int_value = static_cast<int>(static_cast<TargetType>(value));
            }
            else
            {
                arg_.type = Arg::UINT;
                typedef typename internal::MakeUnsigned<TargetType>::Type Unsigned;
                arg_.uint_value = static_cast<unsigned>(static_cast<Unsigned>(value));
            }
        }
        else
        {
            if (is_signed)
            {
                arg_.type = Arg::LONG_LONG;
                // glibc's printf doesn't sign extend arguments of smaller types:
                //   std::printf("%lld", -42);  // prints "4294967254"
                // but we don't have to do the same because it's a UB.
                arg_.long_long_value = static_cast<LongLong>(value);
            }
            else
            {
                arg_.type = Arg::ULONG_LONG;
                arg_.ulong_long_value = static_cast<typename internal::MakeUnsigned<U>::Type>(value);
            }
        }
    }
};

// Converts an integer argument to char for printf.
class CharConverter : public ArgVisitor<CharConverter, void>
{
private:
    internal::Arg &arg_;

    FMT_DISALLOW_COPY_AND_ASSIGN(CharConverter);

public:
    explicit CharConverter(internal::Arg &arg)
        : arg_(arg)
    {
    }

    template<typename T>
    void visit_any_int(T value)
    {
        arg_.type = internal::Arg::CHAR;
        arg_.int_value = static_cast<char>(value);
    }
};

// Checks if an argument is a valid printf width specifier and sets
// left alignment if it is negative.
class WidthHandler : public ArgVisitor<WidthHandler, unsigned>
{
private:
    FormatSpec &spec_;

    FMT_DISALLOW_COPY_AND_ASSIGN(WidthHandler);

public:
    explicit WidthHandler(FormatSpec &spec)
        : spec_(spec)
    {
    }

    void report_unhandled_arg()
    {
        FMT_THROW(FormatError("width is not integer"));
    }

    template<typename T>
    unsigned visit_any_int(T value)
    {
        typedef typename internal::IntTraits<T>::MainType UnsignedType;
        UnsignedType width = static_cast<UnsignedType>(value);
        if (internal::is_negative(value))
        {
            spec_.align_ = ALIGN_LEFT;
            width = 0 - width;
        }
        unsigned int_max = std::numeric_limits<int>::max();
        if (width > int_max)
            FMT_THROW(FormatError("number is too big"));
        return static_cast<unsigned>(width);
    }
};
} // namespace internal

/**
  \rst
  A ``printf`` argument formatter based on the `curiously recurring template
  pattern <http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern>`_.

  To use `~fmt::BasicPrintfArgFormatter` define a subclass that implements some
  or all of the visit methods with the same signatures as the methods in
  `~fmt::ArgVisitor`, for example, `~fmt::ArgVisitor::visit_int()`.
  Pass the subclass as the *Impl* template parameter. When a formatting
  function processes an argument, it will dispatch to a visit method
  specific to the argument type. For example, if the argument type is
  ``double`` then the `~fmt::ArgVisitor::visit_double()` method of a subclass
  will be called. If the subclass doesn't contain a method with this signature,
  then a corresponding method of `~fmt::BasicPrintfArgFormatter` or its
  superclass will be called.
  \endrst
 */
template<typename Impl, typename Char, typename Spec>
class BasicPrintfArgFormatter : public internal::ArgFormatterBase<Impl, Char, Spec>
{
private:
    void write_null_pointer()
    {
        this->spec().type_ = 0;
        this->write("(nil)");
    }

    typedef internal::ArgFormatterBase<Impl, Char, Spec> Base;

public:
    /**
      \rst
      Constructs an argument formatter object.
      *writer* is a reference to the output writer and *spec* contains format
      specifier information for standard argument types.
      \endrst
     */
    BasicPrintfArgFormatter(BasicWriter<Char> &w, Spec &s)
        : internal::ArgFormatterBase<Impl, Char, Spec>(w, s)
    {
    }

    /** Formats an argument of type ``bool``. */
    void visit_bool(bool value)
    {
        Spec &fmt_spec = this->spec();
        if (fmt_spec.type_ != 's')
            return this->visit_any_int(value);
        fmt_spec.type_ = 0;
        this->write(value);
    }

    /** Formats a character. */
    void visit_char(int value)
    {
        const Spec &fmt_spec = this->spec();
        BasicWriter<Char> &w = this->writer();
        if (fmt_spec.type_ && fmt_spec.type_ != 'c')
            w.write_int(value, fmt_spec);
        typedef typename BasicWriter<Char>::CharPtr CharPtr;
        CharPtr out = CharPtr();
        if (fmt_spec.width_ > 1)
        {
            Char fill = ' ';
            out = w.grow_buffer(fmt_spec.width_);
            if (fmt_spec.align_ != ALIGN_LEFT)
            {
                std::fill_n(out, fmt_spec.width_ - 1, fill);
                out += fmt_spec.width_ - 1;
            }
            else
            {
                std::fill_n(out + 1, fmt_spec.width_ - 1, fill);
            }
        }
        else
        {
            out = w.grow_buffer(1);
        }
        *out = static_cast<Char>(value);
    }

    /** Formats a null-terminated C string. */
    void visit_cstring(const char *value)
    {
        if (value)
            Base::visit_cstring(value);
        else if (this->spec().type_ == 'p')
            write_null_pointer();
        else
            this->write("(null)");
    }

    /** Formats a pointer. */
    void visit_pointer(const void *value)
    {
        if (value)
            return Base::visit_pointer(value);
        this->spec().type_ = 0;
        write_null_pointer();
    }

    /** Formats an argument of a custom (user-defined) type. */
    void visit_custom(internal::Arg::CustomValue c)
    {
        BasicFormatter<Char> formatter(ArgList(), this->writer());
        const Char format_str[] = {'}', 0};
        const Char *format = format_str;
        c.format(&formatter, c.value, &format);
    }
};

/** The default printf argument formatter. */
template<typename Char>
class PrintfArgFormatter : public BasicPrintfArgFormatter<PrintfArgFormatter<Char>, Char, FormatSpec>
{
public:
    /** Constructs an argument formatter object. */
    PrintfArgFormatter(BasicWriter<Char> &w, FormatSpec &s)
        : BasicPrintfArgFormatter<PrintfArgFormatter<Char>, Char, FormatSpec>(w, s)
    {
    }
};

/** This template formats data and writes the output to a writer. */
template<typename Char, typename ArgFormatter = PrintfArgFormatter<Char>>
class PrintfFormatter : private internal::FormatterBase
{
private:
    BasicWriter<Char> &writer_;

    void parse_flags(FormatSpec &spec, const Char *&s);

    // Returns the argument with specified index or, if arg_index is equal
    // to the maximum unsigned value, the next argument.
    internal::Arg get_arg(const Char *s, unsigned arg_index = (std::numeric_limits<unsigned>::max)());

    // Parses argument index, flags and width and returns the argument index.
    unsigned parse_header(const Char *&s, FormatSpec &spec);

public:
    /**
     \rst
     Constructs a ``PrintfFormatter`` object. References to the arguments and
     the writer are stored in the formatter object so make sure they have
     appropriate lifetimes.
     \endrst
     */
    explicit PrintfFormatter(const ArgList &al, BasicWriter<Char> &w)
        : FormatterBase(al)
        , writer_(w)
    {
    }

    /** Formats stored arguments and writes the output to the writer. */
    void format(BasicCStringRef<Char> format_str);
};

template<typename Char, typename AF>
void PrintfFormatter<Char, AF>::parse_flags(FormatSpec &spec, const Char *&s)
{
    for (;;)
    {
        switch (*s++)
        {
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

template<typename Char, typename AF>
internal::Arg PrintfFormatter<Char, AF>::get_arg(const Char *s, unsigned arg_index)
{
    (void)s;
    const char *error = FMT_NULL;
    internal::Arg arg = arg_index == std::numeric_limits<unsigned>::max() ? next_arg(error) : FormatterBase::get_arg(arg_index - 1, error);
    if (error)
        FMT_THROW(FormatError(!*s ? "invalid format string" : error));
    return arg;
}

template<typename Char, typename AF>
unsigned PrintfFormatter<Char, AF>::parse_header(const Char *&s, FormatSpec &spec)
{
    unsigned arg_index = std::numeric_limits<unsigned>::max();
    Char c = *s;
    if (c >= '0' && c <= '9')
    {
        // Parse an argument index (if followed by '$') or a width possibly
        // preceded with '0' flag(s).
        unsigned value = internal::parse_nonnegative_int(s);
        if (*s == '$') // value is an argument index
        {
            ++s;
            arg_index = value;
        }
        else
        {
            if (c == '0')
                spec.fill_ = '0';
            if (value != 0)
            {
                // Nonzero value means that we parsed width and don't need to
                // parse it or flags again, so return now.
                spec.width_ = value;
                return arg_index;
            }
        }
    }
    parse_flags(spec, s);
    // Parse width.
    if (*s >= '0' && *s <= '9')
    {
        spec.width_ = internal::parse_nonnegative_int(s);
    }
    else if (*s == '*')
    {
        ++s;
        spec.width_ = internal::WidthHandler(spec).visit(get_arg(s));
    }
    return arg_index;
}

template<typename Char, typename AF>
void PrintfFormatter<Char, AF>::format(BasicCStringRef<Char> format_str)
{
    const Char *start = format_str.c_str();
    const Char *s = start;
    while (*s)
    {
        Char c = *s++;
        if (c != '%')
            continue;
        if (*s == c)
        {
            write(writer_, start, s);
            start = ++s;
            continue;
        }
        write(writer_, start, s - 1);

        FormatSpec spec;
        spec.align_ = ALIGN_RIGHT;

        // Parse argument index, flags and width.
        unsigned arg_index = parse_header(s, spec);

        // Parse precision.
        if (*s == '.')
        {
            ++s;
            if ('0' <= *s && *s <= '9')
            {
                spec.precision_ = static_cast<int>(internal::parse_nonnegative_int(s));
            }
            else if (*s == '*')
            {
                ++s;
                spec.precision_ = internal::PrecisionHandler().visit(get_arg(s));
            }
            else
            {
                spec.precision_ = 0;
            }
        }

        using internal::Arg;
        Arg arg = get_arg(s, arg_index);
        if (spec.flag(HASH_FLAG) && internal::IsZeroInt().visit(arg))
            spec.flags_ &= ~internal::to_unsigned<int>(HASH_FLAG);
        if (spec.fill_ == '0')
        {
            if (arg.type <= Arg::LAST_NUMERIC_TYPE)
                spec.align_ = ALIGN_NUMERIC;
            else
                spec.fill_ = ' '; // Ignore '0' flag for non-numeric types.
        }

        // Parse length and convert the argument to the required type.
        using internal::ArgConverter;
        switch (*s++)
        {
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
            ArgConverter<std::size_t>(arg, *s).visit(arg);
            break;
        case 't':
            ArgConverter<std::ptrdiff_t>(arg, *s).visit(arg);
            break;
        case 'L':
            // printf produces garbage when 'L' is omitted for long double, no
            // need to do the same.
            break;
        default:
            --s;
            ArgConverter<void>(arg, *s).visit(arg);
        }

        // Parse type.
        if (!*s)
            FMT_THROW(FormatError("invalid format string"));
        spec.type_ = static_cast<char>(*s++);

        if (spec.type_ == 's')
        {
            // set the format type to the default if 's' is specified
            spec.type_ = internal::DefaultType().visit(arg);
        }

        if (arg.type <= Arg::LAST_INTEGER_TYPE)
        {
            // Normalize type.
            switch (spec.type_)
            {
            case 'i':
            case 'u':
                spec.type_ = 'd';
                break;
            case 'c':
                // TODO: handle wchar_t
                internal::CharConverter(arg).visit(arg);
                break;
            }
        }

        start = s;

        // Format argument.
        AF(writer_, spec).visit(arg);
    }
    write(writer_, start, s);
}

inline void printf(Writer &w, CStringRef format, ArgList args)
{
    PrintfFormatter<char>(args, w).format(format);
}
FMT_VARIADIC(void, printf, Writer &, CStringRef)

inline void printf(WWriter &w, WCStringRef format, ArgList args)
{
    PrintfFormatter<wchar_t>(args, w).format(format);
}
FMT_VARIADIC(void, printf, WWriter &, WCStringRef)

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
FMT_VARIADIC(std::string, sprintf, CStringRef)

inline std::wstring sprintf(WCStringRef format, ArgList args)
{
    WMemoryWriter w;
    printf(w, format, args);
    return w.str();
}
FMT_VARIADIC_W(std::wstring, sprintf, WCStringRef)

/**
  \rst
  Prints formatted data to the file *f*.

  **Example**::

    fmt::fprintf(stderr, "Don't %s!", "panic");
  \endrst
 */
FMT_API int fprintf(std::FILE *f, CStringRef format, ArgList args);
FMT_VARIADIC(int, fprintf, std::FILE *, CStringRef)

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
FMT_VARIADIC(int, printf, CStringRef)

/**
  \rst
  Prints formatted data to the stream *os*.

  **Example**::

    fprintf(cerr, "Don't %s!", "panic");
  \endrst
 */
inline int fprintf(std::ostream &os, CStringRef format_str, ArgList args)
{
    MemoryWriter w;
    printf(w, format_str, args);
    internal::write(os, w);
    return static_cast<int>(w.size());
}
FMT_VARIADIC(int, fprintf, std::ostream &, CStringRef)
} // namespace fmt

#ifdef FMT_HEADER_ONLY
#include "printf.cc"
#endif

#endif // FMT_PRINTF_H_
