#include "stdafx.h"

#include "c11log/details/os.h"

namespace c11log {
namespace details {
namespace os {
std::tm localtime(const std::time_t &time_t)
{

    std::tm tm;
#ifdef _MSC_VER
    localtime_s(&tm, &time_t);
#else  
	localtime_r(&time_t, &tm);
#endif
    return tm;
}

std::tm localtime()
{
    std::time_t now_t = time(0);
    return localtime(now_t);
}
}
}
}
