#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <vector>

#include "formatter.h"
#include "details/log_msg.h"
#include "details/fast_oss.h"


namespace c11log
{
namespace details {
class pattern_appender
{
public:
    virtual void append(const details::log_msg& msg, details::fast_oss& oss) = 0;
};

// log name appender
class name_appender :public pattern_appender
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss << msg.logger_name;
    }
};

// log level appender
class level_appender :public pattern_appender
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
class Y_appender :public pattern_appender
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_year+1900, 4);
    }
};

// year - 2 digit
class y_appender :public pattern_appender
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_year, 2);
    }
};
// month 1-12
class m_appender :public pattern_appender
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_mon + 1, 2);
    }
};


// day of month 1-31
class d_appender :public pattern_appender
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_mday, 2);
    }
};

// hours in 24 format  0-23
class H_appender :public pattern_appender
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_hour, 2);
    }
};

// hours in 12 format  1-12
class I_appender :public pattern_appender
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int((msg.tm_time.tm_hour + 1) % 1, 2);
    }
};

// ninutes 0-59
class M_appender :public pattern_appender
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_min, 2);
    }
};

// seconds 0-59
class S_appender :public pattern_appender
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_int(msg.tm_time.tm_sec, 2);
    }
};

// milliseconds
class e_appender :public pattern_appender
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        auto duration = msg.time.time_since_epoch();
        int millis = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000);
        oss.put_int(millis, 3);
    }
};


class t_appender :public pattern_appender
{
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.put_str(msg.raw);
    }
};

class ch_appender :public pattern_appender
{
public:
    explicit ch_appender(char ch) : _ch(ch)
    {}
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss.putc( _ch);
    }
private:
    char _ch;
};


class str_appender :public pattern_appender
{
public:
    str_appender()
    {}
    void add_ch(char ch)
    {
        _str += ch;
    }
    void append(const details::log_msg& msg, details::fast_oss& oss) override
    {
        oss << _str;
    }
private:
    std::string _str;
};
}


class pattern_formatter : public formatter
{

public:
    explicit pattern_formatter(const std::string& pattern);
    pattern_formatter(const pattern_formatter&) = delete;
    void format(details::log_msg& msg) override;
private:
    const std::string _pattern;
    std::vector<std::unique_ptr<details::pattern_appender>> _appenders;
    void handle_flag(char flag);
    void compile_pattern(const std::string& pattern);
};
}



inline c11log::pattern_formatter::pattern_formatter(const std::string& pattern)
{
    compile_pattern(pattern);
}


inline void c11log::pattern_formatter::compile_pattern(const std::string& pattern)
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
            _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::ch_appender(*it)));
        }
    }

}
inline void c11log::pattern_formatter::handle_flag(char flag)
{
    switch (flag)
    {
    // logger name
    case 'n':
        _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::name_appender()));
        break;
    // message log level
    case 'l':
        _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::level_appender()));
        break;
    // message text
    case('t') :
        _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::t_appender()));
        break;
    // year
    case('Y') :
        _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::Y_appender()));
        break;
    // year 2 digits
    case('y') :
        _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::y_appender()));
        break;
    // month
    case('m') :
        // minute
        _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::m_appender()));
        break;
    // day in month
    case('d') :
        _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::d_appender()));
        break;
    // hour (24)
    case('H') :
        _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::H_appender()));
        break;
    // hour (12)
    case('I') :
        _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::I_appender()));
        break;
    // minutes
    case('M') :
        _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::M_appender()));
        break;
    // seconds
    case('S') :
        _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::S_appender()));
        break;
    // milliseconds part
    case('e'):
        _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::e_appender()));
        break;
    // % sign
    case('%') :
        _appenders.push_back(std::unique_ptr<details::pattern_appender>(new details::ch_appender('%')));
        break;
    }

}


inline void c11log::pattern_formatter::format(details::log_msg& msg)
{
    details::fast_oss oss;
    for (auto &appender : _appenders)
    {
        appender->append(msg, oss);
    }
    oss.write(details::os::eol(), details::os::eol_size());
    msg.formatted = oss.str();
}