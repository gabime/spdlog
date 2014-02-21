#pragma once

#include<string>
#include<chrono>
#include<functional>
#include <sstream>

#include "../level.h"
#include "../details/os.h"

namespace c11log {
namespace formatters {
typedef std::chrono::system_clock clock;
typedef clock::time_point time_point;
typedef std::function<std::string(const std::string& logger_name, const std::string&, level::level_enum, const time_point&)> format_fn;


std::string to_hex(const unsigned char* buf, std::size_t size);

class formatter {
public:
    formatter() {}
    virtual ~formatter() {}
    virtual void format_header(const std::string& logger_name, level::level_enum level, const time_point& tp, std::ostream& dest) = 0;
};


class default_formatter: public formatter {
public:
    // Format: [2013-12-29 01:04:42.900] [logger_name:Info] Message body
    void format_header(const std::string& logger_name, level::level_enum level, const time_point& tp, std::ostream& dest) override {
        _format_time(tp, dest);
        dest << " [" <<  logger_name << ":" << c11log::level::to_str(level) << "] ";
    }
private:
    void _format_time(const time_point& tp, std::ostream &dest);

};
} //namespace formatter
} //namespace c11log



inline void c11log::formatters::default_formatter::_format_time(const time_point& tp, std::ostream &dest)
{
    using namespace std::chrono;

    static thread_local c11log::formatters::time_point last_tp;
    static thread_local char timestamp_cache[64];


    if(duration_cast<milliseconds>(tp-last_tp).count() > 950) {
        auto tm = details::os::localtime(clock::to_time_t(tp));
        sprintf(timestamp_cache, "[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year + 1900,
                tm.tm_mon + 1,
                tm.tm_mday,
                tm.tm_hour,
                tm.tm_min,
                tm.tm_sec);
        last_tp = tp;
    }
    dest << timestamp_cache;
}
