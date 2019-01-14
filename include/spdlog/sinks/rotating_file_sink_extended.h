//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#ifndef SPDLOG_H
#error "spdlog.h must be included before this file."
#endif

#include "spdlog/details/file_helper.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>

namespace spdlog {
namespace sinks {

template<typename Mutex>
class rotating_file_sink_extended final : public sinks::base_sink<Mutex>
{
public:
    rotating_file_sink_extended(
        const filename_t &base_filename, const filename_t &datetime_ext, std::size_t max_size, std::size_t max_files)
        : base_filename_(std::move(base_filename))
        , base_datetime_ext_(std::move(datetime_ext))
        , max_size_(max_size)
        , max_files_(max_files)
        , daily_rotate_(false)
    {
        file_helper_.open(calc_filename(0));
        current_size_ = file_helper_.size(); // expensive. called only once
    }

    rotating_file_sink_extended(const filename_t &base_filename, const filename_t &datetime_ext, std::size_t max_size,
        std::size_t max_files, int rotation_hour, int rotation_minute)
        : base_filename_(std::move(base_filename))
        , base_datetime_ext_(std::move(datetime_ext))
        , max_size_(max_size)
        , max_files_(max_files)
        , rotation_h_(rotation_hour)
        , rotation_m_(rotation_minute)
        , daily_rotate_(true)
    {
        if (rotation_hour < 0 || rotation_hour > 23 || rotation_minute < 0 || rotation_minute > 59)
        {
            throw spdlog_ex("daily_file_sink: Invalid rotation time in ctor");
        }

        file_helper_.open(calc_filename(0));
        current_size_ = file_helper_.size(); // expensive. called only once
        rotation_tp_ = next_rotation_tp_();
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        fmt::memory_buffer formatted;
        sink::formatter_->format(msg, formatted);
        current_size_ += formatted.size();

        if (daily_rotate_ && msg.time >= rotation_tp_)
        {
            file_helper_.open(calc_filename(0));
            rotation_tp_ = next_rotation_tp_();
            current_size_ = formatted.size();
        }

        if (current_size_ > max_size_)
        {
            rotate_();
            current_size_ = formatted.size();
        }

        file_helper_.write(formatted);
    }

    void flush_() override
    {
        file_helper_.flush();
    }

private:
    filename_t calc_filename(std::size_t index)
    {
        typename std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::memory_buffer, fmt::wmemory_buffer>::type w;
        typename std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::memory_buffer, fmt::wmemory_buffer>::type wt;

        filename_t filename = base_filename_;
        if (base_datetime_ext_.length() > 0)
        {
            filename_t basename, ext;
            std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
            fmt::format_to(wt, SPDLOG_FILENAME_T("{}_{}{}"), basename, formated_time(), ext);
            filename = fmt::to_string(wt);
        }
        else
        {
            auto now = log_clock::now();
            filename = daily_filename_calculator::calc_filename(base_filename_, now);
        }

        if (index != 0u)
        {
            filename_t basename, ext;
            std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
            fmt::format_to(w, SPDLOG_FILENAME_T("{}.{}{}"), basename, index, ext);
        }
        else
        {
            filename_t basename, ext;
            std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
            fmt::format_to(w, SPDLOG_FILENAME_T("{}.{}{}"), basename, index, ext);
        }

        return fmt::to_string(w);
    }

    void rotate_()
    {
        rotate_files_();
        file_helper_.close();
        file_helper_.open(calc_filename(0));
    }

    void rotate_files_()
    {
        using details::os::filename_to_str;
        file_helper_.close();
        for (auto i = max_files_; i > 0; --i)
        {
            filename_t src = calc_filename(i - 1);
            if (!details::file_helper::file_exists(src))
            {
                continue;
            }
            filename_t target = calc_filename(i);

            if (!rename_file(src, target))
            {
                // if failed try again after a small delay.
                // this is a workaround to a windows issue, where very high rotation
                // rates can cause the rename to fail with permission denied (because of antivirus?).
                details::os::sleep_for_millis(100);
                if (!rename_file(src, target))
                {
                    file_helper_.reopen(true); // truncate the log file anyway to prevent it to grow beyond its limit!
                    current_size_ = 0;
                    throw spdlog_ex(
                        "rotating_file_sink: failed renaming " + filename_to_str(src) + " to " + filename_to_str(target), errno);
                }
            }
        }
    }

    filename_t formated_time()
    {
        static filename_t empty_time;

        if (base_datetime_ext_.length() > 0)
        {
            auto now = log_clock::now();
            tm date = now_tm(now);

#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
            wchar_t buffer[63];
            wcsftime(buffer, sizeof(buffer), base_datetime_ext_.c_str(), &date);
            return filename_t(buffer);
#else
            char buffer[63];
            strftime(buffer, sizeof(buffer), base_datetime_ext_.c_str(), &date);
            return filename_t(buffer);
#endif
        }

        return empty_time;
    }

    tm now_tm(log_clock::time_point tp)
    {
        time_t tnow = log_clock::to_time_t(tp);
        return details::os::localtime(tnow);
    }

    log_clock::time_point next_rotation_tp_()
    {
        auto now = log_clock::now();
        tm date = now_tm(now);
        date.tm_hour = rotation_h_;
        date.tm_min = rotation_m_;
        date.tm_sec = 0;
        auto rotation_time = log_clock::from_time_t(std::mktime(&date));
        if (rotation_time > now)
        {
            return rotation_time;
        }

        return {rotation_time + std::chrono::hours(24)};
    }

    // delete the target if exists, and rename the src file  to target
    // return true on success, false otherwise.
    bool rename_file(const filename_t &src_filename, const filename_t &target_filename)
    {
        // try to delete the target file in case it already exists.
        (void)details::os::remove(target_filename);
        return details::os::rename(src_filename, target_filename) == 0;
    }

    filename_t base_filename_;
    filename_t base_datetime_ext_;
    std::size_t max_size_;
    std::size_t max_files_;
    std::size_t current_size_;
    details::file_helper file_helper_;

    bool daily_rotate_;
    int rotation_h_;
    int rotation_m_;
    log_clock::time_point rotation_tp_;
};

using rotating_file_sink_extended_mt = rotating_file_sink_extended<std::mutex>;
using rotating_file_sink_extended_st = rotating_file_sink_extended<details::null_mutex>;

} // namespace sinks
template<typename Factory = default_factory>
inline std::shared_ptr<logger> rotating_logger_extended_mt(
    const std::string &logger_name, const filename_t &filename, const filename_t &datetime_ext, size_t max_size, size_t max_files)
{
    return Factory::template create<sinks::rotating_file_sink_extended_mt>(logger_name, filename, datetime_ext, max_size, max_files);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> rotating_logger_extended_st(
    const std::string &logger_name, const filename_t &filename, const filename_t &datetime_ext, size_t max_size, size_t max_files)
{
    return Factory::template create<sinks::rotating_file_sink_extended_st>(logger_name, filename, datetime_ext, max_size, max_files);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> rotating_logger_extended_mt(const std::string &logger_name, const filename_t &filename,
    const filename_t &datetime_ext, size_t max_size, size_t max_files, int rotation_hour, int rotation_minute)
{
    return Factory::template create<sinks::rotating_file_sink_extended_mt>(
        logger_name, filename, datetime_ext, max_size, max_files, rotation_hour, rotation_minute);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> rotating_logger_extended_st(const std::string &logger_name, const filename_t &filename,
    const filename_t &datetime_ext, size_t max_size, size_t max_files, int rotation_hour, int rotation_minute)
{
    return Factory::template create<sinks::rotating_file_sink_extended_st>(
        logger_name, filename, datetime_ext, max_size, max_files, rotation_hour, rotation_minute);
}
} // namespace spdlog