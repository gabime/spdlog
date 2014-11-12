/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2014 Gabi Melman.                                       */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

#include <mutex>
#include "base_sink.h"
#include "../details/null_mutex.h"
#include "../details/file_helper.h"
#include "../details/fast_oss.h"



namespace spdlog
{
namespace sinks
{

/*
* Trivial file sink with single file as target
*/
template<class Mutex>
class simple_file_sink : public base_sink<Mutex>
{
public:
    explicit simple_file_sink(const std::string &filename,
                              const std::size_t flush_inverval=0):
        _file_helper(flush_inverval)
    {
        _file_helper.open(filename);
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
class rotating_file_sink : public base_sink<Mutex>
{
public:
    rotating_file_sink(const std::string &base_filename, const std::string &extension,
                       std::size_t max_size, std::size_t max_files,
                       std::size_t flush_inverval=0):
        _base_filename(base_filename),
        _extension(extension),
        _max_size(max_size),
        _max_files(max_files),
        _current_size(0),
        _file_helper(flush_inverval)
    {
        _file_helper.open(calc_filename(_base_filename, 0, _extension));
    }


protected:
    void _sink_it(const details::log_msg& msg) override
    {
        _current_size += msg.formatted.size();
        if (_current_size  > _max_size)
        {
            _rotate();
            _current_size = msg.formatted.size();
        }
        _file_helper.write(msg);
    }


private:
    static std::string calc_filename(const std::string& filename, std::size_t index, const std::string& extension)
    {
        details::fast_oss oss;
        if (index)
            oss << filename << "." << index << "." << extension;
        else
            oss << filename << "." << extension;
        return oss.str();
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
        auto cur_name = _file_helper.filename();
        if (std::remove(cur_name.c_str()) != 0)
	  {
	    throw spdlog_ex("rotating_file_sink: failed removing " + cur_name);
	  }
        _file_helper.open(cur_name);
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
class daily_file_sink:public base_sink<Mutex>
{
public:
    explicit daily_file_sink(const std::string& base_filename,
                             const std::string& extension,
                             const std::size_t flush_inverval=0):
        _base_filename(base_filename),
        _extension(extension),
        _midnight_tp (_calc_midnight_tp() ),
        _file_helper(flush_inverval)
    {
        _file_helper.open(calc_filename(_base_filename, _extension));
    }


protected:
    void _sink_it(const details::log_msg& msg) override
    {
        if (std::chrono::system_clock::now() >= _midnight_tp)
        {
            _file_helper.close();
            _file_helper.open(calc_filename(_base_filename, _extension));
            _midnight_tp = _calc_midnight_tp();
        }
        _file_helper.write(msg);
    }

private:
    // Return next midnight's time_point
    static std::chrono::system_clock::time_point _calc_midnight_tp()
    {
        using namespace std::chrono;
        auto now = system_clock::now();
        time_t tnow = std::chrono::system_clock::to_time_t(now);
        tm date = spdlog::details::os::localtime(tnow);
        date.tm_hour = date.tm_min = date.tm_sec = 0;
        auto midnight = std::chrono::system_clock::from_time_t(std::mktime(&date));
        return system_clock::time_point(midnight + hours(24));
    }

    //Create filename for the form basename.YYYY-MM-DD.extension
    static std::string calc_filename(const std::string& basename, const std::string& extension)
    {
        std::tm tm = spdlog::details::os::localtime();
        details::fast_oss oss;
        oss << basename << '.';
        oss << tm.tm_year + 1900 << '-' << std::setw(2) << std::setfill('0') << tm.tm_mon + 1 << '-' << tm.tm_mday;
        oss << '.' << extension;
        return oss.str();
    }

    std::string _base_filename;
    std::string _extension;
    std::chrono::system_clock::time_point _midnight_tp;
    details::file_helper _file_helper;

};

typedef daily_file_sink<std::mutex> daily_file_sink_mt;
typedef daily_file_sink<details::null_mutex> daily_file_sink_st;
}
}
