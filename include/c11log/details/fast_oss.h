#pragma once

// Fast ostringstream like supprt which return its string by ref and nothing more

#include<streambuf>
#include<string>

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


    const std::string& str_ref() const
    {
        return _str;
    }

    void reset_str()
    {
        _str.clear();
    }

protected:
    virtual int sync() override
    {
        return 0;

    }

    virtual std::streamsize xsputn(const char_type* s, std::streamsize count) override
    {
		auto ssize = _str.size();
		auto cap_left = _str.capacity() - ssize;
		if(cap_left < static_cast<std::size_t>(count))
			_str.reserve(ssize + count + 128);

        _str.append(s, static_cast<unsigned int>(count));
        return count;
    }

    virtual int_type overflow(int_type ch) override
    {
        if (ch != traits_type::eof())
            _str.append((char*)&ch, 1);
        return 1;
    }
private:
    std::string _str;
};

class fast_oss:public std::ostream
{
public:
    fast_oss():std::ostream(&_dev) {}
    ~fast_oss() = default;

    fast_oss(const fast_oss& other) = delete;
    fast_oss(fast_oss&& other) = delete;
    fast_oss& operator=(const fast_oss& other) = delete;

    const std::string& str_ref() const
    {
        return _dev.str_ref();
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
