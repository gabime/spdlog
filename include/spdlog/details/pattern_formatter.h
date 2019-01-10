//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/details/fmt_helper.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/details/os.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/formatter.h"

#include <array>
#include <chrono>
#include <ctime>
#include <cctype>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace spdlog {
namespace details {

// padding information.
struct padding_info
{
    enum pad_side
    {
        left,
        right,
        center
    };

    padding_info() = default;
    padding_info(size_t width, padding_info::pad_side side)
        : width_(width)
        , side_(side)
    {
    }

    bool enabled() const
    {
        return width_ != 0;
    }
    const size_t width_ = 0;
    const pad_side side_ = left;
};

class scoped_pad
{
public:
    scoped_pad(size_t wrapped_size, padding_info &padinfo, fmt::memory_buffer &dest)
        : padinfo_(padinfo)
        , dest_(dest)
    {

        if (padinfo_.width_ <= wrapped_size)
        {
            total_pad_ = 0;
            return;
        }

        total_pad_ = padinfo.width_ - wrapped_size;
        if (padinfo_.side_ == padding_info::left)
        {
            pad_it(total_pad_);
            total_pad_ = 0;
        }
        else if (padinfo_.side_ == padding_info::center)
        {
            auto half_pad = total_pad_ / 2;
            auto reminder = total_pad_ & 1;
            pad_it(half_pad);
            total_pad_ = half_pad + reminder; // for the right side
        }
    }

    scoped_pad(spdlog::string_view_t txt, padding_info &padinfo, fmt::memory_buffer &dest)
        : scoped_pad(txt.size(), padinfo, dest)
    {
    }

    ~scoped_pad()
    {
        if (total_pad_)
        {
            pad_it(total_pad_);
        }
    }

private:
    void pad_it(size_t count)
    {
        // count = std::min(count, spaces_.size());
        assert(count <= spaces_.size());
        fmt_helper::append_string_view(string_view_t(spaces_.data(), count), dest_);
    }

    const padding_info &padinfo_;
    fmt::memory_buffer &dest_;
    size_t total_pad_;
    string_view_t spaces_{"                                                                "
                          "                                                                ",
        128};
};

class flag_formatter
{
public:
    explicit flag_formatter(padding_info padinfo)
        : padinfo_(padinfo)
    {
    }
    flag_formatter() = default;
    virtual ~flag_formatter() = default;
    virtual void format(const details::log_msg &msg, const std::tm &tm_time, fmt::memory_buffer &dest) = 0;

protected:
    padding_info padinfo_;
};

///////////////////////////////////////////////////////////////////////
// name & level pattern appender
///////////////////////////////////////////////////////////////////////
class name_formatter : public flag_formatter
{
public:
    explicit name_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        if (padinfo_.enabled())
        {
            scoped_pad p(*msg.logger_name, padinfo_, dest);
            fmt_helper::append_string_view(*msg.logger_name, dest);
        }
        else
        {
            fmt_helper::append_string_view(*msg.logger_name, dest);
        }
    }
};

// log level appender
class level_formatter : public flag_formatter
{
public:
    explicit level_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        string_view_t &level_name = level::to_string_view(msg.level);
        if (padinfo_.enabled())
        {
            scoped_pad p(level_name, padinfo_, dest);
            fmt_helper::append_string_view(level_name, dest);
        }
        else
        {
            fmt_helper::append_string_view(level_name, dest);
        }
    }
};

// short log level appender
class short_level_formatter : public flag_formatter
{
public:
    explicit short_level_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        string_view_t level_name{level::to_short_c_str(msg.level)};
        scoped_pad p(level_name, padinfo_, dest);
        fmt_helper::append_string_view(level_name, dest);
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
static const char *days[]{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
class a_formatter : public flag_formatter
{
public:
    explicit a_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        string_view_t field_value{days[tm_time.tm_wday]};
        scoped_pad p(field_value, padinfo_, dest);
        fmt_helper::append_string_view(field_value, dest);
    }
};

// Full weekday name
static const char *full_days[]{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
class A_formatter : public flag_formatter
{
public:
    explicit A_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        string_view_t field_value{full_days[tm_time.tm_wday]};
        scoped_pad p(field_value, padinfo_, dest);
        fmt_helper::append_string_view(field_value, dest);
    }
};

// Abbreviated month
static const char *months[]{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};
class b_formatter : public flag_formatter
{
public:
    explicit b_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        string_view_t field_value{months[tm_time.tm_mon]};
        scoped_pad p(field_value, padinfo_, dest);
        fmt_helper::append_string_view(field_value, dest);
    }
};

// Full month name
static const char *full_months[]{
    "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
class B_formatter : public flag_formatter
{
public:
    explicit B_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        string_view_t field_value{full_months[tm_time.tm_mon]};
        scoped_pad p(field_value, padinfo_, dest);
        fmt_helper::append_string_view(field_value, dest);
    }
};

// Date and time representation (Thu Aug 23 15:35:46 2014)
class c_formatter final : public flag_formatter
{
public:
    explicit c_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 24;
        scoped_pad p(field_size, padinfo_, dest);

        fmt_helper::append_string_view(days[tm_time.tm_wday], dest);
        dest.push_back(' ');
        fmt_helper::append_string_view(months[tm_time.tm_mon], dest);
        dest.push_back(' ');
        fmt_helper::append_int(tm_time.tm_mday, dest);
        dest.push_back(' ');
        // time

        fmt_helper::pad2(tm_time.tm_hour, dest);
        dest.push_back(':');
        fmt_helper::pad2(tm_time.tm_min, dest);
        dest.push_back(':');
        fmt_helper::pad2(tm_time.tm_sec, dest);
        dest.push_back(' ');
        fmt_helper::append_int(tm_time.tm_year + 1900, dest);
    }
};

// year - 2 digit
class C_formatter final : public flag_formatter
{
public:
    explicit C_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 2;
        scoped_pad p(field_size, padinfo_, dest);
        fmt_helper::pad2(tm_time.tm_year % 100, dest);
    }
};

// Short MM/DD/YY date, equivalent to %m/%d/%y 08/23/01
class D_formatter final : public flag_formatter
{
public:
    explicit D_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 10;
        scoped_pad p(field_size, padinfo_, dest);

        fmt_helper::pad2(tm_time.tm_mon + 1, dest);
        dest.push_back('/');
        fmt_helper::pad2(tm_time.tm_mday, dest);
        dest.push_back('/');
        fmt_helper::pad2(tm_time.tm_year % 100, dest);
    }
};

// year - 4 digit
class Y_formatter final : public flag_formatter
{
public:
    explicit Y_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 4;
        scoped_pad p(field_size, padinfo_, dest);
        fmt_helper::append_int(tm_time.tm_year + 1900, dest);
    }
};

// month 1-12
class m_formatter final : public flag_formatter
{
public:
    explicit m_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 2;
        scoped_pad p(field_size, padinfo_, dest);
        fmt_helper::pad2(tm_time.tm_mon + 1, dest);
    }
};

// day of month 1-31
class d_formatter final : public flag_formatter
{
public:
    explicit d_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 2;
        scoped_pad p(field_size, padinfo_, dest);
        fmt_helper::pad2(tm_time.tm_mday, dest);
    }
};

// hours in 24 format 0-23
class H_formatter final : public flag_formatter
{
public:
    explicit H_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 2;
        scoped_pad p(field_size, padinfo_, dest);
        fmt_helper::pad2(tm_time.tm_hour, dest);
    }
};

// hours in 12 format 1-12
class I_formatter final : public flag_formatter
{
public:
    explicit I_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 2;
        scoped_pad p(field_size, padinfo_, dest);
        fmt_helper::pad2(to12h(tm_time), dest);
    }
};

// minutes 0-59
class M_formatter final : public flag_formatter
{
public:
    explicit M_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 2;
        scoped_pad p(field_size, padinfo_, dest);
        fmt_helper::pad2(tm_time.tm_min, dest);
    }
};

// seconds 0-59
class S_formatter final : public flag_formatter
{
public:
    explicit S_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 2;
        scoped_pad p(field_size, padinfo_, dest);
        fmt_helper::pad2(tm_time.tm_sec, dest);
    }
};

// milliseconds
class e_formatter final : public flag_formatter
{
public:
    explicit e_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        auto millis = fmt_helper::time_fraction<std::chrono::milliseconds>(msg.time);
        if (padinfo_.enabled())
        {
            const size_t field_size = 3;
            scoped_pad p(field_size, padinfo_, dest);
            fmt_helper::pad3(static_cast<uint32_t>(millis.count()), dest);
        }
        else
        {
            fmt_helper::pad3(static_cast<uint32_t>(millis.count()), dest);
        }
    }
};

// microseconds
class f_formatter final : public flag_formatter
{
public:
    explicit f_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        auto micros = fmt_helper::time_fraction<std::chrono::microseconds>(msg.time);
        if (padinfo_.enabled())
        {
            const size_t field_size = 6;
            scoped_pad p(field_size, padinfo_, dest);
            fmt_helper::pad6(static_cast<size_t>(micros.count()), dest);
        }
        else
        {
            fmt_helper::pad6(static_cast<size_t>(micros.count()), dest);
        }
    }
};

// nanoseconds
class F_formatter final : public flag_formatter
{
public:
    explicit F_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        auto ns = fmt_helper::time_fraction<std::chrono::nanoseconds>(msg.time);
        if (padinfo_.enabled())
        {
            const size_t field_size = 9;
            scoped_pad p(field_size, padinfo_, dest);
            fmt_helper::pad9(static_cast<size_t>(ns.count()), dest);
        }
        else
        {
            fmt_helper::pad9(static_cast<size_t>(ns.count()), dest);
        }
    }
};

// seconds since epoch
class E_formatter final : public flag_formatter
{
public:
    explicit E_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 10;
        scoped_pad p(field_size, padinfo_, dest);
        auto duration = msg.time.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        fmt_helper::append_int(seconds, dest);
    }
};

// AM/PM
class p_formatter final : public flag_formatter
{
public:
    explicit p_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 2;
        scoped_pad p(field_size, padinfo_, dest);
        fmt_helper::append_string_view(ampm(tm_time), dest);
    }
};

// 12 hour clock 02:55:02 pm
class r_formatter final : public flag_formatter
{
public:
    explicit r_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 11;
        scoped_pad p(field_size, padinfo_, dest);

        fmt_helper::pad2(to12h(tm_time), dest);
        dest.push_back(':');
        fmt_helper::pad2(tm_time.tm_min, dest);
        dest.push_back(':');
        fmt_helper::pad2(tm_time.tm_sec, dest);
        dest.push_back(' ');
        fmt_helper::append_string_view(ampm(tm_time), dest);
    }
};

// 24-hour HH:MM time, equivalent to %H:%M
class R_formatter final : public flag_formatter
{
public:
    explicit R_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 5;
        scoped_pad p(field_size, padinfo_, dest);

        fmt_helper::pad2(tm_time.tm_hour, dest);
        dest.push_back(':');
        fmt_helper::pad2(tm_time.tm_min, dest);
    }
};

// ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S
class T_formatter final : public flag_formatter
{
public:
    explicit T_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 8;
        scoped_pad p(field_size, padinfo_, dest);

        fmt_helper::pad2(tm_time.tm_hour, dest);
        dest.push_back(':');
        fmt_helper::pad2(tm_time.tm_min, dest);
        dest.push_back(':');
        fmt_helper::pad2(tm_time.tm_sec, dest);
    }
};

// ISO 8601 offset from UTC in timezone (+-HH:MM)
class z_formatter final : public flag_formatter
{
public:
    explicit z_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    const std::chrono::seconds cache_refresh = std::chrono::seconds(5);

    z_formatter() = default;
    z_formatter(const z_formatter &) = delete;
    z_formatter &operator=(const z_formatter &) = delete;

    void format(const details::log_msg &msg, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 6;
        scoped_pad p(field_size, padinfo_, dest);

#ifdef _WIN32
        int total_minutes = get_cached_offset(msg, tm_time);
#else
        // No need to chache under gcc,
        // it is very fast (already stored in tm.tm_gmtoff)
        (void)(msg);
        int total_minutes = os::utc_minutes_offset(tm_time);
#endif
        bool is_negative = total_minutes < 0;
        if (is_negative)
        {
            total_minutes = -total_minutes;
            dest.push_back('-');
        }
        else
        {
            dest.push_back('+');
        }

        fmt_helper::pad2(total_minutes / 60, dest); // hours
        dest.push_back(':');
        fmt_helper::pad2(total_minutes % 60, dest); // minutes
    }

private:
    log_clock::time_point last_update_{std::chrono::seconds(0)};
#ifdef _WIN32
    int offset_minutes_{0};

    int get_cached_offset(const log_msg &msg, const std::tm &tm_time)
    {
        if (msg.time - last_update_ >= cache_refresh)
        {
            offset_minutes_ = os::utc_minutes_offset(tm_time);
            last_update_ = msg.time;
        }
        return offset_minutes_;
    }
#endif
};

// Thread id
class t_formatter final : public flag_formatter
{
public:
    explicit t_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        if (padinfo_.enabled())
        {
            const auto field_size = fmt_helper::count_digits(msg.thread_id);
            scoped_pad p(field_size, padinfo_, dest);
            fmt_helper::append_int(msg.thread_id, dest);
        }
        else
        {
            fmt_helper::append_int(msg.thread_id, dest);
        }
    }
};

// Current pid
class pid_formatter final : public flag_formatter
{
public:
    explicit pid_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &, const std::tm &, fmt::memory_buffer &dest) override
    {
        const auto pid = static_cast<uint32_t>(details::os::pid());
        if (padinfo_.enabled())
        {
            auto field_size = fmt_helper::count_digits(pid);
            scoped_pad p(field_size, padinfo_, dest);
            fmt_helper::append_int(pid, dest);
        }
        else
        {
            fmt_helper::append_int(pid, dest);
        }
    }
};

// message counter formatter
class i_formatter final : public flag_formatter
{
public:
    explicit i_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 6;
        scoped_pad p(field_size, padinfo_, dest);
        fmt_helper::pad6(msg.msg_id, dest);
    }
};

class v_formatter final : public flag_formatter
{
public:
    explicit v_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        if (padinfo_.enabled())
        {
            scoped_pad p(msg.payload, padinfo_, dest);
            fmt_helper::append_string_view(msg.payload, dest);
        }
        else
        {
            fmt_helper::append_string_view(msg.payload, dest);
        }
    }
};

class ch_formatter final : public flag_formatter
{
public:
    explicit ch_formatter(char ch)
        : ch_(ch)
    {
    }

    void format(const details::log_msg &, const std::tm &, fmt::memory_buffer &dest) override
    {
        const size_t field_size = 1;
        scoped_pad p(field_size, padinfo_, dest);
        dest.push_back(ch_);
    }

private:
    char ch_;
};

// aggregate user chars to display as is
class aggregate_formatter final : public flag_formatter
{
public:
    aggregate_formatter() = default;

    void add_ch(char ch)
    {
        str_ += ch;
    }
    void format(const details::log_msg &, const std::tm &, fmt::memory_buffer &dest) override
    {
        fmt_helper::append_string_view(str_, dest);
    }

private:
    std::string str_;
};

// mark the color range. expect it to be in the form of "%^colored text%$"
class color_start_formatter final : public flag_formatter
{
public:
    explicit color_start_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        msg.color_range_start = dest.size();
    }
};
class color_stop_formatter final : public flag_formatter
{
public:
    explicit color_stop_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        msg.color_range_end = dest.size();
    }
};

// print source location
class source_location_formatter final : public flag_formatter
{
public:
    explicit source_location_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        if (msg.source.empty())
        {
            return;
        }
        if (padinfo_.enabled())
        {
            const auto text_size = std::char_traits<char>::length(msg.source.filename) + fmt_helper::count_digits(msg.source.line) + 1;
            scoped_pad p(text_size, padinfo_, dest);
            fmt_helper::append_string_view(msg.source.filename, dest);
            dest.push_back(':');
            fmt_helper::append_int(msg.source.line, dest);
        }
        else
        {
            fmt_helper::append_string_view(msg.source.filename, dest);
            dest.push_back(':');
            fmt_helper::append_int(msg.source.line, dest);
        }
    }
};
// print source filename
class source_filename_formatter final : public flag_formatter
{
public:
    explicit source_filename_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        if (msg.source.empty())
        {
            return;
        }
        scoped_pad p(msg.source.filename, padinfo_, dest);
        fmt_helper::append_string_view(msg.source.filename, dest);
    }
};

class source_linenum_formatter final : public flag_formatter
{
public:
    explicit source_linenum_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        if (msg.source.empty())
        {
            return;
        }
        if (padinfo_.enabled())
        {
            auto field_size = fmt_helper::count_digits(msg.source.line);
            scoped_pad p(field_size, padinfo_, dest);
            fmt_helper::append_int(msg.source.line, dest);
        }
        else
        {
            fmt_helper::append_int(msg.source.line, dest);
        }
    }
};
// print source funcname
class source_funcname_formatter final : public flag_formatter
{
public:
    explicit source_funcname_formatter(padding_info padinfo)
        : flag_formatter(padinfo){};

    void format(const details::log_msg &msg, const std::tm &, fmt::memory_buffer &dest) override
    {
        if (msg.source.empty())
        {
            return;
        }
        scoped_pad p(msg.source.funcname, padinfo_, dest);
        fmt_helper::append_string_view(msg.source.funcname, dest);
    }
};

// Full info formatter
// pattern: [%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v
class full_formatter final : public flag_formatter
{
public:
    explicit full_formatter(padding_info padinfo)
        : flag_formatter(padinfo)
    {
    }

    void format(const details::log_msg &msg, const std::tm &tm_time, fmt::memory_buffer &dest) override
    {
        using std::chrono::duration_cast;
        using std::chrono::milliseconds;
        using std::chrono::seconds;

#ifndef SPDLOG_NO_DATETIME

        // cache the date/time part for the next second.
        auto duration = msg.time.time_since_epoch();
        auto secs = duration_cast<seconds>(duration);

        if (cache_timestamp_ != secs || cached_datetime_.size() == 0)
        {
            cached_datetime_.clear();
            cached_datetime_.push_back('[');
            fmt_helper::append_int(tm_time.tm_year + 1900, cached_datetime_);
            cached_datetime_.push_back('-');

            fmt_helper::pad2(tm_time.tm_mon + 1, cached_datetime_);
            cached_datetime_.push_back('-');

            fmt_helper::pad2(tm_time.tm_mday, cached_datetime_);
            cached_datetime_.push_back(' ');

            fmt_helper::pad2(tm_time.tm_hour, cached_datetime_);
            cached_datetime_.push_back(':');

            fmt_helper::pad2(tm_time.tm_min, cached_datetime_);
            cached_datetime_.push_back(':');

            fmt_helper::pad2(tm_time.tm_sec, cached_datetime_);
            cached_datetime_.push_back('.');

            cache_timestamp_ = secs;
        }
        fmt_helper::append_buf(cached_datetime_, dest);

        auto millis = fmt_helper::time_fraction<milliseconds>(msg.time);
        fmt_helper::pad3(static_cast<uint32_t>(millis.count()), dest);
        dest.push_back(']');
        dest.push_back(' ');

#else // no datetime needed
        (void)tm_time;
#endif

#ifndef SPDLOG_NO_NAME
        if (!msg.logger_name->empty())
        {
            dest.push_back('[');
            // fmt_helper::append_str(*msg.logger_name, dest);
            fmt_helper::append_string_view(*msg.logger_name, dest);
            dest.push_back(']');
            dest.push_back(' ');
        }
#endif

        dest.push_back('[');
        // wrap the level name with color
        msg.color_range_start = dest.size();
        // fmt_helper::append_string_view(level::to_c_str(msg.level), dest);
        fmt_helper::append_string_view(level::to_string_view(msg.level), dest);
        msg.color_range_end = dest.size();
        dest.push_back(']');
        dest.push_back(' ');

        // add source location if present
        if (!msg.source.empty())
        {
            dest.push_back('[');
            fmt_helper::append_string_view(msg.source.filename, dest);
            dest.push_back(':');
            fmt_helper::append_int(msg.source.line, dest);
            dest.push_back(']');
            dest.push_back(' ');
        }
        // fmt_helper::append_string_view(msg.msg(), dest);
        fmt_helper::append_string_view(msg.payload, dest);
    }

private:
    std::chrono::seconds cache_timestamp_{0};
    fmt::basic_memory_buffer<char, 128> cached_datetime_;
};

} // namespace details

class pattern_formatter final : public formatter
{
public:
    explicit pattern_formatter(
        std::string pattern, pattern_time_type time_type = pattern_time_type::local, std::string eol = spdlog::details::os::default_eol)
        : pattern_(std::move(pattern))
        , eol_(std::move(eol))
        , pattern_time_type_(time_type)
        , last_log_secs_(0)
    {
        std::memset(&cached_tm_, 0, sizeof(cached_tm_));
        compile_pattern_(pattern_);
    }

    // use by default full formatter for if pattern is not given
    explicit pattern_formatter(pattern_time_type time_type = pattern_time_type::local, std::string eol = spdlog::details::os::default_eol)
        : pattern_("%+")
        , eol_(std::move(eol))
        , pattern_time_type_(time_type)
        , last_log_secs_(0)
    {
        std::memset(&cached_tm_, 0, sizeof(cached_tm_));
        formatters_.push_back(details::make_unique<details::full_formatter>(details::padding_info{}));
    }

    pattern_formatter(const pattern_formatter &other) = delete;
    pattern_formatter &operator=(const pattern_formatter &other) = delete;

    std::unique_ptr<formatter> clone() const override
    {
        return details::make_unique<pattern_formatter>(pattern_, pattern_time_type_, eol_);
    }

    void format(const details::log_msg &msg, fmt::memory_buffer &dest) override
    {
#ifndef SPDLOG_NO_DATETIME
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(msg.time.time_since_epoch());
        if (secs != last_log_secs_)
        {
            cached_tm_ = get_time_(msg);
            last_log_secs_ = secs;
        }
#endif
        for (auto &f : formatters_)
        {
            f->format(msg, cached_tm_, dest);
        }
        // write eol
        details::fmt_helper::append_string_view(eol_, dest);
    }

private:
    std::string pattern_;
    std::string eol_;
    pattern_time_type pattern_time_type_;
    std::tm cached_tm_;
    std::chrono::seconds last_log_secs_;

    std::vector<std::unique_ptr<details::flag_formatter>> formatters_;

    std::tm get_time_(const details::log_msg &msg)
    {
        if (pattern_time_type_ == pattern_time_type::local)
        {
            return details::os::localtime(log_clock::to_time_t(msg.time));
        }
        return details::os::gmtime(log_clock::to_time_t(msg.time));
    }

    void handle_flag_(char flag, details::padding_info padding)
    {
        switch (flag)
        {

        case ('+'): // default formatter
            formatters_.push_back(details::make_unique<details::full_formatter>(padding));
            break;

        case 'n': // logger name
            formatters_.push_back(details::make_unique<details::name_formatter>(padding));
            break;

        case 'l': // level
            formatters_.push_back(details::make_unique<details::level_formatter>(padding));
            break;

        case 'L': // short level
            formatters_.push_back(details::make_unique<details::short_level_formatter>(padding));
            break;

        case ('t'): // thread id
            formatters_.push_back(details::make_unique<details::t_formatter>(padding));
            break;

        case ('v'): // the message text
            formatters_.push_back(details::make_unique<details::v_formatter>(padding));
            break;

        case ('a'): // weekday
            formatters_.push_back(details::make_unique<details::a_formatter>(padding));
            break;

        case ('A'): // short weekday
            formatters_.push_back(details::make_unique<details::A_formatter>(padding));
            break;

        case ('b'):
        case ('h'): // month
            formatters_.push_back(details::make_unique<details::b_formatter>(padding));
            break;

        case ('B'): // short month
            formatters_.push_back(details::make_unique<details::B_formatter>(padding));
            break;

        case ('c'): // datetime
            formatters_.push_back(details::make_unique<details::c_formatter>(padding));
            break;

        case ('C'): // year 2 digits
            formatters_.push_back(details::make_unique<details::C_formatter>(padding));
            break;

        case ('Y'): // year 4 digits
            formatters_.push_back(details::make_unique<details::Y_formatter>(padding));
            break;

        case ('D'):
        case ('x'): // datetime MM/DD/YY
            formatters_.push_back(details::make_unique<details::D_formatter>(padding));
            break;

        case ('m'): // month 1-12
            formatters_.push_back(details::make_unique<details::m_formatter>(padding));
            break;

        case ('d'): // day of month 1-31
            formatters_.push_back(details::make_unique<details::d_formatter>(padding));
            break;

        case ('H'): // hours 24
            formatters_.push_back(details::make_unique<details::H_formatter>(padding));
            break;

        case ('I'): // hours 12
            formatters_.push_back(details::make_unique<details::I_formatter>(padding));
            break;

        case ('M'): // minutes
            formatters_.push_back(details::make_unique<details::M_formatter>(padding));
            break;

        case ('S'): // seconds
            formatters_.push_back(details::make_unique<details::S_formatter>(padding));
            break;

        case ('e'): // milliseconds
            formatters_.push_back(details::make_unique<details::e_formatter>(padding));
            break;

        case ('f'): // microseconds
            formatters_.push_back(details::make_unique<details::f_formatter>(padding));
            break;

        case ('F'): // nanoseconds
            formatters_.push_back(details::make_unique<details::F_formatter>(padding));
            break;

        case ('E'): // seconds since epoch
            formatters_.push_back(details::make_unique<details::E_formatter>(padding));
            break;

        case ('p'): // am/pm
            formatters_.push_back(details::make_unique<details::p_formatter>(padding));
            break;

        case ('r'): // 12 hour clock 02:55:02 pm
            formatters_.push_back(details::make_unique<details::r_formatter>(padding));
            break;

        case ('R'): // 24-hour HH:MM time
            formatters_.push_back(details::make_unique<details::R_formatter>(padding));
            break;

        case ('T'):
        case ('X'): // ISO 8601 time format (HH:MM:SS)
            formatters_.push_back(details::make_unique<details::T_formatter>(padding));
            break;

        case ('z'): // timezone
            formatters_.push_back(details::make_unique<details::z_formatter>(padding));
            break;

        case ('P'): // pid
            formatters_.push_back(details::make_unique<details::pid_formatter>(padding));
            break;

#ifdef SPDLOG_ENABLE_MESSAGE_COUNTER
        case ('i'):
            formatters_.push_back(details::make_unique<details::i_formatter>(padding));
            break;
#endif
        case ('^'): // color range start
            formatters_.push_back(details::make_unique<details::color_start_formatter>(padding));
            break;

        case ('$'): // color range end
            formatters_.push_back(details::make_unique<details::color_stop_formatter>(padding));
            break;

        case ('@'): // source location (filename:filenumber)
            formatters_.push_back(details::make_unique<details::source_location_formatter>(padding));
            break;

        case ('s'): // source filename
            formatters_.push_back(details::make_unique<details::source_filename_formatter>(padding));
            break;

        case ('#'): // source line number
            formatters_.push_back(details::make_unique<details::source_linenum_formatter>(padding));
            break;

        case ('!'): // source funcname
            formatters_.push_back(details::make_unique<details::source_funcname_formatter>(padding));
            break;

        case ('%'): // % char
            formatters_.push_back(details::make_unique<details::ch_formatter>('%'));
            break;

        default: // Unknown flag appears as is
            auto unknown_flag = details::make_unique<details::aggregate_formatter>();
            unknown_flag->add_ch('%');
            unknown_flag->add_ch(flag);
            formatters_.push_back((std::move(unknown_flag)));
            break;
        }
    }

    // Extract given pad spec (e.g. %8X)
    // Advance the given it pass the end of the padding spec found (if any)
    // Return padding.
    details::padding_info handle_padspec_(std::string::const_iterator &it, std::string::const_iterator end)
    {
        using details::padding_info;
        using details::scoped_pad;
        const size_t max_width = 128;
        if (it == end)
        {
            return padding_info{};
        }

        padding_info::pad_side side;
        switch (*it)
        {
        case '-':
            side = padding_info::right;
            ++it;
            break;
        case '=':
            side = padding_info::center;
            ++it;
            break;
        default:
            side = details::padding_info::left;
            break;
        }

        if (it == end || !std::isdigit(static_cast<unsigned char>(*it)))
        {
            return padding_info{0, side};
        }

        auto width = static_cast<size_t>(*it - '0');
        for (++it; it != end && std::isdigit(static_cast<unsigned char>(*it)); ++it)
        {
            auto digit = static_cast<size_t>(*it - '0');
            width = width * 10 + digit;
        }
        return details::padding_info{std::min<size_t>(width, max_width), side};
    }

    void compile_pattern_(const std::string &pattern)
    {
        auto end = pattern.end();
        std::unique_ptr<details::aggregate_formatter> user_chars;
        formatters_.clear();
        for (auto it = pattern.begin(); it != end; ++it)
        {
            if (*it == '%')
            {
                if (user_chars) // append user chars found so far
                {
                    formatters_.push_back(std::move(user_chars));
                }

                auto padding = handle_padspec_(++it, end);

                if (it != end)
                {
                    handle_flag_(*it, padding);
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
                    user_chars = details::make_unique<details::aggregate_formatter>();
                }
                user_chars->add_ch(*it);
            }
        }
        if (user_chars) // append raw chars found so far
        {
            formatters_.push_back(std::move(user_chars));
        }
    }
};
} // namespace spdlog
