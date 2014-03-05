#pragma once

#include<streambuf>
#include<string>
namespace c11log {
namespace details {

class str_devicebuf:public std::streambuf {
public:
    str_devicebuf() = default;
    ~str_devicebuf() = default;
    str_devicebuf(const str_devicebuf& other):std::streambuf(),_str(other._str) {}
    str_devicebuf& operator=(const str_devicebuf other) {
        if(this != &other)
            _str = other._str;
        return *this;
    }

    const std::string& str_ref() const {
        return _str;
        std::ostringstream oss;
    }

    void clear() {
        _str.clear();
    }

protected:
    virtual int sync() override {
        return 0;
    }

    virtual std::streamsize xsputn(const char_type* s, std::streamsize count) override {
        _str.append(s, static_cast<unsigned int>(count));
        return count;
    }

    virtual int_type overflow(int_type ch) override {
        if (ch != traits_type::eof())
            _str.append((char*)&ch, 1);
        return 1;
    }
private:
    std::string _str;
};

class fast_oss:public std::ostream {
public:
    fast_oss():std::ostream(&_dev) {}
    ~fast_oss() = default;

    fast_oss(const fast_oss& other) :std::basic_ios<char>(), std::ostream(&_dev), _dev(other._dev) {}
    
    fast_oss& operator=(const fast_oss& other) {
        if(&other != this)
            _dev = other._dev;
        return *this;
    }

    const std::string& str_ref() const {
        return _dev.str_ref();
    }

    const std::string str() const {
        return _dev.str_ref();
    }

    void clear() {
        _dev.clear();
    }
private:
    str_devicebuf _dev;
};
}
}
