//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
#include "spdlog/details/file_helper.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/spdlog.h"

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>
#include <tuple>


namespace spdlog {
namespace sinks {

struct daily_rotating_config
{
    filename_t base_filename;
    std::size_t max_size;
    std::size_t max_files;
    std::chrono::hours rotation_hour;
    std::chrono::minutes rotation_minute;
}

/*
 * Rotating file sink based on date. rotates at midnight
 */
template<typename Mutex, typename FileNameCalc = details::filename_calculator>
class daily_rotating_file_sink final : public base_sink<Mutex>
{
public:
    // create daily file sink which rotates on given time
	daily_rotating_file_sink(daily_rotating_config config, bool truncate = false)
        : base_filename_(std::move(config.base_filename))
        , max_size_(config.max_size)
        , max_files_(config.max_files)
        , rotation_h_(config.rotation_hour)
        , rotation_m_(config.rotation_minute)
        , truncate_(truncate)
    {
        if (rotation_hour < 0 || rotation_hour > 23 || rotation_minute < 0 || rotation_minute > 59)
        {
            throw spdlog_ex("daily_file_sink: Invalid rotation time in ctor");
        }
	
        auto now = log_clock::now();
        today_filename_ = FileNameCalc::calc_filename(base_filename_, now_tm(now));
        file_helper_.open(today_filename_, truncate_);
        rotation_tp_ = next_rotation_tp_();
        current_size_ = file_helper_.size(); // expensive. called only once
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        if (msg.time >= rotation_tp_)
        {
            today_filename_ = FileNameCalc::calc_filename(base_filename_, now_tm(msg.time));
            file_helper_.open(today_filename_, truncate_);
            rotation_tp_ = next_rotation_tp_();
        }

        fmt::memory_buffer formatted;
        sink::formatter_->format(msg, formatted);

        current_size_ += formatted.size();
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
    tm now_tm(log_clock::time_point tp)
    {
        time_t tnow = log_clock::to_time_t(tp);
        return spdlog::details::os::localtime(tnow);
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

    // Rotate files:
    // log.txt -> log.1.txt
    // log.1.txt -> log.2.txt
    // log.2.txt -> log.3.txt
    // log.3.txt -> delete
    void rotate_()
    {
        using details::os::filename_to_str;
        file_helper_.close();
        for (auto i = max_files_; i > 0; --i)
        {
            filename_t src = FileNameCalc::calc_filename(today_filename_, i - 1);
            if (!details::file_helper::file_exists(src))
            {
                continue;
            }
            filename_t target = FileNameCalc::calc_filename(today_filename_, i);

            if (!FileNameCalc::rename_file(src, target))
            {
                // if failed try again after a small delay.
                // this is a workaround to a windows issue, where very high rotation
                // rates can cause the rename to fail with permission denied (because of antivirus?).
                details::os::sleep_for_millis(100);
                if (!FileNameCalc::rename_file(src, target))
                {
                    file_helper_.reopen(true); // truncate the log file anyway to prevent it to grow beyond its limit!
                    throw spdlog_ex(
                        "rotating_file_sink: failed renaming " + filename_to_str(src) + " to " + filename_to_str(target), errno);
                }
            }
        }
        file_helper_.reopen(true);
    }

    std::size_t max_size_;
    std::size_t max_files_;
    std::size_t current_size_;
    filename_t today_filename_;
    filename_t base_filename_;
    int rotation_h_;
    int rotation_m_;
    log_clock::time_point rotation_tp_;
    details::file_helper file_helper_;
    bool truncate_;
};

using daily_rotating_file_sink_mt = daily_rotating_file_sink<std::mutex>;
using daily_rotating_file_sink_st = daily_rotating_file_sink<details::null_mutex>;

} // namespace sinks

//
// factory functions
//
template<typename Factory = default_factory>
inline std::shared_ptr<logger> daily_rotating_logger_mt(
    const std::string &logger_name, const daily_rotating_config &config, bool truncate = false)
{
    return Factory::template create<sinks::daily_rotating_file_sink_mt>(logger_name, config, truncate);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> daily_rotating_logger_st(
    const std::string &logger_name, const daily_rotating_config &config, bool truncate = false)
{
    return Factory::template create<sinks::daily_rotating_file_sink_st>(logger_name, config, truncate);
}
} // namespace spdlog
