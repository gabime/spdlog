//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
#pragma once

#include<string>
#include<cstdio>
#include<ctime>
#include<list>

#ifdef _WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <windows.h>

#ifdef __MINGW32__
#include <share.h>
#endif

#elif __linux__
#include <sys/syscall.h> //Use gettid() syscall under linux to get thread id
#include <sys/stat.h>
#include <unistd.h>
#else
#include <thread>
#endif

#include "../common.h"

namespace spdlog
{
namespace details
{
namespace os
{

inline spdlog::log_clock::time_point now()
{

#if defined __linux__ && defined SPDLOG_CLOCK_COARSE
    timespec ts;
    ::clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    return std::chrono::time_point<log_clock, typename log_clock::duration>(
               std::chrono::duration_cast<typename log_clock::duration>(
                   std::chrono::seconds(ts.tv_sec) + std::chrono::nanoseconds(ts.tv_nsec)));

#else
    return log_clock::now();
#endif

}
inline std::tm localtime(const std::time_t &time_tt)
{

#ifdef _WIN32
    std::tm tm;
    localtime_s(&tm, &time_tt);
#else
    std::tm tm;
    localtime_r(&time_tt, &tm);
#endif
    return tm;
}

inline std::tm localtime()
{
    std::time_t now_t = time(nullptr);
    return localtime(now_t);
}


inline std::tm gmtime(const std::time_t &time_tt)
{

#ifdef _WIN32
    std::tm tm;
    gmtime_s(&tm, &time_tt);
#else
    std::tm tm;
    gmtime_r(&time_tt, &tm);
#endif
    return tm;
}

inline std::tm gmtime()
{
    std::time_t now_t = time(nullptr);
    return gmtime(now_t);
}
inline bool operator==(const std::tm& tm1, const std::tm& tm2)
{
    return (tm1.tm_sec == tm2.tm_sec &&
            tm1.tm_min == tm2.tm_min &&
            tm1.tm_hour == tm2.tm_hour &&
            tm1.tm_mday == tm2.tm_mday &&
            tm1.tm_mon == tm2.tm_mon &&
            tm1.tm_year == tm2.tm_year &&
            tm1.tm_isdst == tm2.tm_isdst);
}

inline bool operator!=(const std::tm& tm1, const std::tm& tm2)
{
    return !(tm1 == tm2);
}

#ifdef _WIN32
inline const char* eol()
{
    return "\r\n";
}
#else
constexpr inline const char* eol()
{
    return "\n";
}
#endif

#ifdef _WIN32
inline unsigned short eol_size()
{
    return 2;
}
#else
constexpr inline unsigned short eol_size()
{
    return 1;
}
#endif

//Return true and list if filename contains relative
inline bool dir_check(const std::string& filename, std::list<std::string>& dirs)
{
#ifdef __linux__
    std::string directory;
    std::string::size_type index, previndex = 0, size;

    index = filename.find("/", previndex);
    if(!index)
        dirs.push_back("/");

    do {
        if(index - previndex == 0) 
            directory = filename.substr(previndex, index - previndex + 1);
        else 
            directory = filename.substr(previndex, index - previndex);

        if(directory != "/")
            dirs.push_back(directory);

        previndex = index + 1;
        index = filename.find("/", previndex);
    }while(index != std::string::npos);

    return true;

#endif

}

//Create directories by referring to list 
//Return 0 if success 
inline bool create_dirs(std::list<std::string>& dirs)
{
#ifdef __linux__
    std::string origin_path;
    std::list<std::string>::iterator li;

    int r;

    origin_path = get_current_dir_name();
    for(li = dirs.begin(); li != dirs.end(); ++li) {
        r = mkdir((*li).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if(r && errno != EEXIST)
            return r;

        r = chdir((*li).c_str());
        if(r) return r;
    }

    r = chdir(origin_path.c_str());
    if(r) return r;

    return 0;

#endif

}

//fopen_s on non windows for writing
inline int fopen_s(FILE** fp, const std::string& filename, const char* mode)
{
#ifdef _WIN32
    *fp = _fsopen((filename.c_str()), mode, _SH_DENYWR);
    return *fp == nullptr;
#else
    std::list<std::string> dirs;

    if(dir_check(filename, dirs)) 
    {
        if(create_dirs(dirs))
            return true;
    }

    *fp = fopen((filename.c_str()), mode);
    return *fp == nullptr;

#endif

}


//Return if file exists
inline bool file_exists(const std::string& filename)
{
#ifdef _WIN32
    auto attribs = GetFileAttributesA(filename.c_str());
    return (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY));
#elif __linux__
    struct stat buffer;
    return (stat (filename.c_str(), &buffer) == 0);
#else  
    auto *file = fopen(filename.c_str(), "r");
    if (file != nullptr)
    {
        fclose(file);
        return true;
    }
    return false;

#endif

}


//Return utc offset in minutes or -1 on failure
inline int utc_minutes_offset(const std::tm& tm = details::os::localtime())
{

#ifdef _WIN32
    (void)tm; // avoid unused param warning
#if _WIN32_WINNT < _WIN32_WINNT_WS08
    TIME_ZONE_INFORMATION tzinfo;
    auto rv = GetTimeZoneInformation(&tzinfo);
#else
    DYNAMIC_TIME_ZONE_INFORMATION tzinfo;
    auto rv = GetDynamicTimeZoneInformation(&tzinfo);
#endif
    if (!rv)
        return -1;
    return -1 * (tzinfo.Bias + tzinfo.DaylightBias);
#else
    return static_cast<int>(tm.tm_gmtoff / 60);
#endif
}

//Return current thread id as size_t
//It exists because the std::this_thread::get_id() is much slower(espcially under VS 2013)
inline size_t thread_id()
{
#ifdef _WIN32
    return  static_cast<size_t>(::GetCurrentThreadId());
#elif __linux__
# if defined(__ANDROID__) && defined(__ANDROID_API__) && (__ANDROID_API__ < 21)
#  define SYS_gettid __NR_gettid
# endif
    return  static_cast<size_t>(syscall(SYS_gettid));
#else //Default to standard C++11 (OSX and other Unix)
    return static_cast<size_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
#endif

}

} //os
} //details
} //spdlog


