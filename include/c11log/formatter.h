#pragma once

#include <string>
#include <chrono>
#include <functional>
#include <sstream>
#include <iomanip>

#include "common_types.h"
#include "details/os.h"

namespace c11log {
namespace formatters {

typedef std::function<std::string(const std::string& logger_name, const std::string&, level::level_enum, const c11log::log_clock::time_point&)> format_fn;


std::string to_hex(const unsigned char* buf, std::size_t size);

class formatter {
public:
    formatter() {}
    virtual ~formatter() {}
    virtual void format_header(const std::string& logger_name, level::level_enum level, const log_clock::time_point& tp, std::ostream& dest) = 0;
};


class default_formatter: public formatter {
public:
    // Format: [2013-12-29 01:04:42.900] [logger_name:Info] Message body
    void format_header(const std::string& logger_name, level::level_enum level, const log_clock::time_point& tp, std::ostream& dest) override {
        _format_time(tp, dest);
        dest << " [" <<  logger_name << ":" << c11log::level::to_str(level) << "] ";
    }
private:
    void _format_time(const log_clock::time_point& tp, std::ostream &dest);

};
} //namespace formatter
} //namespace c11log



inline void c11log::formatters::default_formatter::_format_time(const log_clock::time_point& tp, std::ostream &dest)
{
    auto tm = details::os::localtime(log_clock::to_time_t(tp));
    char buff[64];
    int size = snprintf(buff, sizeof(buff), "[%d-%02d-%02d %02d:%02d:%02d]",
                        tm.tm_year + 1900,
                        tm.tm_mon + 1,
                        tm.tm_mday,
                        tm.tm_hour,
                        tm.tm_min,
                        tm.tm_sec);

    dest.write(buff, size);
}
