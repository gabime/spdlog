#pragma once
#include <sstream>
#include <iostream>

namespace c11log {
class log_exception :public std::exception {
public:
    log_exception() : _oss(), _msg() {
    }

    virtual ~log_exception() {
    }

    explicit log_exception(const std::string& msg) :_oss(msg, std::ostringstream::ate), _msg(msg) {
    }

    log_exception(const log_exception &other) :_oss(other._oss.str()), _msg(other._msg) {
    }

    log_exception& operator=(const log_exception& other) {
        _oss.str(other._oss.str());
        _msg = other._msg;
        return *this;
    }

    virtual const char* what() const throw () override {
        return _msg.c_str();
    }

    template<typename T>
    log_exception& operator<<(const T& what) {
        _oss << what;
        _msg = _oss.str();
        return *this;
    }

private:
    std::ostringstream _oss;
    std::string _msg;
};
}