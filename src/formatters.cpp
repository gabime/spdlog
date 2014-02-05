#include "stdafx.h"
#include <memory.h>

#include "c11log/formatters/formatters.h"
#include "c11log/level.h"




static thread_local c11log::formatters::time_point last_tp;
static thread_local char timestamp_cache[64];

void c11log::formatters::format_time(const time_point& tp, std::ostream &dest)
{

    // Cache timestamp string of last second
    using namespace std::chrono;
	if(duration_cast<seconds>(tp-last_tp).count() >= 1)
    {
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

void c11log::formatters::format_time(std::ostream& dest)
{
    return format_time(c11log::formatters::clock::now(), dest);
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
