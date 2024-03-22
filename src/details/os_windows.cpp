// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef _WIN32
    #error "_WIN32 only source file"
#endif

// clang-format off
#include "spdlog/details/windows_include.h"
// clang-format on

#include <fileapi.h>  // for FlushFileBuffers
#include <io.h>       // for _get_osfhandle, _isatty, _fileno
#include <process.h>  // for _get_pid
#include <sys/stat.h>

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

#ifdef __MINGW32__
    #include <share.h>
#endif

#if defined(SPDLOG_WCHAR_FILENAMES)
    #include <cassert>
    #include <limits>
#endif

#include <direct.h>  // for _mkdir/_wmkdir

// clang-format on
namespace spdlog {
namespace details {
namespace os {
spdlog::log_clock::time_point now() noexcept { return log_clock::now(); }

std::tm localtime(const std::time_t &time_tt) noexcept {
    std::tm tm;
    const auto rv = ::localtime_s(&tm, &time_tt);
    return rv == 0 ? tm : std::tm{};
}

std::tm localtime() noexcept {
    std::time_t now_t = ::time(nullptr);
    return localtime(now_t);
}

std::tm gmtime(const std::time_t &time_tt) noexcept {
    std::tm tm;
    ::gmtime_s(&tm, &time_tt);
    return tm;
}

std::tm gmtime() noexcept {
    std::time_t now_t = ::time(nullptr);
    return gmtime(now_t);
}

bool fopen_s(FILE **fp, const filename_t &filename, const filename_t &mode) {
#ifdef SPDLOG_WCHAR_FILENAMES
    *fp = ::_wfsopen((filename.c_str()), mode.c_str(), _SH_DENYNO);
#else
    *fp = ::_fsopen((filename.c_str()), mode.c_str(), _SH_DENYNO);
#endif
#if defined(SPDLOG_PREVENT_CHILD_FD)
    if (*fp != nullptr) {
        auto file_handle = reinterpret_cast<HANDLE>(_get_osfhandle(::_fileno(*fp)));
        if (!::SetHandleInformation(file_handle, HANDLE_FLAG_INHERIT, 0)) {
            ::fclose(*fp);
            *fp = nullptr;
        }
    }
#endif
    return *fp == nullptr;
}

int remove(const filename_t &filename) noexcept {
#if defined(SPDLOG_WCHAR_FILENAMES)
    return ::_wremove(filename.c_str());
#else
    return std::remove(filename.c_str());
#endif
}

int remove_if_exists(const filename_t &filename) noexcept { return path_exists(filename) ? remove(filename) : 0; }

int rename(const filename_t &filename1, const filename_t &filename2) noexcept {
#if defined(SPDLOG_WCHAR_FILENAMES)
    return ::_wrename(filename1.c_str(), filename2.c_str());
#else
    return std::rename(filename1.c_str(), filename2.c_str());
#endif
}

// Return true if path exists (file or directory)
bool path_exists(const filename_t &filename) noexcept {
    struct _stat buffer;
#ifdef SPDLOG_WCHAR_FILENAMES
    return (::_wstat(filename.c_str(), &buffer) == 0);
#else
    return (::_stat(filename.c_str(), &buffer) == 0);
#endif
}

#ifdef _MSC_VER
// avoid warning about unreachable statement at the end of filesize()
    #pragma warning(push)
    #pragma warning(disable : 4702)
#endif

// Return file size according to open FILE* object
size_t filesize(FILE *f) {
    if (f == nullptr) {
        throw_spdlog_ex("Failed getting file size. fd is null");
    }
    int fd = ::_fileno(f);
#if defined(_WIN64)  // 64 bits
    __int64 ret = ::_filelengthi64(fd);
    if (ret >= 0) {
        return static_cast<size_t>(ret);
    }

#else  // windows 32 bits
    long ret = ::_filelength(fd);
    if (ret >= 0) {
        return static_cast<size_t>(ret);
    }
#endif
    throw_spdlog_ex("Failed getting file size from fd", errno);
    return 0;  // will not be reached.
}

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

// Return utc offset in minutes or throw spdlog_ex on failure
int utc_minutes_offset(const std::tm &tm) {
#if _WIN32_WINNT < _WIN32_WINNT_WS08
    TIME_ZONE_INFORMATION tzinfo;
    auto rv = ::GetTimeZoneInformation(&tzinfo);
#else
    DYNAMIC_TIME_ZONE_INFORMATION tzinfo;
    auto rv = ::GetDynamicTimeZoneInformation(&tzinfo);
#endif
    if (rv == TIME_ZONE_ID_INVALID) throw_spdlog_ex("Failed getting timezone info. ", errno);

    int offset = -tzinfo.Bias;
    if (tm.tm_isdst) {
        offset -= tzinfo.DaylightBias;
    } else {
        offset -= tzinfo.StandardBias;
    }
    return offset;
}

// Return current thread id as size_t
// It exists because the std::this_thread::get_id() is much slower(especially
// under VS 2013)
size_t _thread_id() noexcept { return static_cast<size_t>(::GetCurrentThreadId()); }

// Return current thread id as size_t (from thread local storage)
size_t thread_id() noexcept {
    // cache thread id in tls
    static thread_local const size_t tid = _thread_id();
    return tid;
}

// This is avoid msvc issue in sleep_for that happens if the clock changes.
// See https://github.com/gabime/spdlog/issues/609
void sleep_for_millis(unsigned int milliseconds) noexcept { ::Sleep(milliseconds); }

// wchar support for windows file names (SPDLOG_WCHAR_FILENAMES must be defined)
#if defined(SPDLOG_WCHAR_FILENAMES)
std::string filename_to_str(const filename_t &filename) {
    memory_buf_t buf;
    wstr_to_utf8buf(filename, buf);
    return SPDLOG_BUF_TO_STRING(buf);
}
#else
std::string filename_to_str(const filename_t &filename) { return filename; }
#endif

int pid() noexcept { return static_cast<int>(::GetCurrentProcessId()); }

bool is_color_terminal() noexcept { return true; }

// Determine if the terminal attached
bool in_terminal(FILE *file) noexcept { return ::_isatty(_fileno(file)) != 0; }

#if defined(SPDLOG_WCHAR_FILENAMES)
void wstr_to_utf8buf(wstring_view_t wstr, memory_buf_t &target) {
    if (wstr.size() > static_cast<size_t>((std::numeric_limits<int>::max)()) / 4 - 1) {
        throw_spdlog_ex("UTF-16 string is too big to be converted to UTF-8");
    }

    int wstr_size = static_cast<int>(wstr.size());
    if (wstr_size == 0) {
        target.resize(0);
        return;
    }

    int result_size = static_cast<int>(target.capacity());
    if ((wstr_size + 1) * 4 > result_size) {
        result_size = ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr_size, NULL, 0, NULL, NULL);
    }

    if (result_size > 0) {
        target.resize(result_size);
        result_size = ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr_size, target.data(), result_size, NULL, NULL);

        if (result_size > 0) {
            target.resize(result_size);
            return;
        }
    }

    throw_spdlog_ex(fmt_lib::format("WideCharToMultiByte failed. Last error: {}", ::GetLastError()));
}

void utf8_to_wstrbuf(string_view_t str, wmemory_buf_t &target) {
    if (str.size() > static_cast<size_t>((std::numeric_limits<int>::max)()) - 1) {
        throw_spdlog_ex("UTF-8 string is too big to be converted to UTF-16");
    }

    int str_size = static_cast<int>(str.size());
    if (str_size == 0) {
        target.resize(0);
        return;
    }

    // find the size to allocate for the result buffer
    int result_size = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.data(), str_size, NULL, 0);

    if (result_size > 0) {
        target.resize(result_size);
        result_size = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.data(), str_size, target.data(), result_size);
        if (result_size > 0) {
            assert(result_size == target.size());
            return;
        }
    }

    throw_spdlog_ex(fmt_lib::format("MultiByteToWideChar failed. Last error: {}", ::GetLastError()));
}
#endif  // defined(SPDLOG_WCHAR_FILENAMES)

// return true on success
static bool mkdir_(const filename_t &path) {
#
#ifdef SPDLOG_WCHAR_FILENAMES
    return ::_wmkdir(path.c_str()) == 0;
#else
    return ::_mkdir(path.c_str()) == 0;
#endif
}

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
#if defined(_MSC_VER)
    #if defined(__cplusplus_winrt)
    return std::string{};  // not supported under uwp
    #else
    size_t len = 0;
    char buf[128];
    bool ok = ::getenv_s(&len, buf, sizeof(buf), field) == 0;
    return ok ? buf : std::string{};
    #endif
#else  // revert to getenv
    char *buf = ::getenv(field);
    return buf != nullptr ? buf : std::string{};
#endif
}

// Do fsync by FILE handlerpointer
// Return true on success
bool fsync(FILE *fp) { return FlushFileBuffers(reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(fp)))) != 0; }

}  // namespace os
}  // namespace details
}  // namespace spdlog
