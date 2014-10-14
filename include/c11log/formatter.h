#pragma once

#include <string>
#include <chrono>

#include <iomanip>
#include <thread>
#include <cstring>
#include <sstream>

#include "common.h"
#include "details/os.h"
#include "details/log_msg.h"
#include "details/fast_oss.h"


namespace c11log
{
namespace formatters
{

class formatter
{
public:
    virtual void format(details::log_msg& msg) = 0;
};


class default_formatter: public formatter
{
public:
    // Format: [2013-12-29 01:04:42.900] [logger_name:Info] Message body
    void format(details::log_msg& msg) override
    {
        details::fast_oss oss;
        _format_time(msg, oss);

        if(!msg.logger_name.empty())
            oss << " [" <<  msg.logger_name << ':' << c11log::level::to_str(msg.level) << "] ";
        else
            oss << " [" << c11log::level::to_str(msg.level) << "] ";

        oss << msg.raw << details::os::eol();
        msg.formatted = oss.str();
    }
private:
    void _format_time(const details::log_msg& msg, std::ostream &output);

};


} //namespace formatter
} //namespace c11log

// Format datetime like this: [2014-03-14 17:15:22]
inline void c11log::formatters::default_formatter::_format_time(const details::log_msg& msg, std::ostream &output)
{
    output.fill('0');
    output << '[' << msg.tm_time.tm_year + 1900 << '-';
    output.width(2);
    output << msg.tm_time.tm_mon + 1 << '-';
    output << msg.tm_time.tm_mday << ' ';
    output << msg.tm_time.tm_hour << ':';
    output << msg.tm_time.tm_min << ':';
    output << msg.tm_time.tm_sec << ']';
}

