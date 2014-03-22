#pragma once

// Faster than ostringstream--returns its string by ref
#include <ostream>
#include "c11log/details/fast_buf.h"

namespace c11log
{
namespace details
{

class str_devicebuf:public std::streambuf
{
public:
	using Base = std::streambuf;
    str_devicebuf() = default;
    ~str_devicebuf() = default;

    str_devicebuf(const str_devicebuf& other) = delete;
    str_devicebuf(str_devicebuf&& other) = delete;
    str_devicebuf& operator=(const str_devicebuf&) = delete;
    str_devicebuf& operator=(str_devicebuf&&) = delete;

    bufpair_t buf()
    {
        return _fastbuf.get();
    }

    void clear()
    {
        _fastbuf.clear();
    }

protected:
    // copy the give buffer into the accumulated fast buffer
    std::streamsize xsputn(const char_type* s, std::streamsize count) override
    {
        _fastbuf.append(s, static_cast<unsigned int>(count));
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
    fast_buf<192> _fastbuf;
};

class fast_oss:public std::ostream
{
public:
    fast_oss():std::ostream(&_dev) {}
    ~fast_oss() = default;

    fast_oss(const fast_oss& other) = delete;
    fast_oss(fast_oss&& other) = delete;
    fast_oss& operator=(const fast_oss& other) = delete;

    bufpair_t buf()
    {
        return _dev.buf();
    }

    void clear()
    {
        _dev.clear();
    }

private:
    str_devicebuf _dev;
};
}
}
