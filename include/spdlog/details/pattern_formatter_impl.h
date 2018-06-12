//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/details/log_msg.h"
#include "spdlog/details/os.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/formatter.h"

#include <array>
#include <chrono>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {
inline void append_str(const std::string &str, fmt::memory_buffer &dest)
{
    const char *str_ptr = str.data();
    dest.append(str_ptr, str_ptr + str.size());
}

inline void append_buf(const fmt::memory_buffer &buf, fmt::memory_buffer &dest)
{
    const char *buf_ptr = buf.data();
    dest.append(buf_ptr, buf_ptr + buf.size());
}
} // namespace

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
        append_str(*msg.logger_name, msg.formatted);
    }
};

// log level appender
class level_formatter : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        append_str(level::to_str(msg.level), msg.formatted);
    }
};

// short log level appender
class short_level_formatter : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        append_str(level::to_short_str(msg.level), msg.formatted);
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
        append_str(days[tm_time.tm_wday], msg.formatted);
    }
};

// Full weekday name
static const std::string full_days[]{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
class A_formatter : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        append_str(full_days[tm_time.tm_wday], msg.formatted);
    }
};

// Abbreviated month
static const std::string months[]{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};
class b_formatter : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        append_str(months[tm_time.tm_mon], msg.formatted);
    }
};

// Full month name
static const std::string full_months[]{
    "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
class B_formatter : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        append_str(full_months[tm_time.tm_mon], msg.formatted);
    }
};

// Date and time representation (Thu Aug 23 15:35:46 2014)
class c_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{} {} {} {:02}:{:02}:{:02} {}", days[tm_time.tm_wday], months[tm_time.tm_mon], tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, tm_time.tm_year + 1900);
    }
};

// year - 2 digit
class C_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{:02}", tm_time.tm_year % 100);
    }
};

// Short MM/DD/YY date, equivalent to %m/%d/%y 08/23/01
class D_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{:02}/{:02}/{:02}", tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_year % 100);
    }
};

// year - 4 digit
class Y_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{}", tm_time.tm_year + 1900);
    }
};

// month 1-12
class m_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{:02}", tm_time.tm_mon + 1);
    }
};

// day of month 1-31
class d_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{:02}", tm_time.tm_mday);
    }
};

// hours in 24 format 0-23
class H_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{:02}", tm_time.tm_hour);
    }
};

// hours in 12 format 1-12
class I_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{:02}", to12h(tm_time));
    }
};

// minutes 0-59
class M_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{:02}", tm_time.tm_min);
    }
};

// seconds 0-59
class S_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{:02}", tm_time.tm_sec);
    }
};

// milliseconds
class e_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        auto duration = msg.time.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
        fmt::format_to(msg.formatted, "{:03}", static_cast<int>(millis));
    }
};

// microseconds
class f_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        auto duration = msg.time.time_since_epoch();
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000000;
        fmt::format_to(msg.formatted, "{:06}", static_cast<int>(micros));
    }
};

// nanoseconds
class F_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        auto duration = msg.time.time_since_epoch();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() % 1000000000;
        fmt::format_to(msg.formatted, "{:09}", static_cast<int>(ns));
    }
};

class E_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        auto duration = msg.time.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        fmt::format_to(msg.formatted, "{}", seconds);
    }
};

// AM/PM
class p_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{}", ampm(tm_time));
    }
};

// 12 hour clock 02:55:02 pm
class r_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{:02}:{:02}:{:02} {}", to12h(tm_time), tm_time.tm_min, tm_time.tm_sec, ampm(tm_time));
    }
};

// 24-hour HH:MM time, equivalent to %H:%M
class R_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{:02}:{:02}", tm_time.tm_hour, tm_time.tm_min);
    }
};

// ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S
class T_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &tm_time) override
    {
        fmt::format_to(msg.formatted, "{:02}:{:02}:{:02}", tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
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
        fmt::format_to(msg.formatted, "{}{:02}:{:02}", sign, h, m);
    }

private:
    log_clock::time_point last_update_{std::chrono::seconds(0)};
    int offset_minutes_{0};
    std::mutex mutex_;

    int get_cached_offset(const log_msg &msg, const std::tm &tm_time)
    {
        std::lock_guard<std::mutex> l(mutex_);
        if (msg.time - last_update_ >= cache_refresh)
        {
            offset_minutes_ = os::utc_minutes_offset(tm_time);
            last_update_ = msg.time;
        }
        return offset_minutes_;
    }
};

// Thread id
class t_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        fmt::format_to(msg.formatted, "{}", msg.thread_id);
    }
};

// Current pid
class pid_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        fmt::format_to(msg.formatted, "{}", details::os::pid());
    }
};

// message counter formatter
class i_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        fmt::format_to(msg.formatted, "{:06}", msg.msg_id);
    }
};

class v_formatter SPDLOG_FINAL : public flag_formatter
{
    void format(details::log_msg &msg, const std::tm &) override
    {
        append_buf(msg.raw, msg.formatted);
    }
};

class ch_formatter SPDLOG_FINAL : public flag_formatter
{
public:
    explicit ch_formatter(char ch)
        : ch_(ch)
    {
    }
    void format(details::log_msg &msg, const std::tm &) override
    {
        msg.formatted.push_back(ch_);
    }

private:
    char ch_;
};

// aggregate user chars to display as is
class aggregate_formatter SPDLOG_FINAL : public flag_formatter
{
public:
    aggregate_formatter() = default;

    void add_ch(char ch)
    {
        str_ += ch;
    }
    void format(details::log_msg &msg, const std::tm &) override
    {
        append_str(str_, msg.formatted);
    }

private:
    std::string str_;
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

        /*
        msg.formatted << '[' << static_cast<unsigned int>(tm_time.tm_year + 1900) << '-'
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_mon + 1), 2, '0') << '-'
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_mday), 2, '0') << ' '
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_hour), 2, '0') << ':'
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_min), 2, '0') << ':'
                      << fmt::pad(static_cast<unsigned int>(tm_time.tm_sec), 2, '0') << '.'
                      << fmt::pad(static_cast<unsigned int>(millis), 3, '0') << "] ";
        */

        fmt::format_to(msg.formatted, "[{}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}] ", tm_time.tm_year + 1900, tm_time.tm_mon + 1,
            tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, static_cast<int>(millis));

        // no datetime needed
#else
        (void)tm_time;
#endif

#ifndef SPDLOG_NO_NAME
        fmt::format_to(msg.formatted, "[{}] ", *msg.logger_name);
#endif

        msg.formatted.push_back('[');

        // wrap the level name with color
        msg.color_range_start = msg.formatted.size();
        append_str(level::to_str(msg.level), msg.formatted);
        msg.color_range_end = msg.formatted.size();
        msg.formatted.push_back(']');
        msg.formatted.push_back(' ');
        append_buf(msg.raw, msg.formatted);
    }
};

} // namespace details
} // namespace spdlog
///////////////////////////////////////////////////////////////////////////////
// pattern_formatter inline impl
///////////////////////////////////////////////////////////////////////////////
inline spdlog::pattern_formatter::pattern_formatter(const std::string &pattern, pattern_time_type pattern_time, std::string eol)
    : eol_(std::move(eol))
    , pattern_time_(pattern_time)
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
                formatters_.push_back(std::move(user_chars));
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
        formatters_.push_back(std::move(user_chars));
    }
}
inline void spdlog::pattern_formatter::handle_flag(char flag)
{
    switch (flag)
    {
    // logger name
    case 'n':
        formatters_.emplace_back(new details::name_formatter());
        break;

    case 'l':
        formatters_.emplace_back(new details::level_formatter());
        break;

    case 'L':
        formatters_.emplace_back(new details::short_level_formatter());
        break;

    case ('t'):
        formatters_.emplace_back(new details::t_formatter());
        break;

    case ('v'):
        formatters_.emplace_back(new details::v_formatter());
        break;

    case ('a'):
        formatters_.emplace_back(new details::a_formatter());
        break;

    case ('A'):
        formatters_.emplace_back(new details::A_formatter());
        break;

    case ('b'):
    case ('h'):
        formatters_.emplace_back(new details::b_formatter());
        break;

    case ('B'):
        formatters_.emplace_back(new details::B_formatter());
        break;
    case ('c'):
        formatters_.emplace_back(new details::c_formatter());
        break;

    case ('C'):
        formatters_.emplace_back(new details::C_formatter());
        break;

    case ('Y'):
        formatters_.emplace_back(new details::Y_formatter());
        break;

    case ('D'):
    case ('x'):

        formatters_.emplace_back(new details::D_formatter());
        break;

    case ('m'):
        formatters_.emplace_back(new details::m_formatter());
        break;

    case ('d'):
        formatters_.emplace_back(new details::d_formatter());
        break;

    case ('H'):
        formatters_.emplace_back(new details::H_formatter());
        break;

    case ('I'):
        formatters_.emplace_back(new details::I_formatter());
        break;

    case ('M'):
        formatters_.emplace_back(new details::M_formatter());
        break;

    case ('S'):
        formatters_.emplace_back(new details::S_formatter());
        break;

    case ('e'):
        formatters_.emplace_back(new details::e_formatter());
        break;

    case ('f'):
        formatters_.emplace_back(new details::f_formatter());
        break;
    case ('F'):
        formatters_.emplace_back(new details::F_formatter());
        break;

    case ('E'):
        formatters_.emplace_back(new details::E_formatter());
        break;

    case ('p'):
        formatters_.emplace_back(new details::p_formatter());
        break;

    case ('r'):
        formatters_.emplace_back(new details::r_formatter());
        break;

    case ('R'):
        formatters_.emplace_back(new details::R_formatter());
        break;

    case ('T'):
    case ('X'):
        formatters_.emplace_back(new details::T_formatter());
        break;

    case ('z'):
        formatters_.emplace_back(new details::z_formatter());
        break;

    case ('+'):
        formatters_.emplace_back(new details::full_formatter());
        break;

    case ('P'):
        formatters_.emplace_back(new details::pid_formatter());
        break;

    case ('i'):
        formatters_.emplace_back(new details::i_formatter());
        break;

    case ('^'):
        formatters_.emplace_back(new details::color_start_formatter());
        break;

    case ('$'):
        formatters_.emplace_back(new details::color_stop_formatter());
        break;

    default: // Unknown flag appears as is
        formatters_.emplace_back(new details::ch_formatter('%'));
        formatters_.emplace_back(new details::ch_formatter(flag));
        break;
    }
}

inline std::tm spdlog::pattern_formatter::get_time(details::log_msg &msg)
{
    if (pattern_time_ == pattern_time_type::local)
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
    for (auto &f : formatters_)
    {
        f->format(msg, tm_time);
    }
    // write eol
    append_str(eol_, msg.formatted);
}
