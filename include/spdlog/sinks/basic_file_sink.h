//
// Copyright(c) 2015-2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#ifndef SPDLOG_H
#include "spdlog/spdlog.h"
#endif

#include "spdlog/details/file_helper.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/sinks/base_sink.h"

#include <mutex>
#include <string>

namespace spdlog {
namespace sinks {
/*
 * Trivial file sink with single file as target
 */
template<typename Mutex>
class basic_file_sink final : public base_sink<Mutex>
{
public:
    explicit basic_file_sink(const filename_t &filename, bool truncate = false)
    {
        file_helper_.open(filename, truncate);
    }

    const filename_t &filename() const
    {
        return file_helper_.filename();
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        fmt::memory_buffer formatted;
        sink::formatter_->format(msg, formatted);
        file_helper_.write(formatted);
    }

    void flush_() override
    {
        file_helper_.flush();
    }

private:
    details::file_helper file_helper_;
};

using basic_file_sink_mt = basic_file_sink<std::mutex>;
using basic_file_sink_st = basic_file_sink<details::null_mutex>;

} // namespace sinks

//
// factory functions
//
template<typename Factory = default_factory>
inline std::shared_ptr<logger> basic_logger_mt(const std::string &logger_name, const filename_t &filename, bool truncate = false)
{
    return Factory::template create<sinks::basic_file_sink_mt>(logger_name, filename, truncate);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> basic_logger_st(const std::string &logger_name, const filename_t &filename, bool truncate = false)
{
    return Factory::template create<sinks::basic_file_sink_st>(logger_name, filename, truncate);
}

} // namespace spdlog
