// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

//
// Sink to wrap another logger. This is usefull when consolidating multiple application components
// with built-in loggers into one app wide logger.
//

#include <spdlog/sinks/base_sink.h>
#include <spdlog/logger.h>

namespace spdlog {
namespace sinks {

template <typename Mutex>
class relay_sink : public base_sink<Mutex> {
public:
    relay_sink(const std::shared_ptr<spdlog::logger>& logger)
        : m_logger{logger} {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override { m_logger->log_raw(msg); };
    void flush_() override{};

private:
    std::shared_ptr<spdlog::logger> m_logger;
};

using relay_sink_mt = relay_sink<std::mutex>;
using relay_sink_st = relay_sink<spdlog::details::null_mutex>;

}  // namespace sinks
}  // namespace spdlog
