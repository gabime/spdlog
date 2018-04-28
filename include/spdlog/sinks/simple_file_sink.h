//
// Copyright(c) 2015-2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
#include "spdlog/details/file_helper.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/spdlog.h"

#include <mutex>
#include <string>

namespace spdlog {
namespace sinks {
/*
 * Trivial file sink with single file as target
 */
template<class Mutex>
class simple_file_sink SPDLOG_FINAL : public base_sink<Mutex>
{
public:
    explicit simple_file_sink(const filename_t &filename, bool truncate = false)
        : _force_flush(false)
    {
        _file_helper.open(filename, truncate);
    }

    void set_force_flush(bool force_flush)
    {
        _force_flush = force_flush;
    }

protected:
    void _sink_it(const details::log_msg &msg) override
    {
        _file_helper.write(msg);
        if (_force_flush)
        {
            _file_helper.flush();
        }
    }

    void _flush() override
    {
        _file_helper.flush();
    }

private:
    details::file_helper _file_helper;
    bool _force_flush;
};

using simple_file_sink_mt = simple_file_sink<std::mutex>;
using simple_file_sink_st = simple_file_sink<details::null_mutex>;

} // namespace sinks

//
// factory functions
//

// Basic logger simply writes to given file without any limitations or rotations.
template<typename Factory = default_factory>
inline std::shared_ptr<logger> basic_logger_mt(const std::string &logger_name, const filename_t &filename, bool truncate = false)
{
    return Factory::template create<sinks::simple_file_sink_mt>(logger_name, filename, truncate);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> basic_logger_st(const std::string &logger_name, const filename_t &filename, bool truncate = false)
{
    return Factory::template create<sinks::simple_file_sink_st>(logger_name, filename, truncate);
}

} // namespace spdlog
