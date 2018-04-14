////
//// Copyright(c) 2015 Gabi Melman.
//// Distributed under the MIT License (http://opensource.org/licenses/MIT)
////
//
//#pragma once
//
////
//// Global registry functions
////
//#include "../details/registry.h"
//#include "../sinks/file_sinks.h"
//#include "../sinks/stdout_sinks.h"
//#include "../spdlog.h"
//#ifdef SPDLOG_ENABLE_SYSLOG
//#include "../sinks/syslog_sink.h"
//#endif
//
//#if defined _WIN32 && !defined(__cplusplus_winrt)
//#include "../sinks/wincolor_sink.h"
//#else
//#include "../sinks/ansicolor_sink.h"
//#endif
//
//#ifdef __ANDROID__
//#include "../sinks/android_sink.h"
//#endif
//
//#include <chrono>
//#include <functional>
//#include <memory>
//#include <string>
//
// inline void spdlog::register_logger(std::shared_ptr<logger> logger)
//{
//	return details::registry::instance().register_logger(std::move(logger));
//}
//
//
// inline void spdlog::drop(const std::string &name)
//{
//	details::registry::instance().drop(name);
//}
//
//// Create multi/single threaded simple file logger
// inline std::shared_ptr<spdlog::logger> spdlog::basic_logger_mt(const std::string &logger_name, const filename_t &filename, bool truncate)
//{
//	return create_and_register<sinks::simple_file_sink_mt>(logger_name, filename, truncate);
//}
//
// inline std::shared_ptr<spdlog::logger> spdlog::basic_logger_st(const std::string &logger_name, const filename_t &filename, bool truncate)
//{
//	return create_and_register<sinks::simple_file_sink_st>(logger_name, filename, truncate);
//}
//
//// Create multi/single threaded rotating file logger
// inline std::shared_ptr<spdlog::logger> spdlog::rotating_logger_mt(
//	const std::string &logger_name, const filename_t &filename, size_t max_file_size, size_t max_files)
//{
//	return create_and_register<sinks::rotating_file_sink_mt>(logger_name, filename, max_file_size, max_files);
//}
//
// inline std::shared_ptr<spdlog::logger> spdlog::rotating_logger_st(
//	const std::string &logger_name, const filename_t &filename, size_t max_file_size, size_t max_files)
//{
//	return create_and_register<sinks::rotating_file_sink_st>(logger_name, filename, max_file_size, max_files);
//}
//
//// Create file logger which creates new file at midnight):
// inline std::shared_ptr<spdlog::logger> spdlog::daily_logger_mt(
//	const std::string &logger_name, const filename_t &filename, int hour, int minute)
//{
//	return create_and_register<sinks::daily_file_sink_mt>(logger_name, filename, hour, minute);
//}
//
// inline std::shared_ptr<spdlog::logger> spdlog::daily_logger_st(
//	const std::string &logger_name, const filename_t &filename, int hour, int minute)
//{
//	return create_and_register<sinks::daily_file_sink_st>(logger_name, filename, hour, minute);
//}
//
////
//// stdout/stderr loggers
////
// inline std::shared_ptr<spdlog::logger> spdlog::stdout_logger_mt(const std::string &logger_name)
//{
//
//	auto new_logger = std::make_shared<logger>(logger_name, sinks::stdout_sink_mt::instance());
//	details::registry::instance().register_and_init(new_logger);
//	return new_logger;
//	//return details::registry::instance().create(logger_name, sinks::stdout_sink_mt::instance());
//}
//
// inline std::shared_ptr<spdlog::logger> spdlog::stdout_logger_st(const std::string &logger_name)
//{
//	auto new_logger = std::make_shared<logger>(logger_name, sinks::stdout_sink_st::instance());
//	details::registry::instance().register_and_init(new_logger);
//	return new_logger;
//}
//
// inline std::shared_ptr<spdlog::logger> spdlog::stderr_logger_mt(const std::string &logger_name)
//{
//	auto new_logger = std::make_shared<logger>(logger_name, sinks::stderr_sink_mt::instance());
//	details::registry::instance().register_and_init(new_logger);
//	return new_logger;
//}
//
// inline std::shared_ptr<spdlog::logger> spdlog::stderr_logger_st(const std::string &logger_name)
//{
//	auto new_logger = std::make_shared<logger>(logger_name, sinks::stderr_sink_st::instance());
//	details::registry::instance().register_and_init(new_logger);
//	return new_logger;
//}
//
////
//// stdout/stderr color loggers
////
//#if defined _WIN32 && !defined(__cplusplus_winrt)
//
// inline std::shared_ptr<spdlog::logger> spdlog::stdout_color_mt(const std::string &logger_name)
//{
//	return create_and_register<sinks::wincolor_stdout_sink_mt>(logger_name);
//}
//
// inline std::shared_ptr<spdlog::logger> spdlog::stdout_color_st(const std::string &logger_name)
//{
//	return create_and_register<sinks::wincolor_stdout_sink_st>(logger_name);
//}
//
// inline std::shared_ptr<spdlog::logger> spdlog::stderr_color_mt(const std::string &logger_name)
//{
//	return create_and_register<sinks::wincolor_stderr_sink_mt>(logger_name);
//}
//
// inline std::shared_ptr<spdlog::logger> spdlog::stderr_color_st(const std::string &logger_name)
//{
//
//	return create_and_register<sinks::wincolor_stderr_sink_st>(logger_name);
//}
//
//#else // ansi terminal colors
//
//
// inline std::shared_ptr<spdlog::logger> spdlog::stdout_color_mt(const std::string &logger_name)
//{
//	return create_and_register<sinks::ansicolor_stdout_sink_mt>(logger_name);
//}
//
// inline std::shared_ptr<spdlog::logger> spdlog::stdout_color_st(const std::string &logger_name)
//{
//	return create_and_register<sinks::ansicolor_stdout_sink_st>(logger_name);
//}
//
// inline std::shared_ptr<spdlog::logger> spdlog::stderr_color_mt(const std::string &logger_name)
//{
//	return create_and_register<sinks::ansicolor_stderr_sink_mt>(logger_name);
//}
//
// inline std::shared_ptr<spdlog::logger> spdlog::stderr_color_st(const std::string &logger_name)
//{
//	return create_and_register<sinks::ansicolor_stderr_sink_st>(logger_name);
//}
//#endif
//
//#ifdef SPDLOG_ENABLE_SYSLOG
//// Create syslog logger
// inline std::shared_ptr<spdlog::logger> spdlog::syslog_logger(
//	const std::string &logger_name, const std::string &syslog_ident, int syslog_option, int syslog_facility) {
//	return create_and_register<sinks::syslog_sink>(logger_name, syslog_ident, syslog_option, syslog_facility);
//}
//#endif
//
//#ifdef __ANDROID__
// inline std::shared_ptr<spdlog::logger> spdlog::android_logger(const std::string &logger_name, const std::string &tag)
//{
//	return create_and_register<sinks::android_sink>(logger_name, tag);
//}
//#endif
//
//
//
//
// inline void spdlog::flush_on(level::level_enum log_level)
//{
//	details::registry::instance().flush_on(log_level);
//}
//
// inline void spdlog::set_error_handler(log_err_handler handler)
//{
//	details::registry::instance().set_error_handler(std::move(handler));
//}
//
// inline void spdlog::apply_all(std::function<void(std::shared_ptr<logger>)> fun)
//{
//	details::registry::instance().apply_all(std::move(fun));
//}
//
// inline void spdlog::drop_all()
//{
//	details::registry::instance().drop_all();
//}
