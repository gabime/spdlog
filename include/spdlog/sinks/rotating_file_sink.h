//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
#include "spdlog/details/file_helper.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/spdlog.h"

#include <cerrno>
#include <chrono>
#include <ctime>
#include <mutex>
#include <string>
#include <tuple>

namespace spdlog {
namespace sinks {

//
// Rotating file sink based on size
//
template<typename Mutex, typename FileNameCalc = details::filename_calculator>
class rotating_file_sink final : public base_sink<Mutex>
{
public:
    rotating_file_sink(filename_t base_filename, std::size_t max_size, std::size_t max_files)
        : base_filename_(std::move(base_filename))
        , max_size_(max_size)
        , max_files_(max_files)
    {
        // calculate the amount of current backups
        for (current_files_ = 0; ; ++current_files_)
        {
            filename_t src = FileNameCalc::calc_filename(base_filename_, current_files_);
            if (!details::file_helper::file_exists(src))
            {
                break;
            }
        }

        file_helper_.open(FileNameCalc::calc_filename(base_filename_, 0));
        current_size_ = file_helper_.size(); // expensive. called only once
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        fmt::memory_buffer formatted;
        sink::formatter_->format(msg, formatted);
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
        file_helper_.close();

        // update amount of file
        if (++current_files_ > max_files_)
        {
            current_files_ = max_files_;
        }
        for (auto i = current_files_; i > 0; --i)
        {
            filename_t src = FileNameCalc::calc_filename(base_filename_, i - 1);
            if (!details::file_helper::file_exists(src))
            {
                continue;
            }
            filename_t target = FileNameCalc::calc_filename(base_filename_, i);

            if (!FileNameCalc::rename_file(src, target))
            {
                // if failed try again after a small delay.
                // this is a workaround to a windows issue, where very high rotation
                // rates can cause the rename to fail with permission denied (because of antivirus?).
                details::os::sleep_for_millis(100);
                if (!FileNameCalc::rename_file(src, target))
                {
                    file_helper_.reopen(true); // truncate the log file anyway to prevent it to grow beyond its limit!
                    throw spdlog_ex(
                        "rotating_file_sink: failed renaming " + filename_to_str(src) + " to " + filename_to_str(target), errno);
                }
            }
        }
        file_helper_.reopen(true);
    }

    filename_t base_filename_;
    std::size_t max_size_;
    std::size_t max_files_;
    std::size_t current_size_;
    std::size_t current_files_;
    details::file_helper file_helper_;
};

using rotating_file_sink_mt = rotating_file_sink<std::mutex>;
using rotating_file_sink_st = rotating_file_sink<details::null_mutex>;

} // namespace sinks

//
// factory functions
//

template<typename Factory = default_factory>
inline std::shared_ptr<logger> rotating_logger_mt(
    const std::string &logger_name, const filename_t &filename, size_t max_file_size, size_t max_files)
{
    return Factory::template create<sinks::rotating_file_sink_mt>(logger_name, filename, max_file_size, max_files);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> rotating_logger_st(
    const std::string &logger_name, const filename_t &filename, size_t max_file_size, size_t max_files)
{
    return Factory::template create<sinks::rotating_file_sink_st>(logger_name, filename, max_file_size, max_files);
}
} // namespace spdlog
