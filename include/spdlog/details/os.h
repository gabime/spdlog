// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <ctime>  // std::time_t
#include <tuple>

#include "../common.h"
#include "../filename_t.h"

SPDLOG_NAMESPACE_BEGIN
namespace details {
namespace os {

SPDLOG_API log_clock::time_point now() noexcept;

SPDLOG_API std::tm localtime(const std::time_t &time_tt) noexcept;

SPDLOG_API std::tm localtime() noexcept;

SPDLOG_API std::tm gmtime(const std::time_t &time_tt) noexcept;

SPDLOG_API std::tm gmtime() noexcept;

// eol definition and folder separator for the current os
#ifdef _WIN32
constexpr static const char *default_eol = "\r\n";
#else
constexpr static const char *default_eol = "\n";
#endif

// fopen_s on non windows for writing
SPDLOG_API bool fopen_s(FILE **fp, const filename_t &filename, const filename_t &mode);

// Return file size according to open FILE* object
SPDLOG_API size_t filesize(FILE *f);

// Return utc offset in minutes (0 on failure to compute offset)
SPDLOG_API int utc_minutes_offset(const std::tm &tm = details::os::localtime());

// Return current thread id as size_t
// It exists because the std::this_thread::get_id() is much slower(especially
// under VS 2013)
SPDLOG_API size_t _thread_id() noexcept;

// Return current thread id as size_t (from thread local storage)
SPDLOG_API size_t thread_id() noexcept;

// This is avoid msvc issue in sleep_for that happens if the clock changes.
// See https://github.com/gabime/spdlog/issues/609
SPDLOG_API void sleep_for_millis(unsigned int milliseconds) noexcept;

// Return pid
SPDLOG_API int pid() noexcept;

// Determine if the terminal supports colors
// Source: https://github.com/agauniyal/rang/
SPDLOG_API bool is_color_terminal() noexcept;

// Determine if the terminal attached
// Source: https://github.com/agauniyal/rang/
SPDLOG_API bool in_terminal(FILE *file) noexcept;

#if (defined _WIN32)
SPDLOG_API void wstr_to_utf8buf(wstring_view_t wstr, memory_buf_t &target);
SPDLOG_API void utf8_to_wstrbuf(string_view_t str, wmemory_buf_t &target);
#endif

// non thread safe, cross platform getenv/getenv_s
// return empty string if field not found
SPDLOG_API std::string getenv(const char *field);

// Do fsync by FILE objectpointer.
// Return true on success.
SPDLOG_API bool fsync(FILE *fp);

// Do non-locking fwrite if possible by the os or use the regular locking fwrite
// Return true on success.
SPDLOG_API bool fwrite_bytes(const void *ptr, const size_t n_bytes, FILE *fp);

//
// std::filesystem wrapper functions
//

// Return directory name from given path or empty string
// "abc/file" => "abc"
// "abc/" => "abc"
// "abc" => ""
SPDLOG_API filename_t dir_name(const filename_t &path);

// Create a dir from the given path.
// Return true if succeeded or if this dir already exists.
SPDLOG_API bool create_dir(const filename_t &path);

// Remove filename. return true on success
SPDLOG_API bool remove(const filename_t &filename);

// Remove file if exists. return 0 on success
// Note: Non atomic (might return failure to delete if concurrently deleted by other process/thread)
SPDLOG_API bool remove_if_exists(const filename_t &filename);

// Rename file. return true on success
SPDLOG_API bool rename(const filename_t &filename1, const filename_t &filename2) noexcept;

// Return if file exists.
SPDLOG_API bool path_exists(const filename_t &filename) noexcept;

// Return file path and its extension:
//
// "mylog.txt" => ("mylog", ".txt")
// "mylog" => ("mylog", "")
// "mylog." => ("mylog.", "")
// "/dir1/dir2/mylog.txt" => ("/dir1/dir2/mylog", ".txt")
//
// the starting dot in filenames is ignored (hidden files):
//
// ".mylog" => (".mylog". "")
// "my_folder/.mylog" => ("my_folder/.mylog", "")
// "my_folder/.mylog.txt" => ("my_folder/.mylog", ".txt")
SPDLOG_API std::tuple<filename_t, filename_t> split_by_extension(const filename_t &fname);

// Try tp convert filename to string. Return "??" if failed
SPDLOG_API std::string filename_to_str(const filename_t &filename);

}  // namespace os
}  // namespace details
SPDLOG_NAMESPACE_END
