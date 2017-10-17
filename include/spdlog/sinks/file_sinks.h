//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/details/file_helper.h"
#include "spdlog/fmt/fmt.h"

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

/*
 * Rotating file sink based on size
 */
template<class Mutex>
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
        _file_helper.open(calc_filename(_base_filename, 0));
        _current_size = _file_helper.size(); //expensive. called only once
    }


protected:
    void _sink_it(const details::log_msg& msg) override
    {
        _current_size += msg.formatted.size();
        if (_current_size > _max_size)
        {
            _rotate();
            _current_size = msg.formatted.size();
        }
        _file_helper.write(msg);
    }

    void _flush() override
    {
        _file_helper.flush();
    }

private:
    static filename_t calc_filename(const filename_t& filename, std::size_t index)
    {
        std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::MemoryWriter, fmt::WMemoryWriter>::type w;
        if (index)
            w.write(SPDLOG_FILENAME_T("{}.{}"), filename, index);
        else
            w.write(SPDLOG_FILENAME_T("{}"), filename);
        return w.str();
    }

    // Rotate files:
    // log.txt -> log.txt.1
    // log.txt.1 -> log.txt.2
    // log.txt.2 -> log.txt.3
    // lo3.txt.3 -> delete

    void _rotate()
    {
        using details::os::filename_to_str;
        _file_helper.close();
        for (auto i = _max_files; i > 0; --i)
        {
            filename_t src = calc_filename(_base_filename, i - 1);
            filename_t target = calc_filename(_base_filename, i);

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
        _file_helper.reopen(true);
    }
    filename_t _base_filename;
    std::size_t _max_size;
    std::size_t _max_files;
    std::size_t _current_size;
    details::file_helper _file_helper;
};

typedef rotating_file_sink<std::mutex> rotating_file_sink_mt;
typedef rotating_file_sink<details::null_mutex>rotating_file_sink_st;

// Rotating file sink for multi-process environment
template<class Mutex>
class rotating_file_sink_mp SPDLOG_FINAL : public spdlog::sinks::base_sink < Mutex >
{
public:
    rotating_file_sink_mp(const spdlog::filename_t & base_filename,
                          const std::size_t& max_size,
                          const std::size_t& max_files) :
        _base_filename(base_filename),
        _max_size(max_size),
        _max_files(max_files),
        _file_helper()
    {
        _rotating_file = get_folder_path(_base_filename) + "rotating";
        _file_helper.open(calc_filename(_base_filename, 0));
    }

protected:
    void _sink_it(const details::log_msg& msg) override
    {
        if (details::os::file_exists(_rotating_file))
        {
            wait_till_rotation_completed();
        }
        else
        {
            size_t file_size = _file_helper.size();
            if ( (file_size + msg.formatted.size()) > _max_size )
            {
                // Check that _file_helper give us real size of a log file and not a size of already rotated (old) log file
                if (file_size <= details::os::filesize(_file_helper.filename()))
                {
                    // Because previous operation was long, check that other processes did not started rotation
                    if (details::os::file_exists(_rotating_file))
                    {
                        wait_till_rotation_completed();
                    }
                    else
                    {
                        safe_rotation();
                    }
                }
                else
                {
                    // File helper have file descriptor to rotated log file. We need to close it and open current log file
                    _file_helper.reopen(false);
                }
            }
        }

        _file_helper.write(msg);
    }

    void _flush() override
    {
        _file_helper.flush();
    }

private:
    static filename_t get_folder_path(const filename_t& filename)
    {
        size_t pos = filename.find_last_of('/');
        if (pos == std::string::npos)
        {
            return filename_t("/");
        }

        filename_t path;
        path.assign(filename, 0, pos + 1);
        return path;
    }

    static filename_t calc_filename(const filename_t& filename, std::size_t index)
    {
        std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::MemoryWriter, fmt::WMemoryWriter>::type w;
        if (index)
            w.write(SPDLOG_FILENAME_T("{}.{}"), filename, index);
        else
            w.write(SPDLOG_FILENAME_T("{}"), filename);

        return w.str();
    }

    void wait_till_rotation_completed()
    {
        _file_helper.close();

        const int max_tries = 100;
        const int sleep_time = 10;
        for (int i = 0; i < max_tries; ++i)
        {
            if (!details::os::file_exists(_rotating_file))
            {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }

        _file_helper.reopen(false);
    }

    void safe_rotation()
    {
        // create rotating file
        details::file_helper_mp rotating_file;
        rotating_file.open(_rotating_file);
        rotating_file.close();

        // close log file
        _file_helper.close();

        // perform rotation
        _rotate();

        // open log file and truncate it
        _file_helper.reopen(true);

        // delete rotating file
        details::os::remove(_rotating_file);
    }

    // Rotate files:
    // log.txt -> log.txt.1
    // log.txt.1 -> log.txt.2
    // log.txt.2 -> log.txt.3
    // lo3.txt.3 -> delete
    void _rotate()
    {
        for (auto i = _max_files; i > 0; --i)
        {
            filename_t src = calc_filename(_base_filename, i - 1);
            filename_t target = calc_filename(_base_filename, i);
            if (details::os::file_exists(target))
            {
                if (details::os::remove(target) != 0)
                {
                    throw spdlog_ex("rotating_file_sink: failed removing " + details::os::filename_to_str(target), errno);
                }
            }

            if (details::os::file_exists(src) && details::os::rename(src, target))
            {
                throw spdlog_ex("rotating_file_sink: failed renaming " + details::os::filename_to_str(src) + " to " + details::os::filename_to_str(target), errno);
            }
        }
    }

private:
    filename_t _base_filename;
    filename_t _rotating_file;
    std::size_t _max_size;
    std::size_t _max_files;
    details::file_helper_mp _file_helper;
};

typedef rotating_file_sink_mp<std::mutex> rotating_file_sink_mp_mt;
typedef rotating_file_sink_mp<spdlog::details::null_mutex> rotating_file_sink_mp_st;

/*
 * Default generator of daily log file names.
 */
struct default_daily_file_name_calculator
{
    // Create filename for the form basename.YYYY-MM-DD_hh-mm
    static filename_t calc_filename(const filename_t& basename)
    {
        std::tm tm = spdlog::details::os::localtime();
        std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::MemoryWriter, fmt::WMemoryWriter>::type w;
        w.write(SPDLOG_FILENAME_T("{}_{:04d}-{:02d}-{:02d}_{:02d}-{:02d}"), basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
        return w.str();
    }
};

/*
 * Generator of daily log file names in format basename.YYYY-MM-DD
 */
struct dateonly_daily_file_name_calculator
{
    // Create filename for the form basename.YYYY-MM-DD
    static filename_t calc_filename(const filename_t& basename)
    {
        std::tm tm = spdlog::details::os::localtime();
        std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::MemoryWriter, fmt::WMemoryWriter>::type w;
        w.write(SPDLOG_FILENAME_T("{}_{:04d}-{:02d}-{:02d}"), basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
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
