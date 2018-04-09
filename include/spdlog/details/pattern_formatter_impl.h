//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "../details/log_msg.h"
#include "../details/os.h"
#include "../fmt/fmt.h"
#include "../formatter.h"

#include <array>
#include <chrono>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace spdlog {
namespace details {
class flag_formatter
{
public:
    virtual ~flag_formatter() = default;
    virtual void format(details::log_msg &msg, const std::tm &tm_time) = 0;
};

///////////////////////////////////////////////////////////////////////
// name & level pattern appenders
///////////////////////////////////////////////////////////////////////
class name_formatter : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        msg.formatted << *msg.logger_name;
    }
};

// log level appender
class level_formatter : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        msg.formatted << level::to_str(msg.level);
    }
};

// short log level appender
class short_level_formatter : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        msg.formatted << level::to_short_str(msg.level);
    }
};

///////////////////////////////////////////////////////////////////////
// Date time pattern appenders
///////////////////////////////////////////////////////////////////////

static const char *ampm(const tm &t)
{
    return t.tm_hour >= 12 ? "PM" : "AM";
}

static int to12h(const tm &t)
{
    return t.tm_hour > 12 ? t.tm_hour - 12 : t.tm_hour;
}

// Abbreviated weekday name
static const std::string days[]{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
class a_formatter : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << days[tm_time.tm_wday];
    }
};

// Full weekday name
static const std::string full_days[]{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
class A_formatter : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << full_days[tm_time.tm_wday];
    }
};

// Abbreviated month
static const std::string months[]{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};
class b_formatter : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << months[tm_time.tm_mon];
    }
};

// Full month name
static const std::string full_months[]{
    "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
class B_formatter : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << full_months[tm_time.tm_mon];
    }
};

// write 2 ints separated by sep with padding of 2
static fmt::MemoryWriter &pad_n_join(fmt::MemoryWriter &w, int v1, int v2, char sep)
{
    w << fmt::pad(v1, 2, '0') << sep << fmt::pad(v2, 2, '0');
    return w;
}

// write 3 ints separated by sep with padding of 2
static fmt::MemoryWriter &pad_n_join(fmt::MemoryWriter &w, int v1, int v2, int v3, char sep)
{
    w << fmt::pad(v1, 2, '0') << sep << fmt::pad(v2, 2, '0') << sep << fmt::pad(v3, 2, '0');
    return w;
}

// Date and time representation (Thu Aug 23 15:35:46 2014)
class c_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << days[tm_time.tm_wday] << ' ' << months[tm_time.tm_mon] << ' ' << tm_time.tm_mday << ' ';
        pad_n_join(msg.formatted, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, ':') << ' ' << tm_time.tm_year + 1900;
    }
};

// year - 2 digit
class C_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << fmt::pad(tm_time.tm_year % 100, 2, '0');
    }
};

// Short MM/DD/YY date, equivalent to %m/%d/%y 08/23/01
class D_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        pad_n_join(msg.formatted, tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_year % 100, '/');
    }
};

// year - 4 digit
class Y_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << tm_time.tm_year + 1900;
    }
};

// month 1-12
class m_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << fmt::pad(tm_time.tm_mon + 1, 2, '0');
    }
};

// day of month 1-31
class d_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << fmt::pad(tm_time.tm_mday, 2, '0');
    }
};

// hours in 24 format 0-23
class H_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << fmt::pad(tm_time.tm_hour, 2, '0');
    }
};

// hours in 12 format 1-12
class I_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << fmt::pad(to12h(tm_time), 2, '0');
    }
};

// minutes 0-59
class M_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << fmt::pad(tm_time.tm_min, 2, '0');
    }
};

// seconds 0-59
class S_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << fmt::pad(tm_time.tm_sec, 2, '0');
    }
};

// milliseconds
class e_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        auto duration = msg.time.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
        msg.formatted << fmt::pad(static_cast<int>(millis), 3, '0');
    }
};

// microseconds
class f_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        auto duration = msg.time.time_since_epoch();
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000000;
        msg.formatted << fmt::pad(static_cast<int>(micros), 6, '0');
    }
};

// nanoseconds
class F_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        auto duration = msg.time.time_since_epoch();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() % 1000000000;
        msg.formatted << fmt::pad(static_cast<int>(ns), 9, '0');
    }
};

class E_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        auto duration = msg.time.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        msg.formatted << seconds;
    }
};

// AM/PM
class p_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        msg.formatted << ampm(tm_time);
    }
};

// 12 hour clock 02:55:02 pm
class r_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        pad_n_join(msg.formatted, to12h(tm_time), tm_time.tm_min, tm_time.tm_sec, ':') << ' ' << ampm(tm_time);
    }
};

// 24-hour HH:MM time, equivalent to %H:%M
class R_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        pad_n_join(msg.formatted, tm_time.tm_hour, tm_time.tm_min, ':');
    }
};

// ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S
class T_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        pad_n_join(msg.formatted, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, ':');
    }
};

// ISO 8601 offset from UTC in timezone (+-HH:MM)
class z_formatter SPDLOG_FINAL : public flag_formatter
{
public:
    const std::chrono::seconds cache_refresh = std::chrono::seconds(5);

    z_formatter() = default;
    z_formatter(const z_formatter &) = delete;
    z_formatter &operator=(const z_formatter &) = delete;

    void format(details::log_msg &msg, const std::tm &tm_time) override
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
        pad_n_join(msg.formatted, h, m, ':');
    }

private:
    log_clock::time_point _last_update{std::chrono::seconds(0)};
    int _offset_minutes{0};
    std::mutex _mutex;

    int get_cached_offset(const log_msg &msg, const std::tm &tm_time)
    {
        std::lock_guard<std::mutex> l(_mutex);
        if (msg.time - _last_update >= cache_refresh)
        {
            _offset_minutes = os::utc_minutes_offset(tm_time);
            _last_update = msg.time;
        }
        return _offset_minutes;
    }
};

// Thread id
class t_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        msg.formatted << msg.thread_id;
    }
};

// Current pid
class pid_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        msg.formatted << details::os::pid();
    }
};

// message counter formatter
class i_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        msg.formatted << fmt::pad(msg.msg_id, 6, '0');
    }
};

class v_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        msg.formatted << fmt::StringRef(msg.raw.data(), msg.raw.size());
    }
};

class ch_formatter SPDLOG_FINAL : public flag_formatter
{
public:
    explicit ch_formatter(char ch)
        : _ch(ch)
    {
    }
    void format(details::log_msg &msg, const std::tm &) override
    {
        msg.formatted << _ch;
    }

private:
    char _ch;
};

// aggregate user chars to display as is
class aggregate_formatter SPDLOG_FINAL : public flag_formatter
{
public:
    aggregate_formatter() = default;

    void add_ch(char ch)
    {
        _str += ch;
    }
    void format(details::log_msg &msg, const std::tm &) override
    {
        msg.formatted << _str;
    }

private:
    std::string _str;
};

// mark the color range. expect it to be in the form of "%^colored text%$"
class color_start_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        msg.color_range_start = msg.formatted.size();
    }
};
class color_stop_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        msg.color_range_end = msg.formatted.size();
    }
};

// Full info formatter
// pattern: [%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v
class full_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
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
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_mon + 1), 2, '0') << '-'
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_mday), 2, '0') << ' '
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_hour), 2, '0') << ':'
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_min), 2, '0') << ':'
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_sec), 2, '0') << '.'
                      << fmt::pad(static_cast<unsigned int>(millis), 3, '0') << "] ";

        // no datetime needed
#else
        (void)tm_time;
#endif

#ifndef SPDLOG_NO_NAME
        msg.formatted << '[' << *msg.logger_name << "] ";
#endif

        msg.formatted << '[';
        // wrap the level name with color
        msg.color_range_start = msg.formatted.size();
        msg.formatted << level::to_str(msg.level);
        msg.color_range_end = msg.formatted.size();
        msg.formatted << "] " << fmt::StringRef(msg.raw.data(), msg.raw.size());
    }
};

} // namespace details
} // namespace spdlog
///////////////////////////////////////////////////////////////////////////////
// pattern_formatter inline impl
///////////////////////////////////////////////////////////////////////////////
inline spdlog::pattern_formatter::pattern_formatter(const std::string &pattern, pattern_time_type pattern_time, std::string eol)
    : _eol(std::move(eol))
    , _pattern_time(pattern_time)
{
    compile_pattern(pattern);
}

inline void spdlog::pattern_formatter::compile_pattern(const std::string &pattern)
{
    auto end = pattern.end();
    std::unique_ptr<details::aggregate_formatter> user_chars;
    for (auto it = pattern.begin(); it != end; ++it)
    {
        if (*it == '%')
        {
            if (user_chars) // append user chars found so far
            {
                _formatters.push_back(std::move(user_chars));
            }
            // if(
            if (++it != end)
            {
                handle_flag(*it);
            }
            else
            {
                break;
            }
        }
        else // chars not following the % sign should be displayed as is
        {
            if (!user_chars)
            {
                user_chars = std::unique_ptr<details::aggregate_formatter>(new details::aggregate_formatter());
            }
            user_chars->add_ch(*it);
        }
    }
    if (user_chars) // append raw chars found so far
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
        _formatters.emplace_back(new details::name_formatter());
        break;

    case 'l':
        _formatters.emplace_back(new details::level_formatter());
        break;

    case 'L':
        _formatters.emplace_back(new details::short_level_formatter());
        break;

    case ('t'):
        _formatters.emplace_back(new details::t_formatter());
        break;

    case ('v'):
        _formatters.emplace_back(new details::v_formatter());
        break;

    case ('a'):
        _formatters.emplace_back(new details::a_formatter());
        break;

    case ('A'):
        _formatters.emplace_back(new details::A_formatter());
        break;

    case ('b'):
    case ('h'):
        _formatters.emplace_back(new details::b_formatter());
        break;

    case ('B'):
        _formatters.emplace_back(new details::B_formatter());
        break;
    case ('c'):
        _formatters.emplace_back(new details::c_formatter());
        break;

    case ('C'):
        _formatters.emplace_back(new details::C_formatter());
        break;

    case ('Y'):
        _formatters.emplace_back(new details::Y_formatter());
        break;

    case ('D'):
    case ('x'):

        _formatters.emplace_back(new details::D_formatter());
        break;

    case ('m'):
        _formatters.emplace_back(new details::m_formatter());
        break;

    case ('d'):
        _formatters.emplace_back(new details::d_formatter());
        break;

    case ('H'):
        _formatters.emplace_back(new details::H_formatter());
        break;

    case ('I'):
        _formatters.emplace_back(new details::I_formatter());
        break;

    case ('M'):
        _formatters.emplace_back(new details::M_formatter());
        break;

    case ('S'):
        _formatters.emplace_back(new details::S_formatter());
        break;

    case ('e'):
        _formatters.emplace_back(new details::e_formatter());
        break;

    case ('f'):
        _formatters.emplace_back(new details::f_formatter());
        break;
    case ('F'):
        _formatters.emplace_back(new details::F_formatter());
        break;

    case ('E'):
        _formatters.emplace_back(new details::E_formatter());
        break;

    case ('p'):
        _formatters.emplace_back(new details::p_formatter());
        break;

    case ('r'):
        _formatters.emplace_back(new details::r_formatter());
        break;

    case ('R'):
        _formatters.emplace_back(new details::R_formatter());
        break;

    case ('T'):
    case ('X'):
        _formatters.emplace_back(new details::T_formatter());
        break;

    case ('z'):
        _formatters.emplace_back(new details::z_formatter());
        break;

    case ('+'):
        _formatters.emplace_back(new details::full_formatter());
        break;

    case ('P'):
        _formatters.emplace_back(new details::pid_formatter());
        break;

    case ('i'):
        _formatters.emplace_back(new details::i_formatter());
        break;

    case ('^'):
        _formatters.emplace_back(new details::color_start_formatter());
        break;

    case ('$'):
        _formatters.emplace_back(new details::color_stop_formatter());
        break;

    default: // Unknown flag appears as is
        _formatters.emplace_back(new details::ch_formatter('%'));
        _formatters.emplace_back(new details::ch_formatter(flag));
        break;
    }
}

inline std::tm spdlog::pattern_formatter::get_time(details::log_msg &msg)
{
    if (_pattern_time == pattern_time_type::local)
    {
        return details::os::localtime(log_clock::to_time_t(msg.time));
    }
    return details::os::gmtime(log_clock::to_time_t(msg.time));
}

inline void spdlog::pattern_formatter::format(details::log_msg &msg)
{

#ifndef SPDLOG_NO_DATETIME
    auto tm_time = get_time(msg);
#else
    std::tm tm_time;
#endif
    for (auto &f : _formatters)
    {
        f->format(msg, tm_time);
    }
    // write eol
    msg.formatted << _eol;
}
