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
        : force_flush_(false)
    {
        file_helper_.open(filename, truncate);
    }

    void set_force_flush(bool force_flush)
    {
        force_flush_ = force_flush;
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        file_helper_.write(msg);
        if (force_flush_)
        {
            file_helper_.flush();
        }
    }

    void flush_() override
    {
        file_helper_.flush();
    }

private:
    details::file_helper file_helper_;
    bool force_flush_;
};

using simple_file_sink_mt = simple_file_sink<std::mutex>;
using simple_file_sink_st = simple_file_sink<details::null_mutex>;

} // namespace sinks

//
// factory functions
//
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
