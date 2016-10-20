/*
Formatting library for C++ - std::ostream support

Copyright (c) 2012 - 2016, Victor Zverovich
All rights reserved.

For the license information refer to format.h.
*/

#ifndef FMT_OSTREAM_H_
#define FMT_OSTREAM_H_

// commented out by spdlog
//#include "format.h"
#include <ostream>

namespace fmt
{

namespace internal
{

template <class Char>
class FormatBuf : public std::basic_streambuf<Char>
{
private:
    typedef typename std::basic_streambuf<Char>::int_type int_type;
    typedef typename std::basic_streambuf<Char>::traits_type traits_type;

    Buffer<Char> &buffer_;
    Char *start_;

public:
    FormatBuf(Buffer<Char> &buffer) : buffer_(buffer), start_(&buffer[0])
    {
        this->setp(start_, start_ + buffer_.capacity());
    }

    int_type overflow(int_type ch = traits_type::eof())
    {
        if (!traits_type::eq_int_type(ch, traits_type::eof()))
        {
            size_t buf_size = size();
            buffer_.resize(buf_size);
            buffer_.reserve(buf_size * 2);

            start_ = &buffer_[0];
            start_[buf_size] = traits_type::to_char_type(ch);
            this->setp(start_ + buf_size + 1, start_ + buf_size * 2);
        }
        return ch;
    }

    size_t size() const
    {
        return to_unsigned(this->pptr() - start_);
    }
};

Yes &convert(std::ostream &);

struct DummyStream : std::ostream
{
    DummyStream();  // Suppress a bogus warning in MSVC.
    // Hide all operator<< overloads from std::ostream.
    void operator<<(Null<>);
};

No &operator<<(std::ostream &, int);

template<typename T>
struct ConvertToIntImpl<T, true>
{
    // Convert to int only if T doesn't have an overloaded operator<<.
    enum
    {
        value = sizeof(convert(get<DummyStream>() << get<T>())) == sizeof(No)
    };
};

// Write the content of w to os.
void write(std::ostream &os, Writer &w);
}  // namespace internal

// Formats a value.
template <typename Char, typename ArgFormatter, typename T>
void format_arg(BasicFormatter<Char, ArgFormatter> &f,
                const Char *&format_str, const T &value)
{
    internal::MemoryBuffer<Char, internal::INLINE_BUFFER_SIZE> buffer;

    internal::FormatBuf<Char> format_buf(buffer);
    std::basic_ostream<Char> output(&format_buf);
    output << value;

    BasicStringRef<Char> str(&buffer[0], format_buf.size());
    typedef internal::MakeArg< BasicFormatter<Char> > MakeArg;
    format_str = f.format(format_str, MakeArg(str));
}

/**
\rst
Prints formatted data to the stream *os*.

**Example**::

print(cerr, "Don't {}!", "panic");
\endrst
*/
FMT_API void print(std::ostream &os, CStringRef format_str, ArgList args);
FMT_VARIADIC(void, print, std::ostream &, CStringRef)
}  // namespace fmt

#ifdef FMT_HEADER_ONLY
# include "ostream.cc"
#endif

#endif  // FMT_OSTREAM_H_