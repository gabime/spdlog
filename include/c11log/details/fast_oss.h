#pragma once

// A faster-than-ostringstream class
// uses stack_buf as the underlying buffer (upto 192 bytes before using the heap)

#include <ostream>
#include <iomanip>
#include "fast_istostr.h"
#include "stack_buf.h"
#include<iostream>

namespace c11log
{
namespace details
{

class stack_devicebuf :public std::streambuf
{
public:
    static const unsigned short stack_size = 256;
    using stackbuf_t = stack_buf<stack_size>;

    stack_devicebuf() = default;
    ~stack_devicebuf() = default;

    stack_devicebuf(const stack_devicebuf& other) :std::basic_streambuf<char>(), _stackbuf(other._stackbuf)
    {}

    stack_devicebuf(stack_devicebuf&& other):
        std::basic_streambuf<char>(),
        _stackbuf(std::move(other._stackbuf))
    {
        other.clear();
    }

    stack_devicebuf& operator=(stack_devicebuf other)
    {
        std::swap(_stackbuf, other._stackbuf);
        return *this;
    }

    const stackbuf_t& buf() const
    {
        return _stackbuf;
    }
    std::size_t size() const
    {
        return _stackbuf.size();
    }

    void clear()
    {
        _stackbuf.clear();
    }

protected:
    // copy the give buffer into the accumulated fast buffer
    std::streamsize xsputn(const char_type* s, std::streamsize count) override
    {
        _stackbuf.append(s, static_cast<unsigned int>(count));
        return count;
    }

    int_type overflow(int_type ch) override
    {
        if (traits_type::not_eof(ch))
        {
            char c = traits_type::to_char_type(ch);
            xsputn(&c, 1);
        }
        return ch;
    }
private:
    stackbuf_t _stackbuf;
};


class fast_oss :public std::ostream
{
public:
    fast_oss() :std::ostream(&_dev) {}
    ~fast_oss() = default;

    fast_oss(const fast_oss& other) :std::basic_ios<char>(), std::ostream(&_dev), _dev(other._dev)
    {}

    fast_oss(fast_oss&& other) :std::basic_ios<char>(), std::ostream(&_dev), _dev(std::move(other._dev))
    {
        other.clear();
    }


    fast_oss& operator=(fast_oss other)
    {
        swap(*this, other);
        return *this;
    }

    void swap(fast_oss& first, fast_oss& second) // nothrow
    {
        using std::swap;
        swap(first._dev, second._dev);
    }

    std::string str()
    {
        auto& buffer = _dev.buf();
        const char*data = buffer.data();
        return std::string(data, data+buffer.size());
    }

    const stack_devicebuf::stackbuf_t& buf() const
    {
        return _dev.buf();
    }


    std::size_t size() const
    {
        return _dev.size();
    }

    void clear()
    {
        _dev.clear();
    }

    //
    // The following were added because they significantly boost to perfromance
    //
    void putc(char c)
    {
        _dev.sputc(c);
    }

    // put int and pad with zeroes if smalled than min_width
    void put_int(int n, int padding)
    {
        std::string s;
        details::fast_itostr(n, s, padding);
        _dev.sputn(s.data(), s.size());
    }

    void put_data(const char* p, std::size_t data_size)
    {
        _dev.sputn(p, data_size);
    }

    void put_str(const std::string& s)
    {
        _dev.sputn(s.data(), s.size());
    }

    void put_fast_oss(const fast_oss& oss)
    {
        auto& buffer = oss.buf();
        _dev.sputn(buffer.data(), buffer.size());
    }


private:
    stack_devicebuf _dev;
};
}
}
