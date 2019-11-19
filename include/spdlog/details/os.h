// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
#include <ctime> // std::time_t

namespace spdlog {
namespace details {
namespace os {

spdlog::log_clock::time_point now() SPDLOG_NOEXCEPT;

std::tm localtime(const std::time_t &time_tt) SPDLOG_NOEXCEPT;

std::tm localtime() SPDLOG_NOEXCEPT;

std::tm gmtime(const std::time_t &time_tt) SPDLOG_NOEXCEPT;

std::tm gmtime() SPDLOG_NOEXCEPT;

// eol definition
#if !defined(SPDLOG_EOL)
#ifdef _WIN32
#define SPDLOG_EOL "\r\n"
#else
#define SPDLOG_EOL "\n"
#endif
#endif

SPDLOG_CONSTEXPR static const char *default_eol = SPDLOG_EOL;

// folder separator
#ifdef _WIN32
static const char folder_sep = '\\';
#else
SPDLOG_CONSTEXPR static const char folder_sep = '/';
#endif

#ifdef SPDLOG_PREVENT_CHILD_FD
void prevent_child_fd(FILE *f);
#endif

// fopen_s on non windows for writing
bool fopen_s(FILE **fp, const filename_t &filename, const filename_t &mode);

// Remove filename. return 0 on success
int remove(const filename_t &filename) SPDLOG_NOEXCEPT;

// Remove file if exists. return 0 on success
// Note: Non atomic (might return failure to delete if concurrently deleted by other process/thread)
int remove_if_exists(const filename_t &filename) SPDLOG_NOEXCEPT;

int rename(const filename_t &filename1, const filename_t &filename2) SPDLOG_NOEXCEPT;

// Return if file exists.
bool path_exists(const filename_t &filename) SPDLOG_NOEXCEPT;

// Return file size according to open FILE* object
size_t filesize(FILE *f);

// Return utc offset in minutes or throw spdlog_ex on failure
int utc_minutes_offset(const std::tm &tm = details::os::localtime());

// Return current thread id as size_t
// It exists because the std::this_thread::get_id() is much slower(especially
// under VS 2013)
size_t _thread_id() SPDLOG_NOEXCEPT;

// Return current thread id as size_t (from thread local storage)
size_t thread_id() SPDLOG_NOEXCEPT;

// This is avoid msvc issue in sleep_for that happens if the clock changes.
// See https://github.com/gabime/spdlog/issues/609
void sleep_for_millis(int milliseconds) SPDLOG_NOEXCEPT;

std::string filename_to_str(const filename_t &filename);

int pid() SPDLOG_NOEXCEPT;

// Determine if the terminal supports colors
// Source: https://github.com/agauniyal/rang/
bool is_color_terminal() SPDLOG_NOEXCEPT;

// Determine if the terminal attached
// Source: https://github.com/agauniyal/rang/
bool in_terminal(FILE *file) SPDLOG_NOEXCEPT;

#if (defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT) || defined(SPDLOG_WCHAR_FILENAMES)) && defined(_WIN32)
void wstr_to_utf8buf(wstring_view_t wstr, memory_buf_t &target);
#endif

// Return directory name from given path or empty string
// "abc/file" => "abc"
// "abc/" => "abc"
// "abc" => ""
// "abc///" => "abc//"
filename_t dir_name(filename_t path);

// Create a dir from the given path.
// Return true if succeeded or if this dir already exists.
bool create_dir(filename_t path);

} // namespace os
} // namespace details
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "os-inl.h"
#endif
