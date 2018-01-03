//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "base_sink.h"
#include "../details/null_mutex.h"
#include "../details/file_helper.h"
#include "../fmt/fmt.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>
#include <cerrno>

namespace spdlog
{
namespace sinks
{
/*
 * Trivial file sink with single file as target
 */
template<class Mutex>
class simple_file_sink SPDLOG_FINAL : public base_sink < Mutex >
{
public:
    explicit simple_file_sink(const filename_t &filename, bool truncate = false):_force_flush(false)
    {
        _file_helper.open(filename, truncate);
    }

    void set_force_flush(bool force_flush)
    {
        _force_flush = force_flush;
    }

protected:
    void _sink_it(const details::log_msg& msg) override
    {
        _file_helper.write(msg);
        if(_force_flush)
            _file_helper.flush();
    }
    void _flush() override
    {
        _file_helper.flush();
    }
private:
    details::file_helper _file_helper;
    bool _force_flush;
};

typedef simple_file_sink<std::mutex> simple_file_sink_mt;
typedef simple_file_sink<details::null_mutex> simple_file_sink_st;

struct rotating_filename_calculator
{
    // calc filename according to index and file extension if exists.
    // e.g. calc_filename("logs/mylog.txt, 3) => "logs/mylog.3.txt".
    static filename_t calc_filename(const filename_t& filename, std::size_t index, const filename_t& extra_part = "")
    {
        std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::MemoryWriter, fmt::WMemoryWriter>::type w;
        if (index)
        {
            filename_t basename, ext;
            std::tie(basename, ext) = details::file_helper::split_by_extenstion(filename);
            w.write(SPDLOG_FILENAME_T("{}{}.{}{}"), basename, extra_part, index, ext);
        }
        else
        {
            w.write(SPDLOG_FILENAME_T("{}"), filename);
        }
        return w.str();
    }
};

struct no_extra_filename_part
{
    static filename_t calc_filename_part()
    {
        return "";
    }
};

struct dateonly_filename_part
{
    static filename_t calc_filename_part()
    {
        std::tm tm = spdlog::details::os::localtime();
        std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::MemoryWriter, fmt::WMemoryWriter>::type w;
        w.write(SPDLOG_FILENAME_T("_{:04d}-{:02d}-{:02d}"), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
        return w.str();
    }
};

struct rolling_rotate
{
    // Rotate files:
    // log.txt -> log.1.txt
    // log.1.txt -> log.2.txt
    // log.2.txt -> log.3.txt
    // log.3.txt -> delete
    static void rotate(std::size_t max_files, const filename_t& base_filename)
    {
        using details::os::filename_to_str;
        for (auto i = max_files; i > 0; --i)
        {
            filename_t src = rotating_filename_calculator::calc_filename(base_filename, i - 1);
            filename_t target = rotating_filename_calculator::calc_filename(base_filename, i);

            if (details::file_helper::file_exists(target))
            {
                if (details::os::remove(target) != 0)
                {
                    throw spdlog_ex("rotating_file_sink: failed removing " + filename_to_str(target), errno);
                }
            }
            if (details::file_helper::file_exists(src) && details::os::rename(src, target))
            {
                throw spdlog_ex("rotating_file_sink: failed renaming " + filename_to_str(src) + " to " + filename_to_str(target), errno);
            }
        }
    }
};

template <class ExtraFilenamePart = dateonly_filename_part>
struct sequence_rotate
{
    // Rotate files:
    // log.txt -> log.3.txt
    // log.3.txt -> log.2.txt
    // log.2.txt -> log.1.txt
    // log.1.txt -> delete
    static void rotate(std::size_t max_files, const filename_t& base_filename)
    {
        using details::os::filename_to_str;
        filename_t extra_part = ExtraFilenamePart::calc_filename_part();

        auto last_file = rotating_filename_calculator::calc_filename(base_filename, max_files, extra_part);
        if (details::file_helper::file_exists(last_file))
        {
            sequence_rotate::_rotate(max_files, base_filename, extra_part);
            return;
        }

        auto src = rotating_filename_calculator::calc_filename(base_filename, 0);
        filename_t target;
        for (auto i = 1; i <= max_files; ++i)
        {
            target = rotating_filename_calculator::calc_filename(base_filename, i, extra_part);
            if (!details::file_helper::file_exists(target))
            {
                break;
            }
        }
        if (details::file_helper::file_exists(src) && details::os::rename(src, target))
        {
            throw spdlog_ex("rotating_file_sink: failed renaming " + filename_to_str(src) + " to " + filename_to_str(target), errno);
        }
    }

    static void _rotate(std::size_t max_files, const filename_t& base_filename, const filename_t& extra_part)
    {
        using details::os::filename_to_str;
        auto mod_base = max_files + 1;
        for (auto i = 1; i <= max_files; ++i)
        {
            filename_t src = rotating_filename_calculator::calc_filename(base_filename, (i + 1) % mod_base, extra_part);
            filename_t target = rotating_filename_calculator::calc_filename(base_filename, i, extra_part);

            if (details::file_helper::file_exists(target))
            {
                if (details::os::remove(target) != 0)
                {
                    throw spdlog_ex("rotating_file_sink: failed removing " + filename_to_str(target), errno);
                }
            }
            if (details::file_helper::file_exists(src) && details::os::rename(src, target))
            {
                throw spdlog_ex("rotating_file_sink: failed renaming " + filename_to_str(src) + " to " + filename_to_str(target), errno);
            }
        }
    }
};

/*
 * Rotating file sink based on size
 */
template<class Mutex, class Rotating = rolling_rotate>
class rotating_file_sink SPDLOG_FINAL : public base_sink < Mutex >
{
public:
    rotating_file_sink(const filename_t &base_filename,
                       std::size_t max_size, std::size_t max_files) :
        _base_filename(base_filename),
        _max_size(max_size),
        _max_files(max_files),
        _current_size(0),
        _file_helper()
    {
        _file_helper.open(rotating_filename_calculator::calc_filename(_base_filename, 0));
        _current_size = _file_helper.size(); //expensive. called only once
    }

protected:
    void _sink_it(const details::log_msg& msg) override
    {
        _current_size += msg.formatted.size();
        if (_current_size > _max_size)
        {
            _file_helper.close();
            Rotating::rotate(_max_files, _base_filename);
            _file_helper.reopen(true);
            _current_size = msg.formatted.size();
        }
        _file_helper.write(msg);
    }

    void _flush() override
    {
        _file_helper.flush();
    }


private:
    filename_t _base_filename;
    std::size_t _max_size;
    std::size_t _max_files;
    std::size_t _current_size;
    details::file_helper _file_helper;
};

typedef rotating_file_sink<std::mutex> rotating_file_sink_mt;
typedef rotating_file_sink<details::null_mutex>rotating_file_sink_st;
typedef rotating_file_sink<std::mutex, sequence_rotate<dateonly_filename_part>> rotating_file_with_date_sink_mt;
typedef rotating_file_sink<details::null_mutex, sequence_rotate<dateonly_filename_part>> rotating_file_with_date_sink_st;

/*
 * Default generator of daily log file names.
 */
struct default_daily_file_name_calculator
{
    // Create filename for the form filename.YYYY-MM-DD_hh-mm.ext
    static filename_t calc_filename(const filename_t& filename)
    {
        std::tm tm = spdlog::details::os::localtime();
        filename_t basename, ext;
        std::tie(basename, ext) = details::file_helper::split_by_extenstion(filename);
        std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::MemoryWriter, fmt::WMemoryWriter>::type w;
        w.write(SPDLOG_FILENAME_T("{}_{:04d}-{:02d}-{:02d}_{:02d}-{:02d}{}"), basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, ext);
        return w.str();
    }
};

/*
 * Generator of daily log file names in format basename.YYYY-MM-DD.ext
 */
struct dateonly_daily_file_name_calculator
{
    // Create filename for the form basename.YYYY-MM-DD
    static filename_t calc_filename(const filename_t& filename)
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
class daily_file_sink SPDLOG_FINAL :public base_sink < Mutex >
{
public:
    //create daily file sink which rotates on given time
    daily_file_sink(
        const filename_t& base_filename,
        int rotation_hour,
        int rotation_minute) : _base_filename(base_filename),
        _rotation_h(rotation_hour),
        _rotation_m(rotation_minute)
    {
        if (rotation_hour < 0 || rotation_hour > 23 || rotation_minute < 0 || rotation_minute > 59)
            throw spdlog_ex("daily_file_sink: Invalid rotation time in ctor");
        _rotation_tp = _next_rotation_tp();
        _file_helper.open(FileNameCalc::calc_filename(_base_filename));
    }


protected:
    void _sink_it(const details::log_msg& msg) override
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
            return rotation_time;
        else
            return std::chrono::system_clock::time_point(rotation_time + std::chrono::hours(24));
    }

    filename_t _base_filename;
    int _rotation_h;
    int _rotation_m;
    std::chrono::system_clock::time_point _rotation_tp;
    details::file_helper _file_helper;
};

typedef daily_file_sink<std::mutex> daily_file_sink_mt;
typedef daily_file_sink<details::null_mutex> daily_file_sink_st;
}
}
