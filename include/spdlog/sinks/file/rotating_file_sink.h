//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/details/file_helper.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/sinks/base_sink.h"

#include <cerrno>
#include <chrono>
#include <tuple>
#include <ctime>
#include <mutex>
#include <string>

namespace spdlog {
namespace sinks {

//
// Rotating file sink based on size
//
template<class Mutex>
class rotating_file_sink SPDLOG_FINAL : public base_sink<Mutex>
{
public:
    rotating_file_sink(filename_t base_filename, std::size_t max_size, std::size_t max_files)
        : _base_filename(std::move(base_filename))
        , _max_size(max_size)
        , _max_files(max_files)
    {
        _file_helper.open(calc_filename(_base_filename, 0));
        _current_size = _file_helper.size(); // expensive. called only once
    }

    // calc filename according to index and file extension if exists.
    // e.g. calc_filename("logs/mylog.txt, 3) => "logs/mylog.3.txt".
    static filename_t calc_filename(const filename_t &filename, std::size_t index)
    {
        typename std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::MemoryWriter, fmt::WMemoryWriter>::type w;
        if (index != 0u)
        {
            filename_t basename, ext;
            std::tie(basename, ext) = details::file_helper::split_by_extenstion(filename);
            w.write(SPDLOG_FILENAME_T("{}.{}{}"), basename, index, ext);
        }
        else
        {
            w.write(SPDLOG_FILENAME_T("{}"), filename);
        }
        return w.str();
    }

protected:
    void _sink_it(const details::log_msg &msg) override
    {
        _current_size += msg.formatted.size();
        if (_current_size > _max_size)
        {
            _rotate();
            _current_size = msg.formatted.size();
        }
        _file_helper.write(msg);
    }

    void _flush() override
    {
        _file_helper.flush();
    }

private:
    // Rotate files:
    // log.txt -> log.1.txt
    // log.1.txt -> log.2.txt
    // log.2.txt -> log.3.txt
    // log.3.txt -> delete
    void _rotate()
    {
        using details::os::filename_to_str;
        _file_helper.close();
        for (auto i = _max_files; i > 0; --i)
        {
            filename_t src = calc_filename(_base_filename, i - 1);
            filename_t target = calc_filename(_base_filename, i);

            if (details::file_helper::file_exists(target))
            {
                if (details::os::remove(target) != 0)
                {
                    throw spdlog_ex("rotating_file_sink: failed removing " + filename_to_str(target), errno);
                }
            }
            if (details::file_helper::file_exists(src) && details::os::rename(src, target) != 0)
            {
                throw spdlog_ex("rotating_file_sink: failed renaming " + filename_to_str(src) + " to " + filename_to_str(target), errno);
            }
        }
        _file_helper.reopen(true);
    }

    filename_t _base_filename;
    std::size_t _max_size;
    std::size_t _max_files;
    std::size_t _current_size;
    details::file_helper _file_helper;
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
