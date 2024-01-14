// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <thread>

#include "spdlog/common.h"
#include "spdlog/details/os.h"

#ifdef __linux__
    #include <sys/syscall.h>  //Use gettid() syscall under linux to get thread id

#elif defined(_AIX)
    #include <pthread.h>  // for pthread_getthrds_np

#elif defined(__DragonFly__) || defined(__FreeBSD__)
    #include <pthread_np.h>  // for pthread_getthreadid_np

#elif defined(__NetBSD__)
    #include <lwp.h>  // for _lwp_self

#elif defined(__sun)
    #include <thread.h>  // for thr_self
#endif

#if defined __APPLE__
    #include <AvailabilityMacros.h>
#endif

#ifndef __has_feature           // Clang - feature checking macros.
    #define __has_feature(x) 0  // Compatibility with non-clang compilers.
#endif

// clang-format on
namespace spdlog {
namespace details {
namespace os {

spdlog::log_clock::time_point now() noexcept {
#if defined __linux__ && defined SPDLOG_CLOCK_COARSE
    timespec ts;
    ::clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    return std::chrono::time_point<log_clock, typename log_clock::duration>(
        std::chrono::duration_cast<typename log_clock::duration>(std::chrono::seconds(ts.tv_sec) +
                                                                 std::chrono::nanoseconds(ts.tv_nsec)));

#else
    return log_clock::now();
#endif
}
std::tm localtime(const std::time_t &time_tt) noexcept {
    std::tm tm;
    const auto *rv = ::localtime_r(&time_tt, &tm);
    return rv != nullptr ? tm : std::tm{};
}

std::tm localtime() noexcept {
    std::time_t now_t = ::time(nullptr);
    return localtime(now_t);
}

std::tm gmtime(const std::time_t &time_tt) noexcept {
    std::tm tm;
    ::gmtime_r(&time_tt, &tm);
    return tm;
}

std::tm gmtime() noexcept {
    std::time_t now_t = ::time(nullptr);
    return gmtime(now_t);
}

// fopen_s on non windows for writing
bool fopen_s(FILE **fp, const filename_t &filename, const filename_t &mode) {
#if defined(SPDLOG_PREVENT_CHILD_FD)
    const int mode_flag = mode == SPDLOG_FILENAME_T("ab") ? O_APPEND : O_TRUNC;
    const int fd = ::open((filename.c_str()), O_CREAT | O_WRONLY | O_CLOEXEC | mode_flag, mode_t(0644));
    if (fd == -1) {
        return true;
    }
    *fp = ::fdopen(fd, mode.c_str());
    if (*fp == nullptr) {
        ::close(fd);
    }
#else
    *fp = ::fopen((filename.c_str()), mode.c_str());
#endif
    return *fp == nullptr;
}

int remove(const filename_t &filename) noexcept { return std::remove(filename.c_str()); }

int remove_if_exists(const filename_t &filename) noexcept { return path_exists(filename) ? remove(filename) : 0; }

int rename(const filename_t &filename1, const filename_t &filename2) noexcept {
    return std::rename(filename1.c_str(), filename2.c_str());
}

// Return true if path exists (file or directory)
bool path_exists(const filename_t &filename) noexcept {
    struct stat buffer;
    return (::stat(filename.c_str(), &buffer) == 0);
}

// Return file size according to open FILE* object
size_t filesize(FILE *f) {
    if (f == nullptr) {
        throw_spdlog_ex("Failed getting file size. fd is null");
    }

    // OpenBSD and AIX doesn't compile with :: before the fileno(..)
#if defined(__OpenBSD__) || defined(_AIX)
    int fd = fileno(f);
#else
    int fd = ::fileno(f);
#endif
    // 64 bits(but not in osx, linux/musl or cygwin, where fstat64 is deprecated)
#if ((defined(__linux__) && defined(__GLIBC__)) || defined(__sun) || defined(_AIX)) && (defined(__LP64__) || defined(_LP64))
    struct stat64 st;
    if (::fstat64(fd, &st) == 0) {
        return static_cast<size_t>(st.st_size);
    }
#else  // other unix or linux 32 bits or cygwin
    struct stat st;
    if (::fstat(fd, &st) == 0) {
        return static_cast<size_t>(st.st_size);
    }
#endif
    throw_spdlog_ex("Failed getting file size from fd", errno);
    return 0;  // will not be reached.
}

// Return utc offset in minutes or throw spdlog_ex on failure
int utc_minutes_offset(const std::tm &tm) {
#if defined(sun) || defined(__sun) || defined(_AIX) || (defined(__NEWLIB__) && !defined(__TM_GMTOFF)) || \
    (!defined(_BSD_SOURCE) && !defined(_GNU_SOURCE))
    // 'tm_gmtoff' field is BSD extension and it's missing on SunOS/Solaris
    struct helper {
        static long int calculate_gmt_offset(const std::tm &localtm = details::os::localtime(),
                                             const std::tm &gmtm = details::os::gmtime()) {
            int local_year = localtm.tm_year + (1900 - 1);
            int gmt_year = gmtm.tm_year + (1900 - 1);

            long int days = (
                // difference in day of year
                localtm.tm_yday -
                gmtm.tm_yday

                // + intervening leap days
                + ((local_year >> 2) - (gmt_year >> 2)) - (local_year / 100 - gmt_year / 100) +
                ((local_year / 100 >> 2) - (gmt_year / 100 >> 2))

                // + difference in years * 365 */
                + static_cast<long int>(local_year - gmt_year) * 365);

            long int hours = (24 * days) + (localtm.tm_hour - gmtm.tm_hour);
            long int mins = (60 * hours) + (localtm.tm_min - gmtm.tm_min);
            long int secs = (60 * mins) + (localtm.tm_sec - gmtm.tm_sec);

            return secs;
        }
    };

    auto offset_seconds = helper::calculate_gmt_offset(tm);
#else
    auto offset_seconds = tm.tm_gmtoff;
#endif
    return static_cast<int>(offset_seconds / 60);
}

// Return current thread id as size_t
// It exists because the std::this_thread::get_id() is much slower(especially
// under VS 2013)
size_t _thread_id() noexcept {
#if defined(__linux__)
    #if defined(__ANDROID__) && defined(__ANDROID_API__) && (__ANDROID_API__ < 21)
        #define SYS_gettid __NR_gettid
    #endif
    return static_cast<size_t>(::syscall(SYS_gettid));
#elif defined(_AIX)
    struct __pthrdsinfo buf;
    int reg_size = 0;
    pthread_t pt = pthread_self();
    int retval = pthread_getthrds_np(&pt, PTHRDSINFO_QUERY_TID, &buf, sizeof(buf), NULL, &reg_size);
    int tid = (!retval) ? buf.__pi_tid : 0;
    return static_cast<size_t>(tid);
#elif defined(__DragonFly__) || defined(__FreeBSD__)
    return static_cast<size_t>(::pthread_getthreadid_np());
#elif defined(__NetBSD__)
    return static_cast<size_t>(::_lwp_self());
#elif defined(__OpenBSD__)
    return static_cast<size_t>(::getthrid());
#elif defined(__sun)
    return static_cast<size_t>(::thr_self());
#elif __APPLE__
    uint64_t tid;
    // There is no pthread_threadid_np prior to Mac OS X 10.6, and it is not supported on any PPC,
    // including 10.6.8 Rosetta. __POWERPC__ is Apple-specific define encompassing ppc and ppc64.
    #ifdef MAC_OS_X_VERSION_MAX_ALLOWED
    {
        #if (MAC_OS_X_VERSION_MAX_ALLOWED < 1060) || defined(__POWERPC__)
        tid = pthread_mach_thread_np(pthread_self());
        #elif MAC_OS_X_VERSION_MIN_REQUIRED < 1060
        if (&pthread_threadid_np) {
            pthread_threadid_np(nullptr, &tid);
        } else {
            tid = pthread_mach_thread_np(pthread_self());
        }
        #else
        pthread_threadid_np(nullptr, &tid);
        #endif
    }
    #else
    pthread_threadid_np(nullptr, &tid);
    #endif
    return static_cast<size_t>(tid);
#else  // Default to standard C++11 (other Unix)
    return static_cast<size_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
#endif
}

// Return current thread id as size_t (from thread local storage)
size_t thread_id() noexcept {
    // cache thread id in tls
    static thread_local const size_t tid = _thread_id();
    return tid;
}

void sleep_for_millis(unsigned int milliseconds) noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

std::string filename_to_str(const filename_t &filename) { return filename; }

int pid() noexcept { return static_cast<int>(::getpid()); }

// Determine if the terminal supports colors
// Based on: https://github.com/agauniyal/rang/
bool is_color_terminal() noexcept {
    static const bool result = []() {
        const char *env_colorterm_p = std::getenv("COLORTERM");
        if (env_colorterm_p != nullptr) {
            return true;
        }

        static constexpr std::array<const char *, 16> terms = {{"ansi", "color", "console", "cygwin", "gnome", "konsole", "kterm",
                                                                "linux", "msys", "putty", "rxvt", "screen", "vt100", "xterm",
                                                                "alacritty", "vt102"}};

        const char *env_term_p = std::getenv("TERM");
        if (env_term_p == nullptr) {
            return false;
        }

        return std::any_of(terms.begin(), terms.end(),
                           [&](const char *term) { return std::strstr(env_term_p, term) != nullptr; });
    }();

    return result;
}

// Determine if the terminal attached
// Source: https://github.com/agauniyal/rang/
bool in_terminal(FILE *file) noexcept { return ::isatty(fileno(file)) != 0; }

// return true on success
static bool mkdir_(const filename_t &path) { return ::mkdir(path.c_str(), mode_t(0755)) == 0; }

// create the given directory - and all directories leading to it
// return true on success or if the directory already exists
bool create_dir(const filename_t &path) {
    if (path_exists(path)) {
        return true;
    }

    if (path.empty()) {
        return false;
    }

    size_t search_offset = 0;
    do {
        auto token_pos = path.find_first_of(folder_seps_filename, search_offset);
        // treat the entire path as a folder if no folder separator not found
        if (token_pos == filename_t::npos) {
            token_pos = path.size();
        }

        auto subdir = path.substr(0, token_pos);

        if (!subdir.empty() && !path_exists(subdir) && !mkdir_(subdir)) {
            return false;  // return error if failed creating dir
        }
        search_offset = token_pos + 1;
    } while (search_offset < path.size());

    return true;
}

// Return directory name from given path or empty string
// "abc/file" => "abc"
// "abc/" => "abc"
// "abc" => ""
// "abc///" => "abc//"
filename_t dir_name(const filename_t &path) {
    auto pos = path.find_last_of(folder_seps_filename);
    return pos != filename_t::npos ? path.substr(0, pos) : filename_t{};
}

std::string getenv(const char *field) {
    char *buf = ::getenv(field);
    return buf != nullptr ? buf : std::string{};
}

// Do fsync by FILE handlerpointer
// Return true on success
bool fsync(FILE *fp) { return ::fsync(fileno(fp)) == 0; }

}  // namespace os
}  // namespace details
}  // namespace spdlog
