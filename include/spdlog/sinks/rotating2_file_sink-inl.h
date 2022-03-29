// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#    include <spdlog/sinks/rotating2_file_sink.h>
#endif

#include <spdlog/common.h>
#include <spdlog/details/os.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/fmt/fmt.h>

#include <cerrno>
#include <chrono>
#include <ctime>
#include <mutex>
#include <string>
#include <tuple>

namespace spdlog {
namespace sinks {

template<typename Mutex>
SPDLOG_INLINE rotating2_file_sink<Mutex>::rotating2_file_sink(filename_t base_filename, std::size_t max_size, bool rotate_on_open,
    pattern_time_type time_type, const file_event_handlers &event_handlers)
    : base_filename_(std::move(base_filename))
    , max_size_(max_size)
    , time_type_(time_type)
    , file_helper_{event_handlers}
{
    if (max_size == 0)
    {
        throw_spdlog_ex("rotating sink constructor: max_size arg cannot be zero");
    }

    file_helper_.open(base_filename_);
    current_size_ = file_helper_.size(); // expensive. called only once
    if (rotate_on_open && current_size_ > 0)
    {
        rotate_();
        current_size_ = 0;
    }
}

// Create filename for the form basename.YYYY-MM-DD-hh-mm-ss
template<typename Mutex>
filename_t rotating2_file_sink<Mutex>::calc_filename(const filename_t &filename, const tm &now_tm)
{
    filename_t basename, ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
    return fmt_lib::format(SPDLOG_FILENAME_T("{}_{:04d}-{:02d}-{:02d}-{:02d}-{:02d}-{:02d}{}"), basename, now_tm.tm_year + 1900,
        now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec, ext);
}

template<typename Mutex>
SPDLOG_INLINE filename_t rotating2_file_sink<Mutex>::filename()
{
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    return file_helper_.filename();
}

template<typename Mutex>
SPDLOG_INLINE void rotating2_file_sink<Mutex>::sink_it_(const details::log_msg &msg)
{
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    auto new_size = current_size_ + formatted.size();

    // rotate if the new estimated file size exceeds max size.
    // rotate only if the real size > 0 to better deal with full disk (see issue #2261).
    // we only check the real size when new_size > max_size_ because it is relatively expensive.
    if (new_size > max_size_)
    {
        file_helper_.flush();
        if (file_helper_.size() > 0)
        {
            rotate_();
            new_size = formatted.size();
        }
    }
    file_helper_.write(formatted);
    current_size_ = new_size;
}

template<typename Mutex>
SPDLOG_INLINE void rotating2_file_sink<Mutex>::flush_()
{
    file_helper_.flush();
}

template<typename Mutex>
SPDLOG_INLINE void rotating2_file_sink<Mutex>::rotate_()
{
    using details::os::filename_to_str;

    file_helper_.close();

    filename_t src = base_filename_;
    if (!details::os::path_exists(src))
    {
        return;
    }

    auto now = (time_type_ == pattern_time_type::local) ? details::os::localtime(log_clock::to_time_t(log_clock::now()))
                                                        : details::os::gmtime(log_clock::to_time_t(log_clock::now()));

    filename_t target = calc_filename(base_filename_, now);

    if (!rename_file_(src, target))
    {
        // if failed try again after a small delay.
        // this is a workaround to a windows issue, where very high rotation
        // rates can cause the rename to fail with permission denied (because of antivirus?).
        details::os::sleep_for_millis(100);
        if (!rename_file_(src, target))
        {
            file_helper_.reopen(true); // truncate the log file anyway to prevent it to grow beyond its limit!
            current_size_ = 0;
            throw_spdlog_ex("rotating2_file_sink: failed renaming " + filename_to_str(src) + " to " + filename_to_str(target), errno);
        }
    }

    file_helper_.reopen(true);
}

// return true on success, false otherwise.
template<typename Mutex>
SPDLOG_INLINE bool rotating2_file_sink<Mutex>::rename_file_(const filename_t &src_filename, const filename_t &target_filename)
{
    const auto calc_target_filename = [](filename_t filename, int index) {
        if (index > 0)
        {
            filename_t basename, ext;
            std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
            return fmt_lib::format(SPDLOG_FILENAME_T("{}.{}{}"), basename, index, ext);
        }

        return filename;
    };

    int index = 0;

    while (details::os::path_exists(calc_target_filename(target_filename, index)))
    {
        ++index;
    }

    return details::os::rename(src_filename, calc_target_filename(target_filename, index)) == 0;
}

} // namespace sinks
} // namespace spdlog