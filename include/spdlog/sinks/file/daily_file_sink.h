//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/details/file_helper.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/sinks/base_sink.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>

namespace spdlog {
namespace sinks {

/*
 * Default generator of daily log file names.
 */
struct default_daily_file_name_calculator
{
    // Create filename for the form filename.YYYY-MM-DD_hh-mm.ext
    static filename_t calc_filename(const filename_t &filename)
    {
        std::tm tm = spdlog::details::os::localtime();
        filename_t basename, ext;
        std::tie(basename, ext) = details::file_helper::split_by_extenstion(filename);
        std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::MemoryWriter, fmt::WMemoryWriter>::type w;
        w.write(SPDLOG_FILENAME_T("{}_{:04d}-{:02d}-{:02d}_{:02d}-{:02d}{}"), basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, ext);
        return w.str();
    }
};

/*
 * Generator of daily log file names in format basename.YYYY-MM-DD.ext
 */
struct dateonly_daily_file_name_calculator
{
    // Create filename for the form basename.YYYY-MM-DD
    static filename_t calc_filename(const filename_t &filename)
    {
        std::tm tm = spdlog::details::os::localtime();
        filename_t basename, ext;
        std::tie(basename, ext) = details::file_helper::split_by_extenstion(filename);
        std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::MemoryWriter, fmt::WMemoryWriter>::type w;
        w.write(SPDLOG_FILENAME_T("{}_{:04d}-{:02d}-{:02d}{}"), basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, ext);
        return w.str();
    }
};

/*
 * Rotating file sink based on date. rotates at midnight
 */
template<class Mutex, class FileNameCalc = default_daily_file_name_calculator>
class daily_file_sink SPDLOG_FINAL : public base_sink<Mutex>
{
public:
    // create daily file sink which rotates on given time
    daily_file_sink(filename_t base_filename, int rotation_hour, int rotation_minute)
        : _base_filename(std::move(base_filename))
        , _rotation_h(rotation_hour)
        , _rotation_m(rotation_minute)
    {
        if (rotation_hour < 0 || rotation_hour > 23 || rotation_minute < 0 || rotation_minute > 59)
        {
            throw spdlog_ex("daily_file_sink: Invalid rotation time in ctor");
        }
        _rotation_tp = _next_rotation_tp();
        _file_helper.open(FileNameCalc::calc_filename(_base_filename));
    }

protected:
    void _sink_it(const details::log_msg &msg) override
    {
        if (std::chrono::system_clock::now() >= _rotation_tp)
        {
            _file_helper.open(FileNameCalc::calc_filename(_base_filename));
            _rotation_tp = _next_rotation_tp();
        }
        _file_helper.write(msg);
    }

    void _flush() override
    {
        _file_helper.flush();
    }

private:
    std::chrono::system_clock::time_point _next_rotation_tp()
    {
        auto now = std::chrono::system_clock::now();
        time_t tnow = std::chrono::system_clock::to_time_t(now);
        tm date = spdlog::details::os::localtime(tnow);
        date.tm_hour = _rotation_h;
        date.tm_min = _rotation_m;
        date.tm_sec = 0;
        auto rotation_time = std::chrono::system_clock::from_time_t(std::mktime(&date));
        if (rotation_time > now)
        {
            return rotation_time;
        }
        return {rotation_time + std::chrono::hours(24)};
    }

    filename_t _base_filename;
    int _rotation_h;
    int _rotation_m;
    std::chrono::system_clock::time_point _rotation_tp;
    details::file_helper _file_helper;
};

using daily_file_sink_mt = daily_file_sink<std::mutex>;
using daily_file_sink_st = daily_file_sink<details::null_mutex>;

} // namespace sinks

//
// factory functions 
//
template<typename Factory = default_factory>
inline std::shared_ptr<logger> daily_logger_mt(const std::string &logger_name, const filename_t &filename, int hour = 0, int minute = 0)
{
    return Factory::template create<sinks::daily_file_sink_mt>(logger_name, filename, hour, minute);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> daily_logger_st(const std::string &logger_name, const filename_t &filename, int hour = 0, int minute = 0)
{
    return Factory::template create<sinks::daily_file_sink_st>(logger_name, filename, hour, minute);
}
} // namespace spdlog
