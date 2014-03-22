#pragma once

// Faster than ostringstream--returns its string by ref

#include "c11log/details/fast_buf.h"

namespace c11log
{
namespace details
{

class str_devicebuf:public std::streambuf
{
public:
    str_devicebuf() = default;
    ~str_devicebuf() = default;

    str_devicebuf(const str_devicebuf& other) = delete;
    str_devicebuf(str_devicebuf&& other) = delete;
    str_devicebuf& operator=(const str_devicebuf&) = delete;
    str_devicebuf& operator=(str_devicebuf&&) = delete;

    /*
    const std::string& str_ref() const
    {
        return _str;
    }
     */
    bufpair_t buf()
    {
        return _fastbuf.get();
    }

    void reset_str()
    {
        //_str.clear();
        _fastbuf.clear();
    }

protected:
    int sync() override
    {
        return 0;
    }

    // copy the give buffer into the accumulated string.
    // reserve initially 128 bytes which should be enough for common log lines
    std::streamsize xsputn(const char_type* s, std::streamsize count) override
    {
        /*
        if(_str.capacity() < k_initial_reserve)
        {
        	_str.reserve(k_initial_reserve);
        }
        _str.append(s, static_cast<unsigned int>(count));
         */
        _fastbuf.append(s, static_cast<unsigned int>(count));
        return count;
    }

    int_type overflow(int_type ch) override
    {

        bool not_eofile =  traits_type::not_eof(ch);
        if (not_eofile)
        {
            char c = traits_type::to_char_type(ch);
            xsputn(&c, 1);
        }
        return not_eofile;
    }
private:
    //std::string _str;
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
    /*
    const std::string& str_ref() const
    {
        return _dev.str_ref();
    }
     */
    bufpair_t buf()
    {
        return _dev.buf();
    }

    void reset_str()
    {
        _dev.reset_str();
    }

private:
    str_devicebuf _dev;
};
}
}
