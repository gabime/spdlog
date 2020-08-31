// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/logger.h>
#include <spdlog/common.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/async.h>

namespace spdlog {

template<typename Factory>
std::shared_ptr<logger> stdout_color_mt(const std::string &logger_name, color_mode mode)
{
    return Factory::template create<sinks::stdout_color_sink_mt>(logger_name, mode);
}

template<typename Factory>
std::shared_ptr<logger> stdout_color_st(const std::string &logger_name, color_mode mode)
{
    return Factory::template create<sinks::stdout_color_sink_st>(logger_name, mode);
}

template<typename Factory>
std::shared_ptr<logger> stderr_color_mt(const std::string &logger_name, color_mode mode)
{
    return Factory::template create<sinks::stderr_color_sink_mt>(logger_name, mode);
}

template<typename Factory>
std::shared_ptr<logger> stderr_color_st(const std::string &logger_name, color_mode mode)
{
    return Factory::template create<sinks::stderr_color_sink_st>(logger_name, mode);
}

} // namespace spdlog

// template instantiations
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stdout_color_mt<spdlog::synchronous_factory>(
    const std::string &logger_name, color_mode mode);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stdout_color_st<spdlog::synchronous_factory>(
    const std::string &logger_name, color_mode mode);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stderr_color_mt<spdlog::synchronous_factory>(
    const std::string &logger_name, color_mode mode);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stderr_color_st<spdlog::synchronous_factory>(
    const std::string &logger_name, color_mode mode);

template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stdout_color_mt<spdlog::async_factory>(
    const std::string &logger_name, color_mode mode);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stdout_color_st<spdlog::async_factory>(
    const std::string &logger_name, color_mode mode);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stderr_color_mt<spdlog::async_factory>(
    const std::string &logger_name, color_mode mode);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stderr_color_st<spdlog::async_factory>(
    const std::string &logger_name, color_mode mode);
