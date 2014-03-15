#pragma once

#include <string>
#include <chrono>
#include <functional>
#include <sstream>
#include <iomanip>
#include <thread>
#include <cstdlib>

#include "common_types.h"
#include "details/os.h"
#include "details/fast_oss.h"

namespace c11log
{
namespace formatters
{

typedef std::function<std::string(const std::string& logger_name, const std::string&, level::level_enum, const c11log::log_clock::time_point&)> format_fn;


class formatter
{
public:
    formatter() {}
    virtual ~formatter() {}
    virtual void format_header(const std::string& logger_name, level::level_enum level, const log_clock::time_point& tp, std::ostream& dest) = 0;
};


class default_formatter: public formatter
{
public:
    // Format: [2013-12-29 01:04:42.900] [logger_name:Info] Message body
    void format_header(const std::string& logger_name, level::level_enum level, const log_clock::time_point& tp, std::ostream& dest) override
    {
        _format_time(tp, dest);
        if(!logger_name.empty())
            dest << " [" <<  logger_name << ':' << c11log::level::to_str(level) << "] ";
        else
            dest << " [" << c11log::level::to_str(level) << "] ";

    }
private:
    void _format_time(const log_clock::time_point& tp, std::ostream &dest);

};
} //namespace formatter
} //namespace c11log


// Format datetime like this: [2014-03-14 17:15:22]
inline void c11log::formatters::default_formatter::_format_time(const log_clock::time_point& tp, std::ostream &dest)
{
	using namespace c11log::details::os;
	using namespace std::chrono;

#ifdef _MSC_VER
    __declspec(thread) static std::tm s_last_tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0};
    __declspec(thread) static details::fast_oss s_time_oss;
#else

	thread_local static details::fast_oss s_time_oss;
	thread_local static std::time_t s_cache_time_t = 0;
#endif


	std::time_t tp_time_t = log_clock::to_time_t(tp);


	//Cache every second
	if(tp_time_t != s_cache_time_t)
    {		
		auto tm_now = details::os::localtime(tp_time_t);
		s_time_oss.reset_str();
		s_time_oss.fill('0');
		s_time_oss << '[' << tm_now.tm_year + 1900 << '-';
		s_time_oss.width(2);
		s_time_oss << tm_now.tm_mon + 1 << '-';
		s_time_oss.width(2);
		s_time_oss << tm_now.tm_mday << ' ';
		s_time_oss.width(2);
		s_time_oss << tm_now.tm_hour << ':';
		s_time_oss.width(2);
		s_time_oss << tm_now.tm_min << ':';
		s_time_oss.width(2);
		s_time_oss << tm_now.tm_sec << ']';		
		s_cache_time_t = tp_time_t;
    }
	const std::string &s = s_time_oss.str_ref();
	dest.write(s.c_str(), s.size());
}
