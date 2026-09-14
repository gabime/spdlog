// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef SPDLOG_COMPILED_LIB
#error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

#include <mutex>

#include <spdlog/async.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/stdout_sinks-inl.h>

template class SPDLOG_API SPDLOG_NAMESPACE::sinks::stdout_sink_base<SPDLOG_NAMESPACE::details::console_mutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::stdout_sink_base<SPDLOG_NAMESPACE::details::console_nullmutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::stdout_sink<SPDLOG_NAMESPACE::details::console_mutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::stdout_sink<SPDLOG_NAMESPACE::details::console_nullmutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::stderr_sink<SPDLOG_NAMESPACE::details::console_mutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::stderr_sink<SPDLOG_NAMESPACE::details::console_nullmutex>;

template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger>
SPDLOG_NAMESPACE::stdout_logger_mt<SPDLOG_NAMESPACE::synchronous_factory>(const std::string &logger_name);
template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger>
SPDLOG_NAMESPACE::stdout_logger_st<SPDLOG_NAMESPACE::synchronous_factory>(const std::string &logger_name);
template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger>
SPDLOG_NAMESPACE::stderr_logger_mt<SPDLOG_NAMESPACE::synchronous_factory>(const std::string &logger_name);
template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger>
SPDLOG_NAMESPACE::stderr_logger_st<SPDLOG_NAMESPACE::synchronous_factory>(const std::string &logger_name);

template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger> SPDLOG_NAMESPACE::stdout_logger_mt<SPDLOG_NAMESPACE::async_factory>(
    const std::string &logger_name);
template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger> SPDLOG_NAMESPACE::stdout_logger_st<SPDLOG_NAMESPACE::async_factory>(
    const std::string &logger_name);
template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger> SPDLOG_NAMESPACE::stderr_logger_mt<SPDLOG_NAMESPACE::async_factory>(
    const std::string &logger_name);
template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger> SPDLOG_NAMESPACE::stderr_logger_st<SPDLOG_NAMESPACE::async_factory>(
    const std::string &logger_name);
