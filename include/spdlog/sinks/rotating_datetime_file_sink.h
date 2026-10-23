// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/os.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/base_sink.h>

#include <ctime>
#include <deque>
#include <mutex>
#include <string>
#include <tuple>
#include <utility>

SPDLOG_NAMESPACE_BEGIN
namespace sinks {

/*
 * Generator of archive file names for rotating_datetime_file_sink.
 * The timestamp is taken at the moment of rotation, so the sequence of
 * archives is ordered by the time each file was closed:
 *    "logs/app.log" => "logs/app-2026-10-23_10-56-25.log"
 * A custom calculator can be plugged in as the FileNameCalc template
 * argument, using the same signature as the daily/hourly sinks.
 */
struct datetime_archive_calculator {
    static filename_t calc_filename(const filename_t &base_filename, const tm &now_tm) {
        filename_t basename, ext;
        std::tie(basename, ext) = details::file_helper::split_by_extension(base_filename);
        return fmt_lib::format(
            SPDLOG_FMT_STRING(
                SPDLOG_FILENAME_T("{}-{:04d}-{:02d}-{:02d}_{:02d}-{:02d}-{:02d}{}")),
            basename, now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
            now_tm.tm_min, now_tm.tm_sec, ext);
    }
};

/*
 * Rotating file sink based on size, keeping the date encoded in the name of every
 * rotated file.
 *
 * Unlike rotating_file_sink (which renames log.txt -> log.1.txt -> log.2.txt ...),
 * this sink keeps writing to base_filename and renames it to a name produced by
 * FileNameCalc each time it rotates:
 *
 *    logs/app.log                       <- active file, always the same name
 *    logs/app.log => logs/app-2026-10-23_10-56-25.log
 *    logs/app.log                       <- reopened empty, keeps growing
 *
 * Keeping the active file name stable is what allows the sink to be resumed by a
 * new process: the constructor opens base_filename in append mode, so max_size
 * keeps being enforced across restarts (same as rotating_file_sink).
 *
 * If max_files > 0, retain only the last max_files archived files and delete the
 * previous ones. Note that old archived files from previous executions will not be
 * deleted by this class, rotation and deletion is only applied while the program is
 * running (same policy as the daily/hourly sinks). Use rotate_on_open to archive the
 * content left by the previous execution on startup.
 *
 * When max_files == 0 nothing is deleted, but the names of the most recent archives
 * (up to MaxFiles of them) are still remembered, so that a later set_max_files()
 * can prune them immediately.
 */
template <typename Mutex, typename FileNameCalc = datetime_archive_calculator>
class rotating_datetime_file_sink final : public base_sink<Mutex> {
public:
    static constexpr size_t MaxFiles = 200000;
    static constexpr size_t MaxArchiveNameTries = 1000;

    rotating_datetime_file_sink(filename_t base_filename,
                                std::size_t max_size,
                                std::size_t max_files = 0,
                                bool rotate_on_open = false,
                                const file_event_handlers &event_handlers = {},
                                std::size_t buffer_size = 0)
        : base_filename_(std::move(base_filename)),
          max_size_(max_size),
          max_files_(max_files),
          current_size_(0),
          file_helper_{event_handlers} {
        if (max_size == 0) {
            throw_spdlog_ex(
                "rotating_datetime_file_sink constructor: max_size arg cannot be zero");
        }
        if (max_files > MaxFiles) {
            throw_spdlog_ex(
                "rotating_datetime_file_sink constructor: max_files arg cannot exceed MaxFiles");
        }
        file_helper_.set_buffer_size(buffer_size);
        file_helper_.open(base_filename_, /*truncate=*/false);
        current_size_ = file_helper_.size();  // expensive. called only once
        if (rotate_on_open && current_size_ > 0) {
            rotate_(log_clock::now());
        }
    }

    // name of the currently written file (never an archived one)
    filename_t filename() {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return file_helper_.filename();
    }

    void rotate_now() {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        rotate_(log_clock::now());
    }

    void set_max_size(std::size_t max_size) {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        if (max_size == 0) {
            throw_spdlog_ex(
                "rotating_datetime_file_sink set_max_size: max_size arg cannot be zero");
        }
        max_size_ = max_size;
    }

    std::size_t get_max_size() {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return max_size_;
    }

    void set_max_files(std::size_t max_files) {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        if (max_files > MaxFiles) {
            throw_spdlog_ex(
                "rotating_datetime_file_sink set_max_files: max_files arg cannot exceed MaxFiles");
        }
        max_files_ = max_files;
        if (max_files_ == 0) {
            return;  // keep everything
        }
        // shrink immediately instead of waiting for the next rotation
        while (archived_filenames_.size() > max_files_) {
            prune_one_archived_();
        }
    }

    std::size_t get_max_files() {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return max_files_;
    }

    std::size_t get_current_size() {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return current_size_;
    }

    void set_buffer_size(std::size_t buffer_size) {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        // Reopen callbacks may append data, so refresh the size estimate.
        if (file_helper_.buffer_size() != buffer_size) {
            file_helper_.set_buffer_size(buffer_size);
            file_helper_.flush();
            current_size_ = file_helper_.size();
        }
    }

    std::size_t buffer_size() {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return file_helper_.buffer_size();
    }

protected:
    void sink_it_(const details::log_msg &msg) override {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        auto new_size = current_size_ + formatted.size();

        // rotate if the new estimated file size exceeds max size.
        // rotate only if the real size > 0 to better deal with full disk (see issue #2261).
        if (new_size > max_size_) {
            file_helper_.flush();
            if (file_helper_.size() > 0) {
                rotate_(msg.time);
                new_size = current_size_ + formatted.size();
            }
        }
        file_helper_.write(formatted);
        current_size_ = new_size;
    }

    void flush_() override { file_helper_.flush(); }

private:
    static tm now_tm_(log_clock::time_point tp) {
        time_t tnow = log_clock::to_time_t(tp);
        return details::os::localtime(tnow);
    }

    // Rename the active file to a fresh archive name and reopen an empty active file.
    void rotate_(log_clock::time_point tp) {
        using details::os::filename_to_str;

        file_helper_.close();
        filename_t archive = make_unique_archive_name_(tp);

        if (!rename_file_(base_filename_, archive)) {
            // if failed try again after a small delay. this is a workaround to a windows
            // issue, where very high rotation rates can cause the rename to fail with
            // permission denied (because of antivirus?).
            details::os::sleep_for_millis(100);
            if (!rename_file_(base_filename_, archive)) {
                // Keep the data in the current file: truncating here would silently drop
                // the messages that could not be archived, unlike rotating_file_sink
                // where truncating only starts a new size window.
                file_helper_.reopen(false);
                current_size_ = file_helper_.size();
                throw_spdlog_ex("rotating_datetime_file_sink: failed archiving " +
                                    filename_to_str(base_filename_) + " to " +
                                    filename_to_str(archive),
                                errno);
            }
        }

        file_helper_.reopen(true);
        current_size_ = file_helper_.size();  // after_open handlers may prepend data

        delete_old_(std::move(archive));
    }

    // Archive names are produced by FileNameCalc, which usually has a one second
    // resolution: rotating more than once within the same second would otherwise
    // reuse a name. os::rename() overwrites the target on POSIX (and fails on
    // windows), so a colliding name must be ruled out *before* renaming.
    filename_t make_unique_archive_name_(log_clock::time_point tp) {
        using details::os::path_exists;

        const tm now_tm = now_tm_(tp);
        const filename_t candidate = FileNameCalc::calc_filename(base_filename_, now_tm);
        if (candidate != base_filename_ && !path_exists(candidate)) {
            return candidate;
        }

        filename_t stem, ext;
        std::tie(stem, ext) = details::file_helper::split_by_extension(candidate);
        for (std::size_t i = 1; i < MaxArchiveNameTries; ++i) {
            filename_t unique = fmt_lib::format(
                SPDLOG_FMT_STRING(SPDLOG_FILENAME_T("{}-{:03d}{}")), stem, i, ext);
            if (unique != base_filename_ && !path_exists(unique)) {
                return unique;
            }
        }
        throw_spdlog_ex("rotating_datetime_file_sink: cannot generate a unique archive name for " +
                        details::os::filename_to_str(candidate));
    }

    // Unlike rotating_file_sink::rename_file_(), the target is never removed first: the
    // caller guarantees the archive name does not exist yet, and removing it would
    // destroy an already archived file if that guarantee were ever broken.
    bool rename_file_(const filename_t &src_filename, const filename_t &target_filename) {
        return details::os::rename(src_filename, target_filename) == 0;
    }

    void delete_old_(filename_t archive) {
        archived_filenames_.push_back(std::move(archive));
        if (max_files_ > 0) {
            while (archived_filenames_.size() > max_files_) {
                prune_one_archived_();
            }
        } else {
            // Pruning is disabled: keep a bounded history only, so that a later
            // set_max_files() can still act on the most recently archived files.
            while (archived_filenames_.size() > MaxFiles) {
                archived_filenames_.pop_front();
            }
        }
    }

    // Throw spdlog_ex on failure to delete the oldest archived file.
    void prune_one_archived_() {
        using details::os::filename_to_str;
        using details::os::remove_if_exists;

        filename_t old = std::move(archived_filenames_.front());
        archived_filenames_.pop_front();
        if (remove_if_exists(old) != 0) {
            archived_filenames_.push_front(std::move(old));  // restore the queue state
            throw_spdlog_ex("Failed removing archived file " + filename_to_str(old), errno);
        }
    }

    filename_t base_filename_;
    std::size_t max_size_;
    std::size_t max_files_;
    std::size_t current_size_;
    details::file_helper file_helper_;
    std::deque<filename_t> archived_filenames_;
};

using rotating_datetime_file_sink_mt = rotating_datetime_file_sink<std::mutex>;
using rotating_datetime_file_sink_st = rotating_datetime_file_sink<details::null_mutex>;

}  // namespace sinks

//
// factory functions
//
template <typename Factory = synchronous_factory>
inline std::shared_ptr<logger> rotating_datetime_logger_mt(
    const std::string &logger_name,
    const filename_t &filename,
    size_t max_file_size,
    size_t max_files = 0,
    bool rotate_on_open = false,
    const file_event_handlers &event_handlers = {},
    size_t buffer_size = 0) {
    return Factory::template create<sinks::rotating_datetime_file_sink_mt>(
        logger_name, filename, max_file_size, max_files, rotate_on_open, event_handlers,
        buffer_size);
}

template <typename Factory = synchronous_factory>
inline std::shared_ptr<logger> rotating_datetime_logger_st(
    const std::string &logger_name,
    const filename_t &filename,
    size_t max_file_size,
    size_t max_files = 0,
    bool rotate_on_open = false,
    const file_event_handlers &event_handlers = {},
    size_t buffer_size = 0) {
    return Factory::template create<sinks::rotating_datetime_file_sink_st>(
        logger_name, filename, max_file_size, max_files, rotate_on_open, event_handlers,
        buffer_size);
}
SPDLOG_NAMESPACE_END
