#pragma once

#include<streambuf>
#include<string>
namespace c11log {
namespace details {

class str_devicebuf:public std::streambuf {
public:
    str_devicebuf()
    {
        _str.reserve(128);
    }

    const std::string& str_ref()
    {
        return _str;
    }

    void clear()
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

class fast_oss:public std::ostream {
public:
    fast_oss():std::ostream(&_dev)
    {}
    ~fast_oss()
    {}

    const std::string& str_ref() const
    {
        auto mydevice = static_cast<str_devicebuf*>(rdbuf());
        return mydevice->str_ref();
    }

    const std::string str() const
    {
        auto mydevice = static_cast<str_devicebuf*>(rdbuf());
        return mydevice->str_ref();
    }

    void clear()
    {
        auto mydevice = static_cast<str_devicebuf*>(rdbuf());
        mydevice->clear();
    }
private:
    str_devicebuf _dev;

};
}
}