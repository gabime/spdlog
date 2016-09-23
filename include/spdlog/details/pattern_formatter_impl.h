//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <spdlog/formatter.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/details/os.h>
#include <spdlog/fmt/fmt.h>

#include <chrono>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace spdlog
{
namespace details
{
class flag_formatter
{
public:
    virtual ~flag_formatter()
    {}
    virtual void format(details::log_msg& msg, const std::tm& tm_time) = 0;
};

///////////////////////////////////////////////////////////////////////
// name & level pattern appenders
///////////////////////////////////////////////////////////////////////
namespace
{
class name_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        msg.formatted << *msg.logger_name;
    }
};
}

// log level appender
class level_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        msg.formatted << level::to_str(msg.level);
    }
};

// short log level appender
class short_level_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        msg.formatted << level::to_short_str(msg.level);
    }
};

///////////////////////////////////////////////////////////////////////
// Date time pattern appenders
///////////////////////////////////////////////////////////////////////

static const fmt_formatchar_t* ampm(const tm& t)
{
    return t.tm_hour >= 12 ? _SFS("PM") : _SFS("AM");
}

static int to12h(const tm& t)
{
    return t.tm_hour > 12 ? t.tm_hour - 12 : t.tm_hour;
}

//Abbreviated weekday name
static const fmt_formatstring_t days[] { _SFS("Sun"), _SFS("Mon"), _SFS("Tue"), _SFS("Wed"), _SFS("Thu"), _SFS("Fri"), _SFS("Sat") };
class a_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << days[tm_time.tm_wday];
    }
};

//Full weekday name
static const fmt_formatstring_t full_days[] { _SFS("Sunday"), _SFS("Monday"), _SFS("Tuesday"), _SFS("Wednesday"), _SFS("Thursday"), _SFS("Friday"), _SFS("Saturday") };
class A_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << full_days[tm_time.tm_wday];
    }
};

//Abbreviated month
static const std::string  months[]{ "Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sept", "Oct", "Nov", "Dec" };
class b_formatter :public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << months[tm_time.tm_mon];
    }
};

//Full month name
static const fmt_formatstring_t full_months[] { _SFS("January"), _SFS("February"), _SFS("March"), _SFS("April"), _SFS("May"), _SFS("June"), _SFS("July"), _SFS("August"), _SFS("September"), _SFS("October"), _SFS("November"), _SFS("December") };
class B_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << full_months[tm_time.tm_mon];
    }
};


//write 2 ints seperated by sep with padding of 2
static fmt_memory_writer_t& pad_n_join(fmt_memory_writer_t& w, int v1, int v2, fmt_formatchar_t sep)
{
    w << fmt::pad(v1, 2, _SFS('0')) << sep << fmt::pad(v2, 2, _SFS('0'));
    return w;
}

//write 3 ints seperated by sep with padding of 2
static fmt_memory_writer_t& pad_n_join(fmt_memory_writer_t& w, int v1, int v2, int v3, fmt_formatchar_t sep)
{
    w << fmt::pad(v1, 2, _SFS('0')) << sep << fmt::pad(v2, 2, _SFS('0')) << sep << fmt::pad(v3, 2, _SFS('0'));
    return w;
}


//Date and time representation (Thu Aug 23 15:35:46 2014)
class c_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << days[tm_time.tm_wday] << _SFS(' ') << months[tm_time.tm_mon] << _SFS(' ') << tm_time.tm_mday << _SFS(' ');
        pad_n_join(msg.formatted, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, _SFS(':')) << _SFS(' ') << tm_time.tm_year + 1900;
    }
};


// year - 2 digit
class C_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << fmt::pad(tm_time.tm_year % 100, 2, _SFS('0'));
    }
};



// Short MM/DD/YY date, equivalent to %m/%d/%y 08/23/01
class D_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        pad_n_join(msg.formatted, tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_year % 100, _SFS('/'));
    }
};


// year - 4 digit
class Y_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << tm_time.tm_year + 1900;
    }
};

// month 1-12
class m_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << fmt::pad(tm_time.tm_mon + 1, 2, _SFS('0'));
    }
};

// day of month 1-31
class d_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << fmt::pad(tm_time.tm_mday, 2, _SFS('0'));
    }
};

// hours in 24 format  0-23
class H_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << fmt::pad(tm_time.tm_hour, 2, _SFS('0'));
    }
};

// hours in 12 format  1-12
class I_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << fmt::pad(to12h(tm_time), 2, _SFS('0'));
    }
};

// minutes 0-59
class M_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << fmt::pad(tm_time.tm_min, 2, _SFS('0'));
    }
};

// seconds 0-59
class S_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << fmt::pad(tm_time.tm_sec, 2, _SFS('0'));
    }
};

// milliseconds
class e_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        auto duration = msg.time.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
        msg.formatted << fmt::pad(static_cast<int>(millis), 3, _SFS('0'));
    }
};

// microseconds
class f_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        auto duration = msg.time.time_since_epoch();
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000000;
        msg.formatted << fmt::pad(static_cast<int>(micros), 6, _SFS('0'));
    }
};

// nanoseconds
class F_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        auto duration = msg.time.time_since_epoch();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() % 1000000000;
        msg.formatted << fmt::pad(static_cast<int>(ns), 9, _SFS('0'));
    }
};

// AM/PM
class p_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        msg.formatted << ampm(tm_time);
    }
};


// 12 hour clock 02:55:02 pm
class r_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        pad_n_join(msg.formatted, to12h(tm_time), tm_time.tm_min, tm_time.tm_sec, _SFS(':')) << _SFS(' ') << ampm(tm_time);
    }
};

// 24-hour HH:MM time, equivalent to %H:%M
class R_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        pad_n_join(msg.formatted, tm_time.tm_hour, tm_time.tm_min, _SFS(':'));
    }
};

// ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S
class T_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
        pad_n_join(msg.formatted, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, _SFS(':'));
    }
};


// ISO 8601 offset from UTC in timezone (+-HH:MM)
class z_formatter:public flag_formatter
{
public:
    const std::chrono::seconds cache_refresh = std::chrono::seconds(5);

    z_formatter():_last_update(std::chrono::seconds(0))
    {}
    z_formatter(const z_formatter&) = delete;
    z_formatter& operator=(const z_formatter&) = delete;

    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
#ifdef _WIN32
        int total_minutes = get_cached_offset(msg, tm_time);
#else
        // No need to chache under gcc,
        // it is very fast (already stored in tm.tm_gmtoff)
        int total_minutes = os::utc_minutes_offset(tm_time);
#endif
        bool is_negative = total_minutes < 0;
        char sign;
        if (is_negative)
        {
            total_minutes = -total_minutes;
            sign = '-';
        }
        else
        {
            sign = '+';
        }

		int h = total_minutes / 60;
		int m = total_minutes % 60;        
		msg.formatted << sign;
		pad_n_join(msg.formatted, h, m, _SFS(':'));
    }
private:
    log_clock::time_point _last_update;
    int _offset_minutes;
    std::mutex _mutex;

    int get_cached_offset(const log_msg& msg, const std::tm& tm_time)
    {
        using namespace std::chrono;
        std::lock_guard<std::mutex> l(_mutex);
        if (msg.time - _last_update >= cache_refresh)
        {
            _offset_minutes = os::utc_minutes_offset(tm_time);
            _last_update = msg.time;
        }
        return _offset_minutes;
    }
};



//Thread id
class t_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
#ifdef SPDLOG_FMT_THREADID_HEX
		msg.formatted << fmt::pad(fmt::hex(msg.thread_id), 8, '0');
#else
        msg.formatted << fmt::pad(msg.thread_id,10, _SFS('0'));
#endif
    }
};


class v_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm&) override
    {
        msg.formatted << fmt_basicstringref_t(msg.raw.data(), msg.raw.size());
    }
};

class ch_formatter:public flag_formatter
{
public:
    explicit ch_formatter(fmt_formatchar_t ch): _ch(ch)
    {}
    void format(details::log_msg& msg, const std::tm&) override
    {
        msg.formatted << _ch;
    }
private:
	fmt_formatchar_t _ch;
};


//aggregate user chars to display as is
class aggregate_formatter:public flag_formatter
{
public:
    aggregate_formatter()
    {}
    void add_ch(fmt_formatchar_t ch)
    {
        _str += ch;
    }
    void format(details::log_msg& msg, const std::tm&) override
    {
        msg.formatted << _str;
    }
private:
	fmt_formatstring_t _str;
};

// Full info formatter
// pattern: [%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v
class full_formatter:public flag_formatter
{
    void format(details::log_msg& msg, const std::tm& tm_time) override
    {
#ifndef SPDLOG_NO_DATETIME
        auto duration = msg.time.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

        /* Slower version(while still very fast - about 3.2 million lines/sec under 10 threads),
        msg.formatted.write("[{:d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}] [{}] [{}] {} ",
        tm_time.tm_year + 1900,
        tm_time.tm_mon + 1,
        tm_time.tm_mday,
        tm_time.tm_hour,
        tm_time.tm_min,
        tm_time.tm_sec,
        static_cast<int>(millis),
        msg.logger_name,
        level::to_str(msg.level),
        msg.raw.str());*/


        // Faster (albeit uglier) way to format the line (5.6 million lines/sec under 10 threads)
        msg.formatted << '[' << static_cast<unsigned int>(tm_time.tm_year + 1900) << '-'
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_mon + 1), 2, _SFS('0')) << _SFS('-')
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_mday), 2, _SFS('0')) << _SFS(' ')
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_hour), 2, _SFS('0')) << _SFS(':')
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_min), 2, _SFS('0')) << _SFS(':')
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_sec), 2, _SFS('0')) << _SFS('.')
                      << fmt::pad(static_cast<unsigned int>(millis), 3, _SFS('0')) << _SFS("] ");

        //no datetime needed
#else
        (void)tm_time;
#endif

#ifndef SPDLOG_NO_NAME
        msg.formatted << _SFS('[') << *msg.logger_name << _SFS("] ");
#endif

        msg.formatted << _SFS('[') << level::to_str(msg.level) << _SFS("] ");
        msg.formatted << fmt_basicstringref_t(msg.raw.data(), msg.raw.size());
    }
};

}
}
///////////////////////////////////////////////////////////////////////////////
// pattern_formatter inline impl
///////////////////////////////////////////////////////////////////////////////
inline spdlog::pattern_formatter::pattern_formatter(const fmt_formatstring_t& pattern)
{
    compile_pattern(pattern);
}

inline void spdlog::pattern_formatter::compile_pattern(const fmt_formatstring_t& pattern)
{
    auto end = pattern.end();
    std::unique_ptr<details::aggregate_formatter> user_chars;
    for (auto it = pattern.begin(); it != end; ++it)
    {
        if (*it == _SFS('%'))
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
inline void spdlog::pattern_formatter::handle_flag(fmt_formatchar_t flag)
{
    switch (flag)
    {
    // logger name
    case _SFS('n'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::name_formatter()));
        break;

    case _SFS('l'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::level_formatter()));
        break;

    case _SFS('L'):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::short_level_formatter()));
        break;

    case(_SFS('t')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::t_formatter()));
        break;

    case(_SFS('v')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::v_formatter()));
        break;

    case(_SFS('a')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::a_formatter()));
        break;

    case(_SFS('A')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::A_formatter()));
        break;

    case(_SFS('b')):
    case(_SFS('h')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::b_formatter()));
        break;

    case(_SFS('B')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::B_formatter()));
        break;
    case(_SFS('c')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::c_formatter()));
        break;

    case(_SFS('C')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::C_formatter()));
        break;

    case(_SFS('Y')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::Y_formatter()));
        break;

    case(_SFS('D')):
    case(_SFS('x')):

        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::D_formatter()));
        break;

    case(_SFS('m')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::m_formatter()));
        break;

    case(_SFS('d')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::d_formatter()));
        break;

    case(_SFS('H')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::H_formatter()));
        break;

    case(_SFS('I')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::I_formatter()));
        break;

    case(_SFS('M')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::M_formatter()));
        break;

    case(_SFS('S')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::S_formatter()));
        break;

    case(_SFS('e')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::e_formatter()));
        break;

    case(_SFS('f')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::f_formatter()));
        break;
    case(_SFS('F')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::F_formatter()));
        break;

    case(_SFS('p')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::p_formatter()));
        break;

    case(_SFS('r')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::r_formatter()));
        break;

    case(_SFS('R')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::R_formatter()));
        break;

    case(_SFS('T')):
    case(_SFS('X')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::T_formatter()));
        break;

    case(_SFS('z')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::z_formatter()));
        break;

    case (_SFS('+')):
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::full_formatter()));
        break;

    default: //Unkown flag appears as is
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::ch_formatter(_SFS('%'))));
        _formatters.push_back(std::unique_ptr<details::flag_formatter>(new details::ch_formatter(flag)));
        break;
    }
}


inline void spdlog::pattern_formatter::format(details::log_msg& msg)
{

#ifndef SPDLOG_NO_DATETIME
    auto tm_time = details::os::localtime(log_clock::to_time_t(msg.time));
#else
    std::tm tm_time;
#endif
    for (auto &f : _formatters)
    {
        f->format(msg, tm_time);
    }
    //write eol
    msg.formatted.write(details::os::eol, details::os::eol_size);
}
