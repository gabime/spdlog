#pragma once
#include<string>
#include<cstdio>
#include<ctime>

namespace c11log {
namespace details {
namespace os {
std::tm localtime(const std::time_t &time_tt);
std::tm localtime();

}
}
}


inline std::tm c11log::details::os::localtime(const std::time_t &time_tt)
{

    std::tm tm;
#ifdef _MSC_VER
    localtime_s(&tm, &time_tt);
#else
    localtime_r(&time_tt, &tm);
#endif
    return tm;
}

inline std::tm c11log::details::os::localtime()
{
    std::time_t now_t = time(0);
    return localtime(now_t);
}

// Take care of snprintf in visual studio
#ifdef _MSC_VER
#define snprintf _snprintf
#endif
