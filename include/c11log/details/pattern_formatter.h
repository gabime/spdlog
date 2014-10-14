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
class flag_formatter
{
public:
    virtual void format(details::log_msg& msg) = 0;
};

// log name appender
class name_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted << msg.logger_name;
    }
};

// log level appender
class level_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted << level::to_str(msg.level);
    }
};

///////////////////////////////////////////////////////////////////////
// Date time pattern appenders
///////////////////////////////////////////////////////////////////////

// year - 4 digit
class Y_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.write_int(msg.tm_time.tm_year + 1900, 4);
    }
};

// year - 2 digit
class y_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.write_int(msg.tm_time.tm_year, 2);
    }
};
// month 1-12
class m_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.write_int(msg.tm_time.tm_mon + 1, 2);
    }
};

// day of month 1-31
class d_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.write_int(msg.tm_time.tm_mday, 2);
    }
};

// hours in 24 format  0-23
class H_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.write_int(msg.tm_time.tm_hour, 2);
    }
};

// hours in 12 format  1-12
class I_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.write_int((msg.tm_time.tm_hour + 1) % 1, 2);
    }
};

// ninutes 0-59
class M_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.write_int(msg.tm_time.tm_min, 2);
    }
};

// seconds 0-59
class S_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.write_int(msg.tm_time.tm_sec, 2);
    }
};

// milliseconds
class e_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        auto duration = msg.time.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
        msg.formatted.write_int(static_cast<int>(millis), 3);
    }
};


class t_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.write_fast_oss(msg.raw);
    }
};

class ch_formatter :public flag_formatter
{
public:
    explicit ch_formatter(char ch) : _ch(ch)
    {}
    void format(details::log_msg& msg) override
    {
        msg.formatted.putc(_ch);
    }
private:
    char _ch;
};


class str_formatter :public flag_formatter
{
public:
    str_formatter()
    {}
    void add_ch(char ch)
    {
        _str += ch;
    }
    void format(details::log_msg& msg) override
    {
        msg.formatted << _str;
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
    std::vector<std::unique_ptr<details::flag_formatter>> _formatters;
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
            _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::ch_formatter(*it)));
        }
    }

}
inline void c11log::details::pattern_formatter::handle_flag(char flag)
{
    switch (flag)
    {
    // logger name
    case 'n':
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::name_formatter()));
        break;
    // message log level
    case 'l':
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::level_formatter()));
        break;
    // message text
    case('t') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::t_formatter()));
        break;
    // year
    case('Y') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::Y_formatter()));
        break;
    // year 2 digits
    case('y') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::y_formatter()));
        break;
    // month
    case('m') :
        // minute
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::m_formatter()));
        break;
    // day in month
    case('d') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::d_formatter()));
        break;
    // hour (24)
    case('H') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::H_formatter()));
        break;
    // hour (12)
    case('I') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::I_formatter()));
        break;
    // minutes
    case('M') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::M_formatter()));
        break;
    // seconds
    case('S') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::S_formatter()));
        break;
    // milliseconds part
    case('e'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::e_formatter()));
        break;
    // % sign
    case('%') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::ch_formatter('%')));
        break;
    }
}


inline void c11log::details::pattern_formatter::format(details::log_msg& msg)
{
    for (auto &f : _formatters)
    {
        f->format(msg);
    }
    //write eol
    msg.formatted.write(details::os::eol(), details::os::eol_size());
}
