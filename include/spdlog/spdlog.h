/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2014 Gabi Melman.                                       */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/


// spdlog main header file.
//see example.cpp for usage example

#pragma once

#include "logger.h"

namespace spdlog
{

// Return an existing logger or nullptr if a logger with such name doesn't exist.
// Examples:
//
// spdlog::get("mylog")->info("Hello");
// auto logger = spdlog::get("mylog");
// logger.info("This is another message" , x, y, z);
// logger.info() << "This is another message" << x << y << z;
std::shared_ptr<logger> get(const std::string& name);


// Set global formatting
// spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e %l : %v");
void set_pattern(const std::string& format_string);


//Set global logging level
void set_level(level::level_enum log_level);


// Create multi/single threaded rotating file logger
std::shared_ptr<logger> rotating_logger_mt(const std::string& logger_name, const std::string& filename, size_t max_file_size, size_t max_files, size_t flush_inverval = 0);
std::shared_ptr<logger> rotating_logger_st(const std::string& logger_name, const std::string& filename, size_t max_file_size, size_t max_files, size_t flush_inverval = 0);

// Create file logger which creates new file at midnight):
std::shared_ptr<logger> daily_logger_mt(const std::string& logger_name, const std::string& filename, size_t flush_inverval = 0);
std::shared_ptr<logger> daily_logger_st(const std::string& logger_name, const std::string& filename, size_t flush_inverval = 0);


// Create stdout/stderr loggers
std::shared_ptr<logger> stdout_logger_mt(const std::string& logger_name);
std::shared_ptr<logger> stdout_logger_st(const std::string& logger_name);
std::shared_ptr<logger> stderr_logger_mt(const std::string& logger_name);
std::shared_ptr<logger> stderr_logger_st(const std::string& logger_name);


// Create a syslog logger
#ifdef __linux__
 std::shared_ptr<logger> syslog_logger(const std::string& logger_name);
#endif


//
// Create a logger with multiple sinks
//
std::shared_ptr<logger> create(const std::string& logger_name, sinks_init_list sinks);

template<class It>
std::shared_ptr<logger> create(const std::string& logger_name, const It& sinks_begin, const It& sinks_end);


// Create a logger with templated sink type
// Example: spdlog::create<daily_file_sink_st>("mylog", "dailylog_filename", "txt");
template <typename Sink, typename... Args>
std::shared_ptr<spdlog::logger> create(const std::string& logger_name, const Args&...);



// Set global formatter object
void set_formatter(formatter_ptr f);

// Stop logging by setting all the loggers to log level OFF
void stop();


//
// Trace macro enabled only at debug compile
// Example: SPDLOG_TRACE(my_logger, "Some trace message");
//
#ifdef _DEBUG
#define SPDLOG_TRACE(logger, ...) logger->log(__FILE__, " #", __LINE__,": " __VA_ARGS__)
#else
#define SPDLOG_TRACE(logger, ...) {}
#endif


}

#include "details/spdlog_impl.h"
