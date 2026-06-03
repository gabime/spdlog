// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef SPDLOG_COMPILED_LIB
#error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

#include <mutex>

#include <spdlog/async.h>
#include <spdlog/details/null_mutex.h>
//
// color sinks
//
#ifdef _WIN32
#include <spdlog/sinks/wincolor_sink-inl.h>
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::wincolor_sink<SPDLOG_NAMESPACE::details::console_mutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::wincolor_sink<SPDLOG_NAMESPACE::details::console_nullmutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::wincolor_stdout_sink<SPDLOG_NAMESPACE::details::console_mutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::wincolor_stdout_sink<SPDLOG_NAMESPACE::details::console_nullmutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::wincolor_stderr_sink<SPDLOG_NAMESPACE::details::console_mutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::wincolor_stderr_sink<SPDLOG_NAMESPACE::details::console_nullmutex>;
#else
#include "spdlog/sinks/ansicolor_sink-inl.h"
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::ansicolor_sink<SPDLOG_NAMESPACE::details::console_mutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::ansicolor_sink<SPDLOG_NAMESPACE::details::console_nullmutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::ansicolor_stdout_sink<SPDLOG_NAMESPACE::details::console_mutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::ansicolor_stdout_sink<SPDLOG_NAMESPACE::details::console_nullmutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::ansicolor_stderr_sink<SPDLOG_NAMESPACE::details::console_mutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::ansicolor_stderr_sink<SPDLOG_NAMESPACE::details::console_nullmutex>;
#endif

// factory methods for color loggers
#include "spdlog/sinks/stdout_color_sinks-inl.h"
template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger>
SPDLOG_NAMESPACE::stdout_color_mt<SPDLOG_NAMESPACE::synchronous_factory>(const std::string &logger_name,
                                                     color_mode mode);
template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger>
SPDLOG_NAMESPACE::stdout_color_st<SPDLOG_NAMESPACE::synchronous_factory>(const std::string &logger_name,
                                                     color_mode mode);
template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger>
SPDLOG_NAMESPACE::stderr_color_mt<SPDLOG_NAMESPACE::synchronous_factory>(const std::string &logger_name,
                                                     color_mode mode);
template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger>
SPDLOG_NAMESPACE::stderr_color_st<SPDLOG_NAMESPACE::synchronous_factory>(const std::string &logger_name,
                                                     color_mode mode);

template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger> SPDLOG_NAMESPACE::stdout_color_mt<SPDLOG_NAMESPACE::async_factory>(
    const std::string &logger_name, color_mode mode);
template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger> SPDLOG_NAMESPACE::stdout_color_st<SPDLOG_NAMESPACE::async_factory>(
    const std::string &logger_name, color_mode mode);
template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger> SPDLOG_NAMESPACE::stderr_color_mt<SPDLOG_NAMESPACE::async_factory>(
    const std::string &logger_name, color_mode mode);
template SPDLOG_API std::shared_ptr<SPDLOG_NAMESPACE::logger> SPDLOG_NAMESPACE::stderr_color_st<SPDLOG_NAMESPACE::async_factory>(
    const std::string &logger_name, color_mode mode);
