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
// filename_t &tmp_ext = ".tmp", unsigned max_file_size = std::numeric_limits<unsigned>::max()); std::shared_ptr<logger>
// step_logger_st(const std::string &logger_name, const filename_t &filename, unsigned seconds = 60, const filename_t &tmp_ext = ".tmp",
// unsigned max_file_size = std::numeric_limits<unsigned>::max());

// Example for spdlog_impl.h
// Create a file logger that creates new files with a specified increment
// inline std::shared_ptr<spdlog::logger> spdlog::step_logger_mt(
//     const std::string &logger_name, const filename_t &filename_fmt, unsigned seconds, const filename_t &tmp_ext, unsigned max_file_size)
// {
//     return create<spdlog::sinks::step_file_sink_mt>(logger_name, filename_fmt, seconds, tmp_ext, max_file_size);
// }

// inline std::shared_ptr<spdlog::logger> spdlog::step_logger_st(
//     const std::string &logger_name, const filename_t &filename_fmt, unsigned seconds, const filename_t &tmp_ext, unsigned max_file_size)
// {
//     return create<spdlog::sinks::step_file_sink_st>(logger_name, filename_fmt, seconds, tmp_ext, max_file_size);
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
    step_file_sink(filename_t base_filename, unsigned step_seconds, filename_t tmp_ext, unsigned max_size)
        : _base_filename(std::move(base_filename))
        , _tmp_ext(std::move(tmp_ext))
        , _step_seconds(step_seconds)
        , _max_size(max_size)
    {
        if (step_seconds == 0)
        {
            throw spdlog_ex("step_file_sink: Invalid time step in ctor");
        }
        if (max_size == 0)
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
        _current_size += msg.formatted.size();
        if (std::chrono::system_clock::now() >= _tp || _current_size > _max_size)
        {
            close_current_file();

            std::tie(_current_filename, std::ignore) = FileNameCalc::calc_filename(_base_filename, _tmp_ext);
            _file_helper.open(_current_filename);
            _tp = _next_tp();
            _current_size = msg.formatted.size();
        }
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

        filename_t src = _current_filename, target;
        std::tie(target, std::ignore) = details::file_helper::split_by_extenstion(src);
        target += _ext;

        if (details::file_helper::file_exists(src) && details::os::rename(src, target) != 0)
        {
            throw spdlog_ex("step_file_sink: failed renaming " + filename_to_str(src) + " to " + filename_to_str(target), errno);
        }
    }

    const filename_t _base_filename;
    const filename_t _tmp_ext;
    const std::chrono::seconds _step_seconds;
    const unsigned _max_size;

    std::chrono::system_clock::time_point _tp;
    filename_t _current_filename;
    filename_t _ext;
    unsigned _current_size;

    details::file_helper _file_helper;
};

using step_file_sink_mt = step_file_sink<std::mutex>;
using step_file_sink_st = step_file_sink<details::null_mutex>;

} // namespace sinks
} // namespace spdlog
