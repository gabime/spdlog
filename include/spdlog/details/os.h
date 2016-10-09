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
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>


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

#include <sys/types.h>
#include <io.h>

#elif __linux__

#include <sys/syscall.h> //Use gettid() syscall under linux to get thread id
#include <unistd.h>
#include <chrono>

#elif __FreeBSD__
#include <sys/thr.h> //Use thr_self() syscall under FreeBSD to get thread id

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
#else //common linux/unix all have the stat system call
    struct stat buffer;
    return (stat (filename.c_str(), &buffer) == 0);
#endif
}




//Return file size according to open FILE* object
inline size_t filesize(FILE *f)
{
    if (f == nullptr)
        throw spdlog_ex("Failed getting file size. fd is null");
#ifdef _WIN32
    int fd = _fileno(f);
#if _WIN64 //64 bits
    struct _stat64 st;
    if (_fstat64(fd, &st) == 0)
        return st.st_size;

#else //windows 32 bits
    long ret = _filelength(fd);
    if (ret >= 0)
        return static_cast<size_t>(ret);
#endif

#else // unix
    int fd = fileno(f);
    //64 bits(but not in osx, where fstat64 is deprecated)
#if !defined(__FreeBSD__) && !defined(__APPLE__) && (defined(__x86_64__) || defined(__ppc64__))
    struct stat64 st;
    if (fstat64(fd, &st) == 0)
        return static_cast<size_t>(st.st_size);
#else // unix 32 bits or osx
    struct stat st;
    if (fstat(fd, &st) == 0)
        return static_cast<size_t>(st.st_size);
#endif
#endif
    throw spdlog_ex("Failed getting file size from fd", errno);
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
        throw spdlog::spdlog_ex("Failed getting timezone info. ", errno);

    int offset = -tzinfo.Bias;
    if (tm.tm_isdst)
        offset -= tzinfo.DaylightBias;
    else
        offset -= tzinfo.StandardBias;
    return offset;
#else

#if defined(sun) || defined(__sun)
    // 'tm_gmtoff' field is BSD extension and it's missing on SunOS/Solaris
    struct helper
    {
        static long int calculate_gmt_offset(const std::tm & localtm = details::os::localtime(), const std::tm & gmtm = details::os::gmtime())
        {
            int local_year = localtm.tm_year + (1900 - 1);
            int gmt_year = gmtm.tm_year + (1900 - 1);

            long int days = (
                                // difference in day of year
                                localtm.tm_yday - gmtm.tm_yday

                                // + intervening leap days
                                + ((local_year >> 2) - (gmt_year >> 2))
                                - (local_year / 100 - gmt_year / 100)
                                + ((local_year / 100 >> 2) - (gmt_year / 100 >> 2))

                                // + difference in years * 365 */
                                + (long int)(local_year - gmt_year) * 365
                            );

            long int hours = (24 * days) + (localtm.tm_hour - gmtm.tm_hour);
            long int mins = (60 * hours) + (localtm.tm_min - gmtm.tm_min);
            long int secs = (60 * mins) + (localtm.tm_sec - gmtm.tm_sec);

            return secs;
        }
    };

    long int offset_seconds = helper::calculate_gmt_offset(tm);
#else
    long int offset_seconds = tm.tm_gmtoff;
#endif

    return static_cast<int>(offset_seconds / 60);
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
#elif __FreeBSD__
    long tid;
    thr_self(&tid);
    return static_cast<size_t>(tid);
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


// Return errno string (thread safe)
inline std::string errno_str(int err_num)
{
    char buf[256];
    SPDLOG_CONSTEXPR auto buf_size = sizeof(buf);

#ifdef _WIN32
    if(strerror_s(buf, buf_size, err_num) == 0)
        return std::string(buf);
    else
        return "Unkown error";

#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(ANDROID) || \
      ((_POSIX_C_SOURCE >= 200112L) && ! _GNU_SOURCE) // posix version

    if (strerror_r(err_num, buf, buf_size) == 0)
        return std::string(buf);
    else
        return "Unkown error";

#else  // gnu version (might not use the given buf, so its retval pointer must be used)
    return std::string(strerror_r(err_num, buf, buf_size));
#endif
}

} //os
} //details
} //spdlog
