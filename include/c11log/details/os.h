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


inline bool operator==(const std::tm& tm1, const std::tm& tm2)
{
	return (tm1.tm_sec == tm2.tm_sec &&
		   tm1.tm_min == tm2.tm_min &&
		   tm1.tm_hour == tm2.tm_hour &&
		   tm1.tm_mday == tm2.tm_mday &&
		   tm1.tm_mon == tm2.tm_mon &&
		   tm1.tm_year == tm2.tm_year &&
		   tm1.tm_isdst == tm2.tm_isdst &&
		   tm1.tm_gmtoff == tm2.tm_gmtoff);
}

inline bool operator!=(const std::tm& tm1, const std::tm& tm2)
{
	return !(tm1==tm2);
}
