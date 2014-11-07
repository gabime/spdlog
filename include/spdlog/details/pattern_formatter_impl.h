/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2014 Gabi Melman.                                       */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <thread>

#include "../formatter.h"
#include "./log_msg.h"
#include "./fast_oss.h"
#include "./os.h"

namespace spdlog
{
namespace details
{
class flag_formatter
{
public:
    virtual ~flag_formatter() {}
    virtual void format(details::log_msg& msg) = 0;
};

///////////////////////////////////////////////////////////////////////
// name & level pattern appenders
///////////////////////////////////////////////////////////////////////
namespace
{
class name_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted << msg.logger_name;
    }
};
}

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

static const char* ampm(const tm& t)
{
    return t.tm_hour >= 12 ? "PM" : "AM";
}

static int to12h(const tm& t)
{
    return t.tm_hour > 12 ? t.tm_hour - 12 : t.tm_hour;
}

//Abbreviated weekday name
static const std::string days[] { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
class a_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_str(days[msg.tm_time.tm_wday]);
    }
};

//Full weekday name
static const std::string full_days[] { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
class A_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_str(full_days[msg.tm_time.tm_wday]);
    }
};

//Abbreviated month
static const std::string  months[] { "Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sept", "Oct", "Nov", "Dec" };
class b_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_str(months[msg.tm_time.tm_mon]);
    }
};

//Full month name
static const std::string full_months[] { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
class B_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_str(full_months[msg.tm_time.tm_mon]);
    }
};

//Date and time representation (Thu Aug 23 15:35:46 2014)
class c_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_str(days[msg.tm_time.tm_wday]);
        msg.formatted.putc(' ');
        msg.formatted.put_str(months[msg.tm_time.tm_mon]);
        msg.formatted.putc(' ');
        msg.formatted.put_int(msg.tm_time.tm_mday, 2);
        msg.formatted.putc(' ');
        msg.formatted.put_int(msg.tm_time.tm_hour, 2);
        msg.formatted.putc(':');
        msg.formatted.put_int(msg.tm_time.tm_min, 2);
        msg.formatted.putc(':');
        msg.formatted.put_int(msg.tm_time.tm_sec, 2);
    }
};


// year - 2 digit
class C_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_int(msg.tm_time.tm_year % 100, 2);
    }
};



// Short MM/DD/YY date, equivalent to %m/%d/%y 08/23/01
class D_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_int(msg.tm_time.tm_mon + 1, 2);
        msg.formatted.putc('/');
        msg.formatted.put_int(msg.tm_time.tm_mday, 2);
        msg.formatted.putc('/');
        msg.formatted.put_int(msg.tm_time.tm_year % 100, 2);
    }
};


// year - 4 digit
class Y_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_int(msg.tm_time.tm_year + 1900, 4);
    }
};

// month 1-12
class m_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_int(msg.tm_time.tm_mon + 1, 2);
    }
};

// day of month 1-31
class d_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_int(msg.tm_time.tm_mday, 2);
    }
};

// hours in 24 format  0-23
class H_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_int(msg.tm_time.tm_hour, 2);
    }
};

// hours in 12 format  1-12
class I_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_int(to12h(msg.tm_time), 2);
    }
};

// ninutes 0-59
class M_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_int(msg.tm_time.tm_min, 2);
    }
};

// seconds 0-59
class S_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_int(msg.tm_time.tm_sec, 2);
    }
};

// milliseconds
class e_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        auto duration = msg.time.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
        msg.formatted.put_int(static_cast<int>(millis), 3);
    }
};

// AM/PM
class p_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_data(ampm(msg.tm_time), 2);
    }
};


// 12 hour clock 02:55:02 pm
class r_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_int(to12h(msg.tm_time), 2);
        msg.formatted.putc(':');
        msg.formatted.put_int(msg.tm_time.tm_min, 2);
        msg.formatted.putc(':');
        msg.formatted.put_int(msg.tm_time.tm_sec, 2);
        msg.formatted.putc(' ');
        msg.formatted.put_data(ampm(msg.tm_time), 2);
    }
};

// 24-hour HH:MM time, equivalent to %H:%M
class R_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_int(msg.tm_time.tm_hour, 2);
        msg.formatted.putc(':');
        msg.formatted.put_int(msg.tm_time.tm_min, 2);

    }
};

// ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S
class T_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_int(msg.tm_time.tm_hour, 2);
        msg.formatted.putc(':');
        msg.formatted.put_int(msg.tm_time.tm_min, 2);
        msg.formatted.putc(':');
        msg.formatted.put_int(msg.tm_time.tm_sec, 2);
    }
};

// ISO 8601 offset from UTC in timezone (HH:MM)
class z_formatter :public flag_formatter
{
public:
    z_formatter() {}
    z_formatter(const z_formatter&) = delete;
    z_formatter& operator=(const z_formatter&) = delete;

    void format(log_msg& msg) override
    {
        std::lock_guard<std::mutex> l(_mutex);
        using namespace std::chrono;
        auto diff = msg.time - _last_update;
        auto secs_diff = std::abs((duration_cast<seconds>(diff)).count());
        if (secs_diff >= 2)
        {
            _value = get_value(msg);
            _last_update = msg.time;
        }
        msg.formatted.put_str(_value);
    }
private:
    log_clock::time_point _last_update;
    std::string _value;
    std::mutex _mutex;

    std::string get_value(const log_msg& msg)
    {
        int total_minutes = os::utc_minutes_offset(msg.tm_time);
        int h = total_minutes / 60;
        int m = total_minutes % 60;
        fast_oss oss;
        oss.putc(h < 0 ? '-' : '+');
        oss.put_int(h, 2);
        oss.putc(':');
        oss.put_int(m, 2);
        return oss.str();
    }

};


//Thread id
class t_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted << std::this_thread::get_id();
    }
};


class v_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_fast_oss(msg.raw);
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


//aggregate user chars to display as is
class aggregate_formatter :public flag_formatter
{
public:
    aggregate_formatter()
    {}
    void add_ch(char ch)
    {
        _str += ch;
    }
    void format(details::log_msg& msg) override
    {
        msg.formatted.put_str(_str);
    }
private:
    std::string _str;
};

// Full info formatter
// pattern: [%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v
class full_formatter :public flag_formatter
{
    void format(details::log_msg& msg) override
    {
        msg.formatted.putc('[');
        msg.formatted.put_int(msg.tm_time.tm_year+1900, 4);
        msg.formatted.putc('-');
        msg.formatted.put_int(msg.tm_time.tm_mon+ 1, 2);
        msg.formatted.putc('-');
        msg.formatted.put_int(msg.tm_time.tm_mday, 2);
        msg.formatted.putc(' ');
        msg.formatted.put_int(msg.tm_time.tm_hour, 2);
        msg.formatted.putc(':');
        msg.formatted.put_int(msg.tm_time.tm_min, 2);
        msg.formatted.putc(':');
        msg.formatted.put_int(msg.tm_time.tm_sec, 2);
        //millis
        msg.formatted.putc('.');
        auto duration = msg.time.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
        msg.formatted.put_int(static_cast<int>(millis), 3);
        msg.formatted.putc(']');
        msg.formatted << " [" << msg.logger_name << "] [" << level::to_str(msg.level) << "] ";
        msg.formatted.put_fast_oss(msg.raw);

    }
};

}
}
///////////////////////////////////////////////////////////////////////////////
// pattern_formatter inline impl
///////////////////////////////////////////////////////////////////////////////
inline spdlog::pattern_formatter::pattern_formatter(const std::string& pattern)
{
    compile_pattern(pattern);
}

inline void spdlog::pattern_formatter::compile_pattern(const std::string& pattern)
{
    auto end = pattern.end();
    std::unique_ptr<details::aggregate_formatter> user_chars;
    for (auto it = pattern.begin(); it != end; ++it)
    {
        if (*it == '%')
        {
            if (user_chars) //append user chars found so far
                _formatters.push_back(std::move(user_chars));

            if (++it != end)
                handle_flag(*it);
            else
                break;
        }
        else // chars not following the % sign should be displayed as is
        {
            if (!user_chars)
                user_chars = std::unique_ptr<details::aggregate_formatter>(new details::aggregate_formatter());
            user_chars->add_ch(*it);
        }
    }
    if (user_chars) //append raw chars found so far
    {
        _formatters.push_back(std::move(user_chars));
    }

}
inline void spdlog::pattern_formatter::handle_flag(char flag)
{
    switch (flag)
    {
        // logger name
    case 'n':
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::name_formatter()));
        break;

    case 'l':
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::level_formatter()));
        break;

    case('t') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::t_formatter()));
        break;

    case('v') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::v_formatter()));
        break;

    case('a') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::a_formatter()));
        break;

    case('A') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::A_formatter()));
        break;

    case('b') :
    case('h') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::b_formatter()));
        break;

    case('B') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::B_formatter()));
        break;
    case('c') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::c_formatter()));
        break;

    case('C') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::C_formatter()));
        break;

    case('Y') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::Y_formatter()));
        break;

    case('D') :
    case('x') :

        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::D_formatter()));
        break;

    case('m') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::m_formatter()));
        break;

    case('d') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::d_formatter()));
        break;

    case('H') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::H_formatter()));
        break;

    case('I') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::I_formatter()));
        break;

    case('M') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::M_formatter()));
        break;

    case('S') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::S_formatter()));
        break;

    case('e') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::e_formatter()));
        break;

    case('p') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::p_formatter()));
        break;

    case('r') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::r_formatter()));
        break;

    case('R') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::R_formatter()));
        break;

    case('T') :
    case('X') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::T_formatter()));
        break;

    case('z') :
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::z_formatter()));
        break;

    case ('+'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::full_formatter()));
        break;

    default: //Unkown flag appears as is
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::ch_formatter('%')));
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::ch_formatter(flag)));
        break;
    }
}


inline void spdlog::pattern_formatter::format(details::log_msg& msg)
{
    for (auto &f : _formatters)
    {
        f->format(msg);
    }
    //write eol
    msg.formatted.write(details::os::eol(), details::os::eol_size());
}
