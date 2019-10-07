// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef SPDLOG_COMPILED_LIB
#error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

#include <mutex>

#ifdef SPDLOG_USE_EXPORT_HEADER
#include "spdlog_export.h"
#else
#define SPDLOG_EXPORT
#endif
#include "spdlog/details/null_mutex.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_sinks-inl.h"

template class SPDLOG_EXPORT spdlog::sinks::stdout_sink_base<spdlog::details::console_mutex>;
template class SPDLOG_EXPORT spdlog::sinks::stdout_sink_base<spdlog::details::console_nullmutex>;
template class SPDLOG_EXPORT spdlog::sinks::stdout_sink<spdlog::details::console_mutex>;
template class SPDLOG_EXPORT spdlog::sinks::stdout_sink<spdlog::details::console_nullmutex>;
template class SPDLOG_EXPORT spdlog::sinks::stderr_sink<spdlog::details::console_mutex>;
template class SPDLOG_EXPORT spdlog::sinks::stderr_sink<spdlog::details::console_nullmutex>;

template SPDLOG_EXPORT std::shared_ptr<spdlog::logger> spdlog::stdout_logger_mt<spdlog::synchronous_factory>(const std::string &logger_name);
template SPDLOG_EXPORT std::shared_ptr<spdlog::logger> spdlog::stdout_logger_st<spdlog::synchronous_factory>(const std::string &logger_name);
template SPDLOG_EXPORT std::shared_ptr<spdlog::logger> spdlog::stderr_logger_mt<spdlog::synchronous_factory>(const std::string &logger_name);
template SPDLOG_EXPORT std::shared_ptr<spdlog::logger> spdlog::stderr_logger_st<spdlog::synchronous_factory>(const std::string &logger_name);

template SPDLOG_EXPORT std::shared_ptr<spdlog::logger> spdlog::stdout_logger_mt<spdlog::async_factory>(const std::string &logger_name);
template SPDLOG_EXPORT std::shared_ptr<spdlog::logger> spdlog::stdout_logger_st<spdlog::async_factory>(const std::string &logger_name);
template SPDLOG_EXPORT std::shared_ptr<spdlog::logger> spdlog::stderr_logger_mt<spdlog::async_factory>(const std::string &logger_name);
template SPDLOG_EXPORT std::shared_ptr<spdlog::logger> spdlog::stderr_logger_st<spdlog::async_factory>(const std::string &logger_name);
