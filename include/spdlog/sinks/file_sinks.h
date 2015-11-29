//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <mutex>
#include "base_sink.h"
#include "../details/null_mutex.h"
#include "../details/file_helper.h"
#include "../details/format.h"

namespace spdlog
{
namespace sinks
{
/*
* Trivial file sink with single file as target
*/
template<class Mutex>
class simple_file_sink : public base_sink < Mutex >
{
public:
    explicit simple_file_sink(const std::string &filename,
                              bool force_flush = false) :
        _file_helper(force_flush)
    {
        _file_helper.open(filename);
    }
    void flush() override
    {
        _file_helper.flush();
    }

protected:
    void _sink_it(const details::log_msg& msg) override
    {
        _file_helper.write(msg);
    }
private:
    details::file_helper _file_helper;
};

typedef simple_file_sink<std::mutex> simple_file_sink_mt;
typedef simple_file_sink<details::null_mutex> simple_file_sink_st;

/*
* Rotating file sink based on size
*/
template<class Mutex>
class rotating_file_sink : public base_sink < Mutex >
{
public:
    rotating_file_sink(const std::string &base_filename, const std::string &extension,
                       std::size_t max_size, std::size_t max_files,
                       bool force_flush = false) :
        _base_filename(base_filename),
        _extension(extension),
        _max_size(max_size),
        _max_files(max_files),
        _current_size(0),
        _file_helper(force_flush)
    {
        _file_helper.open(calc_filename(_base_filename, 0, _extension));
        _current_size = _file_helper.size(); //expensive. called only once
    }

    void flush() override
    {
        _file_helper.flush();
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

private:
    static std::string calc_filename(const std::string& filename, std::size_t index, const std::string& extension)
    {
        fmt::MemoryWriter w;
        if (index)
            w.write("{}.{}.{}", filename, index, extension);
        else
            w.write("{}.{}", filename, extension);
        return w.str();
    }

    // Rotate files:
    // log.txt -> log.1.txt
    // log.1.txt -> log2.txt
    // log.2.txt -> log3.txt
    // log.3.txt -> delete

    void _rotate()
    {
        _file_helper.close();
        for (auto i = _max_files; i > 0; --i)
        {
            std::string src = calc_filename(_base_filename, i - 1, _extension);
            std::string target = calc_filename(_base_filename, i, _extension);

            if (details::file_helper::file_exists(target))
            {
                if (std::remove(target.c_str()) != 0)
                {
                    throw spdlog_ex("rotating_file_sink: failed removing " + target);
                }
            }
            if (details::file_helper::file_exists(src) && std::rename(src.c_str(), target.c_str()))
            {
                throw spdlog_ex("rotating_file_sink: failed renaming " + src + " to " + target);
            }
        }
        _file_helper.reopen(true);
    }
    std::string _base_filename;
    std::string _extension;
    std::size_t _max_size;
    std::size_t _max_files;
    std::size_t _current_size;
    details::file_helper _file_helper;
};

typedef rotating_file_sink<std::mutex> rotating_file_sink_mt;
typedef rotating_file_sink<details::null_mutex>rotating_file_sink_st;

/*
* Rotating file sink based on date. rotates at midnight
*/
template<class Mutex>
class daily_file_sink :public base_sink < Mutex >
{
public:
    //create daily file sink which rotates on given time
    daily_file_sink(
        const std::string& base_filename,
        const std::string& extension,
        int rotation_hour,
        int rotation_minute,
        bool force_flush = false) : _base_filename(base_filename),
        _extension(extension),
        _rotation_h(rotation_hour),
        _rotation_m(rotation_minute),
        _file_helper(force_flush)
    {
        if (rotation_hour < 0 || rotation_hour > 23 || rotation_minute < 0 || rotation_minute > 59)
            throw spdlog_ex("daily_file_sink: Invalid rotation time in ctor");
        _rotation_tp = _next_rotation_tp();
        _file_helper.open(calc_filename(_base_filename, _extension));
    }

    void flush() override
    {
        _file_helper.flush();
    }

protected:
    void _sink_it(const details::log_msg& msg) override
    {
        if (std::chrono::system_clock::now() >= _rotation_tp)
        {
            _file_helper.open(calc_filename(_base_filename, _extension));
            _rotation_tp = _next_rotation_tp();
        }
        _file_helper.write(msg);
    }

private:
    std::chrono::system_clock::time_point _next_rotation_tp()
    {
        using namespace std::chrono;
        auto now = system_clock::now();
        time_t tnow = std::chrono::system_clock::to_time_t(now);
        tm date = spdlog::details::os::localtime(tnow);
        date.tm_hour = _rotation_h;
        date.tm_min = _rotation_m;
        date.tm_sec = 0;
        auto rotation_time = std::chrono::system_clock::from_time_t(std::mktime(&date));
        if (rotation_time > now)
            return rotation_time;
        else
            return system_clock::time_point(rotation_time + hours(24));
    }

    //Create filename for the form basename.YYYY-MM-DD.extension
    static std::string calc_filename(const std::string& basename, const std::string& extension)
    {
        std::tm tm = spdlog::details::os::localtime();
        fmt::MemoryWriter w;
        w.write("{}_{:04d}-{:02d}-{:02d}_{:02d}-{:02d}.{}", basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, extension);
        return w.str();
    }

    std::string _base_filename;
    std::string _extension;
    int _rotation_h;
    int _rotation_m;
    std::chrono::system_clock::time_point _rotation_tp;
    details::file_helper _file_helper;
};

typedef daily_file_sink<std::mutex> daily_file_sink_mt;
typedef daily_file_sink<details::null_mutex> daily_file_sink_st;
}
}
