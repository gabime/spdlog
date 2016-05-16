//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
#pragma once

#include <spdlog/common.h>

#include <cstdio>
#include <ctime>
#include <functional>
#include <string>

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX //prevent windows redefining min/max
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __MINGW32__
#include <share.h>
#endif

#elif __linux__
#include <sys/syscall.h> //Use gettid() syscall under linux to get thread id
#include <sys/stat.h>
#include <unistd.h>
#include <chrono>
#else
#include <thread>
#endif

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

// eol definition
#if !defined (SPDLOG_EOL)
#ifdef _WIN32
#define SPDLOG_EOL "\r\n"
#else
#define SPDLOG_EOL "\n"
#endif
#endif

SPDLOG_CONSTEXPR static const char* eol = SPDLOG_EOL;
SPDLOG_CONSTEXPR static int eol_size = sizeof(SPDLOG_EOL) - 1;



//fopen_s on non windows for writing
inline int fopen_s(FILE** fp, const filename_t& filename, const filename_t& mode)
{
#ifdef _WIN32
#ifdef SPDLOG_WCHAR_FILENAMES
    *fp = _wfsopen((filename.c_str()), mode.c_str(), _SH_DENYWR);
#else
    *fp = _fsopen((filename.c_str()), mode.c_str(), _SH_DENYWR);
#endif
    return *fp == nullptr;
#else
    *fp = fopen((filename.c_str()), mode.c_str());
    return *fp == nullptr;
#endif
}

inline int remove(const filename_t &filename)
{
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
    return _wremove(filename.c_str());
#else
    return std::remove(filename.c_str());
#endif
}

inline int rename(const filename_t& filename1, const filename_t& filename2)
{
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
    return _wrename(filename1.c_str(), filename2.c_str());
#else
    return std::rename(filename1.c_str(), filename2.c_str());
#endif
}


//Return if file exists
inline bool file_exists(const filename_t& filename)
{
#ifdef _WIN32
#ifdef SPDLOG_WCHAR_FILENAMES
    auto attribs = GetFileAttributesW(filename.c_str());
#else
    auto attribs = GetFileAttributesA(filename.c_str());
#endif
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

//Return utc offset in minutes or throw spdlog_ex on failure
inline int utc_minutes_offset(const std::tm& tm = details::os::localtime())
{

#ifdef _WIN32
#if _WIN32_WINNT < _WIN32_WINNT_WS08
    TIME_ZONE_INFORMATION tzinfo;
    auto rv = GetTimeZoneInformation(&tzinfo);
#else
    DYNAMIC_TIME_ZONE_INFORMATION tzinfo;
    auto rv = GetDynamicTimeZoneInformation(&tzinfo);
#endif
    if (rv == TIME_ZONE_ID_INVALID)
        throw spdlog::spdlog_ex("Failed getting timezone info. Last error: " + std::to_string(GetLastError()));

    int offset = -tzinfo.Bias;
    if (tm.tm_isdst)
        offset -= tzinfo.DaylightBias;
    else
        offset -= tzinfo.StandardBias;
    return offset;
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


// wchar support for windows file names (SPDLOG_WCHAR_FILENAMES must be defined)
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
#define SPDLOG_FILENAME_T(s) L ## s
inline std::string filename_to_str(const filename_t& filename)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> c;
    return c.to_bytes(filename);
}
#else
#define SPDLOG_FILENAME_T(s) s
inline std::string filename_to_str(const filename_t& filename)
{
    return filename;
}
#endif

} //os
} //details
} //spdlog


