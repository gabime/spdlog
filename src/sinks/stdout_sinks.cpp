// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <mutex>

#include <spdlog/details/null_mutex.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/details/console_globals.h>
#include <spdlog/pattern_formatter.h>
#include <memory>

namespace spdlog {

namespace sinks {

template<typename ConsoleMutex>
stdout_sink_base<ConsoleMutex>::stdout_sink_base(FILE *file)
    : mutex_(ConsoleMutex::mutex())
    , file_(file)
    , formatter_(std::make_unique<spdlog::pattern_formatter>())
{}

template<typename ConsoleMutex>
void stdout_sink_base<ConsoleMutex>::log(const details::log_msg &msg)
{
    std::lock_guard<mutex_t> lock(mutex_);
    memory_buf_t formatted;
    formatter_->format(msg, formatted);
    fwrite(formatted.data(), sizeof(char), formatted.size(), file_);
    fflush(file_); // flush every line to terminal
}

template<typename ConsoleMutex>
void stdout_sink_base<ConsoleMutex>::flush()
{
    std::lock_guard<mutex_t> lock(mutex_);
    fflush(file_);
}

template<typename ConsoleMutex>
void stdout_sink_base<ConsoleMutex>::set_pattern(const std::string &pattern)
{
    std::lock_guard<mutex_t> lock(mutex_);
    formatter_ = std::unique_ptr<spdlog::formatter>(new pattern_formatter(pattern));
}

template<typename ConsoleMutex>
void stdout_sink_base<ConsoleMutex>::set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter)
{
    std::lock_guard<mutex_t> lock(mutex_);
    formatter_ = std::move(sink_formatter);
}

// stdout sink
template<typename ConsoleMutex>
stdout_sink<ConsoleMutex>::stdout_sink()
    : stdout_sink_base<ConsoleMutex>(stdout)
{}

// stderr sink
template<typename ConsoleMutex>
stderr_sink<ConsoleMutex>::stderr_sink()
    : stdout_sink_base<ConsoleMutex>(stderr)
{}

} // namespace sinks

// factory methods
template<typename Factory>
std::shared_ptr<logger> stdout_logger_mt(const std::string &logger_name)
{
    return Factory::template create<sinks::stdout_sink_mt>(logger_name);
}

template<typename Factory>
std::shared_ptr<logger> stdout_logger_st(const std::string &logger_name)
{
    return Factory::template create<sinks::stdout_sink_st>(logger_name);
}

template<typename Factory>
std::shared_ptr<logger> stderr_logger_mt(const std::string &logger_name)
{
    return Factory::template create<sinks::stderr_sink_mt>(logger_name);
}

template<typename Factory>
std::shared_ptr<logger> stderr_logger_st(const std::string &logger_name)
{
    return Factory::template create<sinks::stderr_sink_st>(logger_name);
}
} // namespace spdlog

template class SPDLOG_API spdlog::sinks::stdout_sink_base<spdlog::details::console_mutex>;
template class SPDLOG_API spdlog::sinks::stdout_sink_base<spdlog::details::console_nullmutex>;
template class SPDLOG_API spdlog::sinks::stdout_sink<spdlog::details::console_mutex>;
template class SPDLOG_API spdlog::sinks::stdout_sink<spdlog::details::console_nullmutex>;
template class SPDLOG_API spdlog::sinks::stderr_sink<spdlog::details::console_mutex>;
template class SPDLOG_API spdlog::sinks::stderr_sink<spdlog::details::console_nullmutex>;

template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stdout_logger_mt<spdlog::synchronous_factory>(const std::string &logger_name);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stdout_logger_st<spdlog::synchronous_factory>(const std::string &logger_name);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stderr_logger_mt<spdlog::synchronous_factory>(const std::string &logger_name);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stderr_logger_st<spdlog::synchronous_factory>(const std::string &logger_name);

template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stdout_logger_mt<spdlog::async_factory>(const std::string &logger_name);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stdout_logger_st<spdlog::async_factory>(const std::string &logger_name);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stderr_logger_mt<spdlog::async_factory>(const std::string &logger_name);
template SPDLOG_API std::shared_ptr<spdlog::logger> spdlog::stderr_logger_st<spdlog::async_factory>(const std::string &logger_name);