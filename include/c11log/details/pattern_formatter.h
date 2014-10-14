#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <vector>

#include "../formatter.h"
#include "log_msg.h"
#include "fast_oss.h"
#include "os.h"


namespace c11log
{
namespace details {
class pattern_compiler
{
public:
    virtual void append(const details::log_msg& msg, details::fast_oss& oss) = 0;
};

// log name appender
class name_compiler :public pattern_compiler
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss << msg.logger_name;
    }
};

// log level appender
class level_compiler :public pattern_compiler
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss << level::to_str(msg.level);
    }
};

///////////////////////////////////////////////////////////////////////
// Date time pattern appenders
///////////////////////////////////////////////////////////////////////

// year - 4 digit
class Y_compiler :public pattern_compiler
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_year + 1900, 4);
    }
};

// year - 2 digit
class y_compiler :public pattern_compiler
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_year, 2);
    }
};
// month 1-12
class m_compiler :public pattern_compiler
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_mon + 1, 2);
    }
};

// day of month 1-31
class d_compiler :public pattern_compiler
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_mday, 2);
    }
};

// hours in 24 format  0-23
class H_compiler :public pattern_compiler
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_hour, 2);
    }
};

// hours in 12 format  1-12
class I_compiler :public pattern_compiler
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int((msg.tm_time.tm_hour + 1) % 1, 2);
    }
};

// ninutes 0-59
class M_compiler :public pattern_compiler
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_min, 2);
    }
};

// seconds 0-59
class S_compiler :public pattern_compiler
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_sec, 2);
    }
};

// milliseconds
class e_compiler :public pattern_compiler
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        auto duration = msg.time.time_since_epoch();
        int millis = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000);
        oss.put_int(millis, 3);
    }
};


class t_compiler :public pattern_compiler
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_str(msg.raw);
    }
};

class ch_compiler :public pattern_compiler
{
public:
    explicit ch_compiler(char ch) : _ch(ch)
    {}
    void append(const details::log_msg&, details::fast_oss& oss) override
    {
        oss.putc(_ch);
    }
private:
    char _ch;
};


class str_compiler :public pattern_compiler
{
public:
    str_compiler()
    {}
    void add_ch(char ch)
    {
        _str += ch;
    }
    void append(const details::log_msg&, details::fast_oss& oss) override
    {
        oss << _str;
    }
private:
    std::string _str;
};



class pattern_formatter : public formatter
{

public:
    explicit pattern_formatter(const std::string& pattern);
    pattern_formatter(const pattern_formatter&) = delete;
    void format(details::log_msg& msg) override;
private:
    const std::string _pattern;
    std::vector<std::unique_ptr<details::pattern_compiler>> _compilers;
    void handle_flag(char flag);
    void compile_pattern(const std::string& pattern);
};
}
}


inline c11log::details::pattern_formatter::pattern_formatter(const std::string& pattern)
{
    compile_pattern(pattern);
}


inline void c11log::details::pattern_formatter::compile_pattern(const std::string& pattern)
{
    auto end = pattern.end();
    for (auto it = pattern.begin(); it != end; ++it)
    {
        if (*it == '%')
        {
            if (++it != end)
                handle_flag(*it);
            else
                return;
        }
        else
        {
            // chars not following the % sign should be displayed as is
            _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::ch_compiler(*it)));
        }
    }

}
inline void c11log::details::pattern_formatter::handle_flag(char flag)
{
    switch (flag)
    {
    // logger name
    case 'n':
        _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::name_compiler()));
        break;
    // message log level
    case 'l':
        _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::level_compiler()));
        break;
    // message text
    case('t') :
        _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::t_compiler()));
        break;
    // year
    case('Y') :
        _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::Y_compiler()));
        break;
    // year 2 digits
    case('y') :
        _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::y_compiler()));
        break;
    // month
    case('m') :
        // minute
        _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::m_compiler()));
        break;
    // day in month
    case('d') :
        _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::d_compiler()));
        break;
    // hour (24)
    case('H') :
        _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::H_compiler()));
        break;
    // hour (12)
    case('I') :
        _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::I_compiler()));
        break;
    // minutes
    case('M') :
        _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::M_compiler()));
        break;
    // seconds
    case('S') :
        _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::S_compiler()));
        break;
    // milliseconds part
    case('e'):
        _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::e_compiler()));
        break;
    // % sign
    case('%') :
        _compilers.push_back(std::unique_ptr<details::pattern_compiler>(new details::ch_compiler('%')));
        break;
    }
}


inline void c11log::details::pattern_formatter::format(details::log_msg& msg)
{
    details::fast_oss oss;
    for (auto &appender : _compilers)
    {
        appender->append(msg, oss);
    }
    oss.write(details::os::eol(), details::os::eol_size());
    msg.formatted = oss.str();
}
