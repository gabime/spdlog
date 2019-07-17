// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/details/null_mutex.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/synchronous_factory.h"

#include <mutex>
#include <stdio.h>

namespace spdlog {
namespace sinks {

template<typename Mutex>
class null_sink : public base_sink<Mutex>
{
protected:
    void sink_it_(const details::log_msg &msg) override
    {
        // prevent optimizer to remove this sink altogether (and do useful check while at it).
        if(msg.level == level::off)
        {
            printf("null_sink: should never not be called if level is off!\n");
            abort();
        }
    }
    void flush_() override {}
};

using null_sink_mt = null_sink<details::null_mutex>;
using null_sink_st = null_sink<details::null_mutex>;

} // namespace sinks

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> null_logger_mt(const std::string &logger_name)
{
    auto null_logger = Factory::template create<sinks::null_sink_mt>(logger_name);
    null_logger->set_level(level::off);
    return null_logger;
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> null_logger_st(const std::string &logger_name)
{
    auto null_logger = Factory::template create<sinks::null_sink_st>(logger_name);
    null_logger->set_level(level::off);
    return null_logger;
}

} // namespace spdlog
