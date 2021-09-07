// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/formatter.h>
#include <spdlog/details/os.h>
#include <spdlog/details/fmt_helper.h>

// Default spdlog formatter
// pattern: [%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%s:%#] %v
namespace spdlog {
class default_formatter final : public formatter
{
public:
    default_formatter() = default;
    ~default_formatter() = default;

    std::unique_ptr<formatter> clone() const override
    {
        return details::make_unique<default_formatter>();
    }

    void format(const details::log_msg &msg, memory_buf_t &dest) override
    {
        using std::chrono::duration_cast;
        using std::chrono::milliseconds;
        using std::chrono::seconds;
        // cache the date/time part for the next second.
        auto duration = msg.time.time_since_epoch();
        auto secs = duration_cast<seconds>(duration);
        //[2021-08-23 00:57:12.310] [info] Welcome to spdlog version 1.9.2  !
        if (cache_timestamp_ != secs || cached_datetime_.size() == 0)
        {
            auto tm_time = details::os::localtime(log_clock::to_time_t(msg.time));
            cached_datetime_.clear();
            cached_datetime_.push_back('[');
            details::fmt_helper::append_int(tm_time.tm_year + 1900, cached_datetime_);
            cached_datetime_.push_back('-');

            details::fmt_helper::pad2(tm_time.tm_mon + 1, cached_datetime_);
            cached_datetime_.push_back('-');

            details::fmt_helper::pad2(tm_time.tm_mday, cached_datetime_);
            cached_datetime_.push_back(' ');

            details::fmt_helper::pad2(tm_time.tm_hour, cached_datetime_);
            cached_datetime_.push_back(':');

            details::fmt_helper::pad2(tm_time.tm_min, cached_datetime_);
            cached_datetime_.push_back(':');

            details::fmt_helper::pad2(tm_time.tm_sec, cached_datetime_);
            cached_datetime_.push_back('.');

            cache_timestamp_ = secs;
        }
        dest.append(cached_datetime_.begin(), cached_datetime_.end());
        auto millis = details::fmt_helper::time_fraction<milliseconds>(msg.time);
        details::fmt_helper::pad3(static_cast<uint32_t>(millis.count()), dest);
        dest.push_back(']');
        dest.push_back(' ');

        // append logger name if exists
        if (msg.logger_name.size() > 0)
        {
            dest.push_back('[');
            details::fmt_helper::append_string_view(msg.logger_name, dest);
            dest.push_back(']');
            dest.push_back(' ');
        }
        dest.push_back('[');
        // wrap the level name with color
        msg.color_range_start = dest.size();
        // fmt_helper::append_string_view(level::to_c_str(msg.level), dest);
        details::fmt_helper::append_string_view(level::to_string_view(msg.level), dest);
        msg.color_range_end = dest.size();
        dest.push_back(']');
        dest.push_back(' ');

        // add source location if present

        if (!msg.source.empty())
        {
            dest.push_back('[');
            // const char *filename = details::short_filename_formatter<details::null_scoped_padder>::basename(msg.source.filename);
            // details::fmt_helper::append_string_view(filename, dest);
            dest.push_back(':');
            details::fmt_helper::append_int(msg.source.line, dest);
            dest.push_back(']');
            dest.push_back(' ');
        }
        details::fmt_helper::append_string_view(msg.payload, dest);
        details::fmt_helper::append_string_view(details::os::default_eol, dest);
    }

private:
    std::chrono::seconds cache_timestamp_{0};
    memory_buf_t cached_datetime_;
};
} // namespace spdlog
