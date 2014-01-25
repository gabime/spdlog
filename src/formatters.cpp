#include "stdafx.h"
#include "c11log/formatters/formatters.h"
#include "c11log/level.h"

void c11log::formatters::format_time(const c11log::formatters::timepoint& tp, std::ostream &dest)
{
    std::tm tm = details::os::localtime(std::chrono::system_clock::to_time_t(tp));
    //get ms
    auto duration = tp.time_since_epoch();
    int millis = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000);
    //std::put_time(&tm, "[ %Y-%m-%d %H:%M:%S ]") - seems too slow
    char buf[64];
    sprintf(buf, "[%d-%02d-%02d %02d:%02d:%02d.%03d]",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec, millis);
    dest << buf;
}

void c11log::formatters::format_time(std::ostream& dest)
{
    return format_time(std::chrono::system_clock::now(), dest);
}


static const char _hex_chars[17] = "0123456789ABCDEF";

std::string c11log::formatters::to_hex(const unsigned char* buf, std::size_t size)
{
    std::ostringstream oss;

    for (std::size_t i = 0; i < size; i++) {
        oss << _hex_chars[buf[i] >> 4];
        oss << _hex_chars[buf[i] & 0x0F];
    }
    return oss.str();
}