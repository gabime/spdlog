//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
#pragma once

#include "spdlog/common.h"

#include <cstdio>
#include <ctime>
#include <functional>
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX //prevent windows redefining min/max
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <process.h> //  _get_pid support
#include <io.h> // _get_osfhandle and _isatty support

#ifdef __MINGW32__
#include <share.h>
#endif

#else // unix

#include <unistd.h>
#include <sys/uio.h>

#ifdef __linux__
#include <sys/syscall.h> //Use gettid() syscall under linux to get thread id
#include <linux/limits.h> // for PIPE_BUF

#elif __FreeBSD__
#include <sys/thr.h> //Use thr_self() syscall under FreeBSD to get thread id
#include <limits.h>
#endif

// Max number of iovec structures
#ifndef IOV_MAX
#define IOV_MAX UIO_MAXIOV
#endif

#endif //unix

// File open modes
#if defined(_WIN32) && !defined(__MINGW32__)

#define SPDLOG_O_APPEND _O_APPEND
#define SPDLOG_O_CREATE _O_CREAT
#define SPDLOG_O_BINARY _O_BINARY
#define SPDLOG_O_WRONLY _O_WRONLY
#define SPDLOG_O_RDONLY _O_RDONLY
#define SPDLOG_O_RDWR _O_RDWR
#define SPDLOG_O_TRUNCATE _O_TRUNC

#else // unix and MinGW

#define SPDLOG_O_APPEND O_APPEND
#define SPDLOG_O_CREATE O_CREAT

#ifdef O_BINARY
#define SPDLOG_O_BINARY O_BINARY
#else
#define SPDLOG_O_BINARY 0
#endif

#define SPDLOG_O_WRONLY O_WRONLY
#define SPDLOG_O_RDONLY O_RDONLY
#define SPDLOG_O_RDWR O_RDWR
#define SPDLOG_O_TRUNCATE O_TRUNC

#endif

#ifndef __has_feature       // Clang - feature checking macros.
#define __has_feature(x) 0  // Compatibility with non-clang compilers.
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

inline int fileno_s(FILE* file)
{
#ifdef _WIN32
    return _fileno(file);
#else
    return fileno(file);
#endif
}

inline void prevent_child_fd(int fd)
{
#ifdef _WIN32
    auto file_handle = (HANDLE)_get_osfhandle(fd);
    if (!::SetHandleInformation(file_handle, HANDLE_FLAG_INHERIT, 0))
        throw spdlog_ex("SetHandleInformation failed", errno);
#else
    if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
        throw spdlog_ex("fcntl with FD_CLOEXEC failed", errno);
#endif
}

inline void prevent_child_fd(FILE *f)
{
    prevent_child_fd(fileno_s(f));
}

//fopen_s on non windows for writing
inline int fopen_s(FILE** fp, const filename_t& filename, const filename_t& mode)
{
#ifdef _WIN32
#ifdef SPDLOG_WCHAR_FILENAMES
    *fp = _wfsopen((filename.c_str()), (mode.c_str()), _SH_DENYWR);
#else
    *fp = _fsopen((filename.c_str()), (mode.c_str()), _SH_DENYWR);
#endif
#else //unix
    *fp = fopen((filename.c_str()), (mode.c_str()));
#endif

#ifdef SPDLOG_PREVENT_CHILD_FD
    if (*fp != nullptr)
        prevent_child_fd(*fp);
#endif
    return *fp == nullptr;
}

inline int fopen_s(int* fp, const filename_t& filename, const int& mode)
{
#ifdef _WIN32
#ifdef SPDLOG_WCHAR_FILENAMES
    _wsopen_s(fp, (filename.c_str()), mode, _SH_DENYNO, (_S_IREAD | _S_IWRITE));
#else
    _sopen_s(fp, (filename.c_str()), mode, _SH_DENYNO, (_S_IREAD | _S_IWRITE));
#endif
#else //unix
    *fp = open((filename.c_str()), mode, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
#endif

#ifdef SPDLOG_PREVENT_CHILD_FD
    if (*fp != nullptr)
        prevent_child_fd(*fp);
#endif

    // Return true if something went wrong; false - if there was no errors and
    // we get new file descriptor
    return (fp != nullptr) && (*fp < 0) ? true : false;
}

inline void write_s(int fd, const char* msg, std::size_t size)
{
#ifdef _WIN32
    // TODO: what is max size of data for atomic write on Windows?
    int result = _write(fd, msg, size);
    if ( (result < 0) || (static_cast<std::size_t>(result) != size) )
    {
        throw spdlog_ex("Write to file failed", errno);
    }

    // Immediately flush data to file
    _commit(fd);

#else // unix
    // PIPE_BUF - max message length for atomic write
    if (size <= static_cast<std::size_t>(PIPE_BUF))
    {
        if (write(fd, msg, size) != static_cast<ssize_t>(size))
        {
            throw spdlog_ex("Write to file failed", errno);
        }
    }
    else
    {
        // Max message length should be less than SSIZE_MAX (see writev() docs)
        std::size_t msg_size = std::min(size, static_cast<std::size_t>(SSIZE_MAX));

        // Calculate how many iovec structures we need
        const std::size_t iovec_num = std::min((size / PIPE_BUF) + 1, static_cast<std::size_t>(IOV_MAX));
        std::vector<iovec> iovectors(iovec_num);

        // Warning: using const_cast so we could move along the message buffer
        char* msg_ptr = const_cast<char*>(msg);
        for (std::size_t i = 0; i < iovectors.size(); ++i)
        {
            iovectors[i].iov_base = static_cast<void*>(msg_ptr);

            std::size_t msg_part_length = 0;
            if (i < iovectors.size() - 1)
            {
                msg_part_length = PIPE_BUF;
            }
            else
            {
                // Calc length of the last iovec
                std::size_t previos_parts_length = PIPE_BUF * i;
                msg_part_length = std::min(msg_size - previos_parts_length, static_cast<std::size_t>(PIPE_BUF));
            }

            iovectors[i].iov_len = msg_part_length;
            msg_ptr += msg_part_length;
        }

        if (writev(fd, &iovectors[0], iovec_num) != static_cast<ssize_t>(msg_size))
        {
            throw spdlog::spdlog_ex("Failed writing to file", errno);
        }
    }
#endif
}

inline void close_s(int fd)
{
#ifdef _WIN32
    _close(fd);
#else // unix
    close(fd);
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
    return (stat(filename.c_str(), &buffer) == 0);
#endif
}

//Return file size according to open file descriptor
inline size_t filesize(int fd)
{
#ifdef _WIN32
#if _WIN64 //64 bits
    struct _stat64 st;
    if (_fstat64(fd, &st) == 0)
        return st.st_size;

#else // windows 32 bits
    long ret = _filelength(fd);
    if (ret >= 0)
        return static_cast<size_t>(ret);
#endif

#else // unix
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

//Return file size according to open FILE* object
inline size_t filesize(FILE *f)
{
    if (f == nullptr)
        throw spdlog_ex("Failed getting file size. fd is null");

    return filesize(fileno_s(f));
}

inline size_t filesize(const filename_t& filename)
{
    int fd = -1;
    const int open_tries = 5;
    const int open_interval = 10;
    for (int tries = 0; tries < open_tries; ++tries)
    {
        if (!fopen_s(&fd, filename, SPDLOG_O_RDONLY))
        {
            size_t file_size = filesize(fd);
            close_s(fd);

            return file_size;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(open_interval));
    }

    throw spdlog_ex("Failed opening file for filesize command", errno);
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
inline size_t _thread_id()
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
#elif __APPLE__
    uint64_t tid;
    pthread_threadid_np(nullptr, &tid);
    return static_cast<size_t>(tid);
#else //Default to standard C++11 (other Unix)
    return static_cast<size_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
#endif
}

//Return current thread id as size_t (from thread local storage)
inline size_t thread_id()
{
#if defined(_MSC_VER) && (_MSC_VER < 1900) || defined(__clang__) && !__has_feature(cxx_thread_local)
    return _thread_id();
#else
    static thread_local const size_t tid = _thread_id();
    return tid;
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

inline std::string errno_to_string(char[256], char* res)
{
    return std::string(res);
}

inline std::string errno_to_string(char buf[256], int res)
{
    if (res == 0)
    {
        return std::string(buf);
    }
    else
    {
        return "Unknown error";
    }
}

// Return errno string (thread safe)
inline std::string errno_str(int err_num)
{
    char buf[256];
    SPDLOG_CONSTEXPR auto buf_size = sizeof(buf);

#ifdef _WIN32
    if (strerror_s(buf, buf_size, err_num) == 0)
        return std::string(buf);
    else
        return "Unknown error";

#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(ANDROID) || defined(__SUNPRO_CC) || \
      ((_POSIX_C_SOURCE >= 200112L) && ! defined(_GNU_SOURCE)) // posix version

    if (strerror_r(err_num, buf, buf_size) == 0)
        return std::string(buf);
    else
        return "Unknown error";

#else  // gnu version (might not use the given buf, so its retval pointer must be used)
    auto err = strerror_r(err_num, buf, buf_size); // let compiler choose type
    return errno_to_string(buf, err); // use overloading to select correct stringify function
#endif
}

inline int pid()
{
#ifdef _WIN32
    return ::_getpid();
#else
    return static_cast<int>(::getpid());
#endif
}


// Detrmine if the terminal supports colors
// Source: https://github.com/agauniyal/rang/
inline bool is_color_terminal()
{
#ifdef _WIN32
    return true;
#else
    static constexpr const char* Terms[] =
    {
        "ansi", "color", "console", "cygwin", "gnome", "konsole", "kterm",
        "linux", "msys", "putty", "rxvt", "screen", "vt100", "xterm"
    };

    const char *env_p = std::getenv("TERM");
    if (env_p == nullptr)
    {
        return false;
    }

    static const bool result = std::any_of(
                                   std::begin(Terms), std::end(Terms), [&](const char* term)
    {
        return std::strstr(env_p, term) != nullptr;
    });
    return result;
#endif
}


// Detrmine if the terminal attached
// Source: https://github.com/agauniyal/rang/
inline bool in_terminal(FILE* file)
{
#ifdef _WIN32
    return _isatty(fileno_s(file)) ? true : false;
#else
    return isatty(fileno_s(file)) ? true : false;
#endif
}
} //os
} //details
} //spdlog
