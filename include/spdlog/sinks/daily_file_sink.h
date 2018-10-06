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

#include <chrono>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>

namespace spdlog {
namespace sinks {

/*
 * Rotating file sink based on date. rotates at midnight
 */
template<typename Mutex, typename FileNameCalc = details::filename_calculator>
class daily_file_sink final : public base_sink<Mutex>
{
public:
    // create daily file sink which rotates on given time
    daily_file_sink(filename_t base_filename, int rotation_hour, int rotation_minute, bool truncate = false)
        : base_filename_(std::move(base_filename))
        , rotation_h_(rotation_hour)
        , rotation_m_(rotation_minute)
        , truncate_(truncate)
    {
        if (rotation_hour < 0 || rotation_hour > 23 || rotation_minute < 0 || rotation_minute > 59)
        {
            throw spdlog_ex("daily_file_sink: Invalid rotation time in ctor");
        }
        auto now = log_clock::now();
        file_helper_.open(FileNameCalc::calc_filename(base_filename_, now_tm(now)), truncate_);
        rotation_tp_ = next_rotation_tp_();
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {

        if (msg.time >= rotation_tp_)
        {
            file_helper_.open(FileNameCalc::calc_filename(base_filename_, now_tm(msg.time)), truncate_);
            rotation_tp_ = next_rotation_tp_();
        }
        fmt::memory_buffer formatted;
        sink::formatter_->format(msg, formatted);
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

    filename_t base_filename_;
    int rotation_h_;
    int rotation_m_;
    log_clock::time_point rotation_tp_;
    details::file_helper file_helper_;
    bool truncate_;
};

using daily_file_sink_mt = daily_file_sink<std::mutex>;
using daily_file_sink_st = daily_file_sink<details::null_mutex>;

} // namespace sinks

//
// factory functions
//
template<typename Factory = default_factory>
inline std::shared_ptr<logger> daily_logger_mt(
    const std::string &logger_name, const filename_t &filename, int hour = 0, int minute = 0, bool truncate = false)
{
    return Factory::template create<sinks::daily_file_sink_mt>(logger_name, filename, hour, minute, truncate);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> daily_logger_st(
    const std::string &logger_name, const filename_t &filename, int hour = 0, int minute = 0, bool truncate = false)
{
    return Factory::template create<sinks::daily_file_sink_st>(logger_name, filename, hour, minute, truncate);
}
} // namespace spdlog
