#pragma once

#include<string>
#include<chrono>
#include<functional>
#include <sstream>

#include "../level.h"
#include "../details/os.h"

namespace c11log {
namespace formatters {
typedef std::chrono::system_clock::time_point timepoint;
typedef std::function<std::string(const std::string& logger_name, const std::string&, level::level_enum, const timepoint&)> format_fn;
void format_time(const timepoint& tp, std::ostream &dest);
void format_time(std::ostream &dest);
std::string to_hex(const unsigned char* buf, std::size_t size);

class formatter {
public:
    formatter() {}
    virtual ~formatter() {}
    virtual void format_header(const std::string& logger_name, level::level_enum level, const timepoint& tp, std::ostream& dest) = 0;
};


class default_formatter: public formatter {
public:
    // Format: [2013-12-29 01:04:42.900] [logger_name:Info] Message body
    void format_header(const std::string& logger_name, level::level_enum level, const timepoint& tp, std::ostream& dest) override
    {
        format_time(tp, dest);
        dest << " [" <<  logger_name << ":" << c11log::level::to_str(level) << "] ";
    }

};
} //namespace formatter
} //namespace c11log
