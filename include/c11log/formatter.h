#pragma once

#include <string>
#include <chrono>
#include <functional>

#include <iomanip>
#include <thread>
#include <cstring>
#include <sstream>
#include "common_types.h"
#include "details/os.h"


namespace c11log
{
namespace formatters
{

class formatter
{
public:
    virtual void format_header(const std::string& logger_name, level::level_enum level, const log_clock::time_point& tp, std::ostream& output) = 0;
};


class default_formatter: public formatter
{
public:
    // Format: [2013-12-29 01:04:42.900] [logger_name:Info] Message body
    void format_header(const std::string& logger_name, level::level_enum level, const log_clock::time_point& tp, std::ostream& output) override
    {
        _format_time(tp, output);
        if(!logger_name.empty())
            output << " [" <<  logger_name << ':' << c11log::level::to_str(level) << "] ";
        else
            output << " [" << c11log::level::to_str(level) << "] ";

    }
private:
    void _format_time(const log_clock::time_point& tp, std::ostream &output);

};


} //namespace formatter
} //namespace c11log

// Format datetime like this: [2014-03-14 17:15:22]
inline void c11log::formatters::default_formatter::_format_time(const log_clock::time_point& tp, std::ostream &output)
{
    using namespace c11log::details::os;
    using namespace std::chrono;

#ifdef _WIN32 //VS2013 doesn't support yet thread_local keyword
    __declspec(thread) static char s_cache_str[64];
    __declspec(thread) static size_t s_cache_size;
    __declspec(thread) static std::time_t s_cache_time_t = 0;
#else
    thread_local static std::string s_cache_timestr;
    thread_local static std::time_t s_cache_time_t = 0;
#endif

    //Cache every second
    std::time_t tp_time_t = log_clock::to_time_t(tp);
    if(tp_time_t != s_cache_time_t)
    {
        auto tm_now = details::os::localtime(tp_time_t);
        std::ostringstream time_oss;
        time_oss.fill('0');
        time_oss << '[' << tm_now.tm_year + 1900 << '-';
        time_oss.width(2);
        time_oss << tm_now.tm_mon + 1 << '-';
        time_oss.width(2);
        time_oss << tm_now.tm_mday << ' ';
        time_oss.width(2);
        time_oss << tm_now.tm_hour << ':';
        time_oss.width(2);
        time_oss << tm_now.tm_min << ':';
        time_oss.width(2);
        time_oss << tm_now.tm_sec << ']';
        //Cache the resulted string and its size
        s_cache_time_t = tp_time_t;
        //const std::string &s = time_oss.str_ref();        
		s_cache_timestr = time_oss.str();


    }
    output << s_cache_timestr;
}
