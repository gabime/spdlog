// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/os.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/fmt/chrono.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/base_sink.h>

#if defined(_WIN32)
#include <spdlog/details/windows_include.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace spdlog {
namespace sinks {

/*
 * Generator of daily log file names in format basename.YYYY-MM-DD.ext
 */
struct daily_filename_calculator {
    // Create filename for the form basename.YYYY-MM-DD
    static filename_t calc_filename(const filename_t &filename, const tm &now_tm) {
        filename_t basename, ext;
        std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
        return fmt_lib::format(SPDLOG_FMT_STRING(SPDLOG_FILENAME_T("{}_{:04d}-{:02d}-{:02d}{}")),
                               basename, now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday,
                               ext);
    }
};

/*
 * Generator of daily log file names with strftime format.
 * Usages:
 *    auto sink =
 * std::make_shared<spdlog::sinks::daily_file_format_sink_mt>("myapp-%Y-%m-%d:%H:%M:%S.log", hour,
 * minute);" auto logger = spdlog::daily_logger_format_mt("loggername, "myapp-%Y-%m-%d:%X.log",
 * hour,  minute)"
 *
 */
struct daily_filename_format_calculator {
    static filename_t calc_filename(const filename_t &file_path, const tm &now_tm) {
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
        std::wstringstream stream;
#else
        std::stringstream stream;
#endif
        stream << std::put_time(&now_tm, file_path.c_str());
        return stream.str();
    }
};

/*
 * Rotating file sink based on date.
 * If truncate != false , the created file will be truncated.
 * If max_files > 0, retain only the last max_files and delete previous.
 * Cleanup scans the target directory and removes old files matching this sink naming pattern.
 * Rotation and deletion is applied while the program is running.
 */
template <typename Mutex, typename FileNameCalc = daily_filename_calculator>
class daily_file_sink final : public base_sink<Mutex> {
public:
    // create daily file sink which rotates on given time
    daily_file_sink(filename_t base_filename,
                    int rotation_hour,
                    int rotation_minute,
                    bool truncate = false,
                    uint16_t max_files = 0,
                    const file_event_handlers &event_handlers = {})
        : base_filename_(std::move(base_filename)),
          rotation_h_(rotation_hour),
          rotation_m_(rotation_minute),
          file_helper_{event_handlers},
          truncate_(truncate),
          max_files_(max_files) {
        if (rotation_hour < 0 || rotation_hour > 23 || rotation_minute < 0 ||
            rotation_minute > 59) {
            throw_spdlog_ex("daily_file_sink: Invalid rotation time in ctor");
        }

        auto now = log_clock::now();
        const auto new_filename = FileNameCalc::calc_filename(base_filename_, now_tm(now));
        file_helper_.open(new_filename, truncate_);
        rotation_tp_ = next_rotation_tp_();
    }

    filename_t filename() {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return file_helper_.filename();
    }

protected:
    void sink_it_(const details::log_msg &msg) override {
        auto time = msg.time;
        bool should_rotate = time >= rotation_tp_;
        if (should_rotate) {
            const auto new_filename = FileNameCalc::calc_filename(base_filename_, now_tm(time));
            file_helper_.open(new_filename, truncate_);
            rotation_tp_ = next_rotation_tp_();
        }
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        file_helper_.write(formatted);

        // Do the cleaning only at the end because it might throw on failure.
        if (should_rotate && max_files_ > 0) {
            delete_old_();
        }
    }

    void flush_() override { file_helper_.flush(); }

private:
    static bool starts_with_(const filename_t &value, const filename_t &prefix) {
        return value.size() >= prefix.size() &&
               std::equal(prefix.begin(), prefix.end(), value.begin());
    }

    // Normalize path separators to '/' so comparisons are reliable regardless of
    // whether the caller used '\' or '/' (both are valid on Windows).
    static filename_t normalize_seps_(filename_t path) {
#if defined(_WIN32)
        std::replace(path.begin(), path.end(),
                     static_cast<filename_t::value_type>('\\'),
                     static_cast<filename_t::value_type>('/'));
#endif
        return path;
    }

    static bool date_like_suffix_(const filename_t &suffix) {
        // Validate that suffix matches the daily_filename_calculator pattern: _YYYY-MM-DD
        // Expected format: _4digits-2digits-2digits (case insensitive for hexadecimal won't apply here)
        // Examples valid: _2026-04-22
        // Examples invalid: _123, _v1_2, _latest, _12-34-5, etc.
        
        if (suffix.size() < 11) {  // Minimum: _YYYY-MM-DD = 11 chars
            return false;
        }

        const auto c0 = static_cast<filename_t::value_type>('0');
        const auto c9 = static_cast<filename_t::value_type>('9');
        const auto dash = static_cast<filename_t::value_type>('-');
        const auto underscore = static_cast<filename_t::value_type>('_');

        // Must start with underscore or hyphen (depending on calculator variant)
        size_t pos = 0;
        if (suffix[0] != underscore && suffix[0] != dash) {
            return false;
        }
        ++pos;

        // Expect exactly 4 digits (YYYY)
        for (int i = 0; i < 4; ++i, ++pos) {
            if (pos >= suffix.size() || suffix[pos] < c0 || suffix[pos] > c9) {
                return false;
            }
        }

        // Expect dash
        if (pos >= suffix.size() || suffix[pos] != dash) {
            return false;
        }
        ++pos;

        // Expect exactly 2 digits (MM)
        for (int i = 0; i < 2; ++i, ++pos) {
            if (pos >= suffix.size() || suffix[pos] < c0 || suffix[pos] > c9) {
                return false;
            }
        }

        // Expect dash
        if (pos >= suffix.size() || suffix[pos] != dash) {
            return false;
        }
        ++pos;

        // Expect exactly 2 digits (DD)
        for (int i = 0; i < 2; ++i, ++pos) {
            if (pos >= suffix.size() || suffix[pos] < c0 || suffix[pos] > c9) {
                return false;
            }
        }

        // All characters consumed should be the date pattern
        // Allow trailing characters (for format calculators that append more)
        return true;
    }

    static filename_t join_path_(const filename_t &dir, const filename_t &basename) {
        if (dir.empty()) {
            return basename;
        }

        filename_t result = dir;
        // Check against ALL valid separators (on Windows both '\' and '/' are valid).
        // folder_seps_filename contains all valid separator characters for the platform.
        const filename_t seps(details::os::folder_seps_filename);
        if (seps.find(result.back()) == filename_t::npos) {
            result.push_back(seps[0]);
        }
        result += basename;
        return result;
    }

    bool is_matching_daily_file_(const filename_t &filename) const {
        filename_t base_name_no_ext;
        filename_t base_ext;
        std::tie(base_name_no_ext, base_ext) = details::file_helper::split_by_extension(base_filename_);
        base_name_no_ext = normalize_seps_(base_name_no_ext);

        filename_t candidate_no_ext;
        filename_t candidate_ext;
        std::tie(candidate_no_ext, candidate_ext) = details::file_helper::split_by_extension(filename);
        candidate_no_ext = normalize_seps_(candidate_no_ext);

        if (candidate_ext != base_ext || !starts_with_(candidate_no_ext, base_name_no_ext)) {
            return false;
        }

        return date_like_suffix_(candidate_no_ext.substr(base_name_no_ext.size()));
    }

    std::vector<filename_t> scan_matching_filenames_() const {
        std::vector<filename_t> matching_filenames;
        const filename_t dir_path = details::os::dir_name(base_filename_);

#if defined(_WIN32)
        filename_t search_pattern = dir_path.empty() ? SPDLOG_FILENAME_T("*")
                                                     : join_path_(dir_path, SPDLOG_FILENAME_T("*"));

#if defined(SPDLOG_WCHAR_FILENAMES)
        WIN32_FIND_DATAW find_data;
        HANDLE find_handle = ::FindFirstFileW(search_pattern.c_str(), &find_data);
#else
        WIN32_FIND_DATAA find_data;
        HANDLE find_handle = ::FindFirstFileA(search_pattern.c_str(), &find_data);
#endif
        if (find_handle == INVALID_HANDLE_VALUE) {
            return matching_filenames;
        }

        do {
            if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                continue;
            }

            filename_t filename = find_data.cFileName;
            const filename_t full_path = join_path_(dir_path, filename);
            if (is_matching_daily_file_(full_path)) {
                matching_filenames.push_back(normalize_seps_(full_path));
            }
        } while (
#if defined(SPDLOG_WCHAR_FILENAMES)
            ::FindNextFileW(find_handle, &find_data)
#else
            ::FindNextFileA(find_handle, &find_data)
#endif
        );
        ::FindClose(find_handle);
#else
        const filename_t scan_path = dir_path.empty() ? SPDLOG_FILENAME_T(".") : dir_path;
        DIR *dir = ::opendir(scan_path.c_str());
        if (dir == nullptr) {
            return matching_filenames;
        }

        struct dirent *entry = nullptr;
        while ((entry = ::readdir(dir)) != nullptr) {
            if (entry->d_name[0] == '.' &&
                (entry->d_name[1] == '\0' || (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) {
                continue;
            }

            const filename_t filename = entry->d_name;
            const filename_t full_path = join_path_(dir_path, filename);

            struct stat st;
            if (::stat(full_path.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) {
                continue;
            }

            if (is_matching_daily_file_(full_path)) {
                matching_filenames.push_back(normalize_seps_(full_path));
            }
        }
        (void)::closedir(dir);
#endif

        return matching_filenames;
    }

    tm now_tm(log_clock::time_point tp) {
        time_t tnow = log_clock::to_time_t(tp);
        return spdlog::details::os::localtime(tnow);
    }

    log_clock::time_point next_rotation_tp_() {
        auto now = log_clock::now();
        tm date = now_tm(now);
        date.tm_hour = rotation_h_;
        date.tm_min = rotation_m_;
        date.tm_sec = 0;
        auto rotation_time = log_clock::from_time_t(std::mktime(&date));
        if (rotation_time > now) {
            return rotation_time;
        }
        return {rotation_time + std::chrono::hours(24)};
    }

    // Delete the file N rotations ago.
    // Throw spdlog_ex on failure to delete the old file.
    void delete_old_() {
        using details::os::filename_to_str;
        using details::os::remove_if_exists;

        auto matching_filenames = scan_matching_filenames_();
        const filename_t current_file = normalize_seps_(file_helper_.filename());

        matching_filenames.erase(
            std::remove(matching_filenames.begin(), matching_filenames.end(), current_file),
            matching_filenames.end());

        const size_t files_to_keep_excluding_current =
            max_files_ > 0 ? static_cast<size_t>(max_files_ - 1) : 0;

        if (matching_filenames.size() <= files_to_keep_excluding_current) {
            return;
        }

        std::sort(matching_filenames.begin(), matching_filenames.end());

        const size_t files_to_delete =
            matching_filenames.size() - files_to_keep_excluding_current;

        for (size_t i = 0; i < files_to_delete; ++i) {
            const filename_t &old_filename = matching_filenames[i];
            const bool ok = remove_if_exists(old_filename) == 0;
            if (!ok) {
                throw_spdlog_ex("Failed removing daily file " + filename_to_str(old_filename),
                                errno);
            }
        }
    }

    filename_t base_filename_;
    int rotation_h_;
    int rotation_m_;
    log_clock::time_point rotation_tp_;
    details::file_helper file_helper_;
    bool truncate_;
    uint16_t max_files_;
};

using daily_file_sink_mt = daily_file_sink<std::mutex>;
using daily_file_sink_st = daily_file_sink<details::null_mutex>;
using daily_file_format_sink_mt = daily_file_sink<std::mutex, daily_filename_format_calculator>;
using daily_file_format_sink_st =
    daily_file_sink<details::null_mutex, daily_filename_format_calculator>;

}  // namespace sinks

//
// factory functions
//
template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> daily_logger_mt(const std::string &logger_name,
                                               const filename_t &filename,
                                               int hour = 0,
                                               int minute = 0,
                                               bool truncate = false,
                                               uint16_t max_files = 0,
                                               const file_event_handlers &event_handlers = {}) {
    return Factory::template create<sinks::daily_file_sink_mt>(logger_name, filename, hour, minute,
                                                               truncate, max_files, event_handlers);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> daily_logger_format_mt(
    const std::string &logger_name,
    const filename_t &filename,
    int hour = 0,
    int minute = 0,
    bool truncate = false,
    uint16_t max_files = 0,
    const file_event_handlers &event_handlers = {}) {
    return Factory::template create<sinks::daily_file_format_sink_mt>(
        logger_name, filename, hour, minute, truncate, max_files, event_handlers);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> daily_logger_st(const std::string &logger_name,
                                               const filename_t &filename,
                                               int hour = 0,
                                               int minute = 0,
                                               bool truncate = false,
                                               uint16_t max_files = 0,
                                               const file_event_handlers &event_handlers = {}) {
    return Factory::template create<sinks::daily_file_sink_st>(logger_name, filename, hour, minute,
                                                               truncate, max_files, event_handlers);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> daily_logger_format_st(
    const std::string &logger_name,
    const filename_t &filename,
    int hour = 0,
    int minute = 0,
    bool truncate = false,
    uint16_t max_files = 0,
    const file_event_handlers &event_handlers = {}) {
    return Factory::template create<sinks::daily_file_format_sink_st>(
        logger_name, filename, hour, minute, truncate, max_files, event_handlers);
}
}  // namespace spdlog
