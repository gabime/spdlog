#include "stdafx.h"

#include "c11log/details/os.h"

namespace c11log {
namespace details {
namespace os {
std::tm localtime(const std::time_t &time_t)
{
#ifdef _MSC_VER
    std::tm tm;
    localtime_s(&tm, &time_t);
    return tm;
#endif
}

std::tm localtime()
{
    std::time_t now_t = time(0);
    return localtime(now_t);
}
}
}
}