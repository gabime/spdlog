// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>

#include <chrono>
#include <mutex>
#include <string>

namespace spdlog {
namespace sinks {

/*
 * Generator of rotating log file names in format basename.index.extension
 * e.g. calc_filename("logs/mylog.txt, 3) => "logs/mylog.3.txt".
 */
struct rotating_filename_calculator
{
    // Create filename for the form basename.index.extension
    static filename_t calc_filename(const filename_t &filename, std::size_t index)
    {
        if (index == 0u)
        {
            return filename;
        }

        filename_t basename, ext;
        std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
        return fmt::format(SPDLOG_FILENAME_T("{}.{}{}"), basename, index, ext);
    }
};

//
// Rotating file sink based on size
//
template<typename Mutex, typename FileNameCalc = rotating_filename_calculator>
class rotating_file_sink final : public base_sink<Mutex>
{
public:
    rotating_file_sink(filename_t base_filename, std::size_t max_size, std::size_t max_files, bool rotate_on_open = false)
        : base_filename_(std::move(base_filename))
        , max_size_(max_size)
        , max_files_(max_files)
    {
        file_helper_.open(FileNameCalc::calc_filename(base_filename_, 0));
        current_size_ = file_helper_.size(); // expensive. called only once
        if (rotate_on_open && current_size_ > 0)
        {
            rotate_();
        }
    }
    filename_t filename()
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return file_helper_.filename();
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        current_size_ += formatted.size();
        if (current_size_ > max_size_)
        {
            rotate_();
            current_size_ = formatted.size();
        }
        file_helper_.write(formatted);
    }
    void flush_() override
    {
        file_helper_.flush();
    }

private:
    // Rotate files:
    // log.txt -> log.1.txt
    // log.1.txt -> log.2.txt
    // log.2.txt -> log.3.txt
    // log.3.txt -> delete
    void rotate_()
    {
        using details::os::filename_to_str;
        using details::os::path_exists;
        file_helper_.close();
        for (auto i = max_files_; i > 0; --i)
        {
            filename_t src = FileNameCalc::calc_filename(base_filename_, i - 1);
            if (!path_exists(src))
            {
                continue;
            }
            filename_t target = FileNameCalc::calc_filename(base_filename_, i);

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
                    throw_spdlog_ex("rotating_file_sink: failed renaming " + filename_to_str(src) + " to " + filename_to_str(target), errno);
                }
            }
        }
        file_helper_.reopen(true);
    }

    // delete the target if exists, and rename the src file  to target
    // return true on success, false otherwise.
    bool rename_file_(const filename_t &src_filename, const filename_t &target_filename)
    {
        // try to delete the target file in case it already exists.
        (void)details::os::remove(target_filename);
        return details::os::rename(src_filename, target_filename) == 0;
    }

    filename_t base_filename_;
    std::size_t max_size_;
    std::size_t max_files_;
    std::size_t current_size_;
    details::file_helper file_helper_;
};


using rotating_file_sink_mt = rotating_file_sink<std::mutex>;
using rotating_file_sink_st = rotating_file_sink<details::null_mutex>;

} // namespace sinks

//
// factory functions
//

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> rotating_logger_mt(
    const std::string &logger_name, const filename_t &filename, size_t max_file_size, size_t max_files, bool rotate_on_open = false)
{
    return Factory::template create<sinks::rotating_file_sink_mt>(logger_name, filename, max_file_size, max_files, rotate_on_open);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> rotating_logger_st(
    const std::string &logger_name, const filename_t &filename, size_t max_file_size, size_t max_files, bool rotate_on_open = false)
{
    return Factory::template create<sinks::rotating_file_sink_st>(logger_name, filename, max_file_size, max_files, rotate_on_open);
}
} // namespace spdlog
