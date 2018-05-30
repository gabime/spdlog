#pragma once

#include "../../details/file_helper.h"
#include "../../details/null_mutex.h"
#include "../../fmt/fmt.h"
#include "../../sinks/base_sink.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>

// Example for spdlog.h
//
// Create a file logger which creates new files with a specified time step and fixed file size:
//
// std::shared_ptr<logger> step_logger_mt(const std::string &logger_name, const filename_t &filename, unsigned seconds = 60, const
// filename_t &tmp_ext = ".tmp", unsigned max_file_size = std::numeric_limits<unsigned>::max(), bool delete_empty_files = true, const filename_t &file_header = ""); std::shared_ptr<logger>
// step_logger_st(const std::string &logger_name, const filename_t &filename, unsigned seconds = 60, const filename_t &tmp_ext = ".tmp",
// unsigned max_file_size = std::numeric_limits<unsigned>::max());

// Example for spdlog_impl.h
// Create a file logger that creates new files with a specified increment
// inline std::shared_ptr<spdlog::logger> spdlog::step_logger_mt(
//     const std::string &logger_name, const filename_t &filename_fmt, unsigned seconds, const filename_t &tmp_ext, unsigned max_file_size, bool delete_empty_files, const filename_t &file_header)
// {
//     return create<spdlog::sinks::step_file_sink_mt>(logger_name, filename_fmt, seconds, tmp_ext, max_file_size, delete_empty_files, file_header);
// }

// inline std::shared_ptr<spdlog::logger> spdlog::step_logger_st(
//     const std::string &logger_name, const filename_t &filename_fmt, unsigned seconds, const filename_t &tmp_ext, unsigned max_file_size, bool delete_empty_files, const filename_t &file_header)
// {
//     return create<spdlog::sinks::step_file_sink_st>(logger_name, filename_fmt, seconds, tmp_ext, max_file_size, delete_empty_files, file_header);
// }

namespace spdlog {
namespace sinks {

/*
 * Default generator of step log file names.
 */
struct default_step_file_name_calculator
{
    // Create filename for the form filename_YYYY-MM-DD_hh-mm-ss.ext
    static std::tuple<filename_t, filename_t> calc_filename(const filename_t &filename, const filename_t &tmp_ext)
    {
        std::tm tm = spdlog::details::os::localtime();
        filename_t basename, ext;
        std::tie(basename, ext) = details::file_helper::split_by_extenstion(filename);
        std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::MemoryWriter, fmt::WMemoryWriter>::type w;
        w.write(SPDLOG_FILENAME_T("{}_{:04d}-{:02d}-{:02d}_{:02d}-{:02d}-{:02d}{}"), basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec, tmp_ext);
        return std::make_tuple(w.str(), ext);
    }
};

/*
 * Rotating file sink based on size and a specified time step
 */
template<class Mutex, class FileNameCalc = default_step_file_name_calculator>
class step_file_sink SPDLOG_FINAL : public base_sink<Mutex>
{
public:
    step_file_sink(filename_t base_filename, unsigned step_seconds, filename_t tmp_ext, unsigned max_size, bool delete_empty_files, filename_t file_header)
        : _base_filename(std::move(base_filename))
        , _tmp_ext(std::move(tmp_ext))
        , _step_seconds(step_seconds)
        , _max_size(max_size)
        , _delete_empty_files(delete_empty_files)
    {
        if (step_seconds == 0)
        {
            throw spdlog_ex("step_file_sink: Invalid time step in ctor");
        }

        if (!file_header.empty())
        {
            pattern_formatter formatter_for_file_header("%v");
            _file_header.raw << file_header;
            formatter_for_file_header.format(_file_header);
        }

        if (max_size <= _file_header.formatted.size())
        {
            throw spdlog_ex("step_file_sink: Invalid max log size in ctor");
        }

        _tp = _next_tp();
        std::tie(_current_filename, _ext) = FileNameCalc::calc_filename(_base_filename, _tmp_ext);

        if (_tmp_ext == _ext)
        {
            throw spdlog_ex("step_file_sink: The temporary extension matches the specified in ctor");
        }

        _file_helper.open(_current_filename);
        _current_size = _file_helper.size(); // expensive. called only once

        if (!_current_size)
        {
            _current_size += _file_header.formatted.size();
            if (_current_size) _file_helper.write(_file_header);
        }
    }

    ~step_file_sink()
    {
        try
        {
            close_current_file();
        }
        catch (...)
        {
        }
    }

protected:
    void _sink_it(const details::log_msg &msg) override
    {
        auto msg_size = msg.formatted.size();

        if (std::chrono::system_clock::now() >= _tp || _current_size + msg_size > _max_size)
        {
            filename_t new_filename;
            std::tie(new_filename, std::ignore) = FileNameCalc::calc_filename(_base_filename, _tmp_ext);
            
            bool change_occured = !details::file_helper::file_exists(new_filename);
            if (change_occured) 
            {
                close_current_file();

                _current_filename = std::move(new_filename);

                _file_helper.open(_current_filename);
            }

            _tp = _next_tp();

            if (change_occured)
            {
                _current_size = _file_header.formatted.size();
                if (_current_size) _file_helper.write(_file_header);
            }
        }

        _current_size += msg_size;
        _file_helper.write(msg);
    }

    void _flush() override
    {
        _file_helper.flush();
    }

private:
    std::chrono::system_clock::time_point _next_tp()
    {
        return std::chrono::system_clock::now() + _step_seconds;
    }

    void close_current_file()
    {
        using details::os::filename_to_str;

        // Delete empty files, if required
        if (_delete_empty_files && _current_size <= _file_header.formatted.size())
        {
            if (details::os::remove(_current_filename) != 0)
            {
                throw spdlog_ex("step_file_sink: not remove " + filename_to_str(_current_filename), errno);
            }

            return;
        }

        filename_t target;
        std::tie(target, std::ignore) = details::file_helper::split_by_extenstion(_current_filename);
        target += _ext;

        if (details::file_helper::file_exists(_current_filename) && details::os::rename(_current_filename, target) != 0)
        {
            throw spdlog_ex("step_file_sink: failed renaming " + filename_to_str(_current_filename) + " to " + filename_to_str(target), errno);
        }
    }

    const filename_t _base_filename;
    const filename_t _tmp_ext;
    const std::chrono::seconds _step_seconds;
    const unsigned _max_size;
    const bool _delete_empty_files;

    std::chrono::system_clock::time_point _tp;
    filename_t _current_filename;
    filename_t _ext;
    unsigned _current_size;

    details::file_helper _file_helper;
    details::log_msg _file_header;
};

using step_file_sink_mt = step_file_sink<std::mutex>;
using step_file_sink_st = step_file_sink<details::null_mutex>;

} // namespace sinks
} // namespace spdlog
