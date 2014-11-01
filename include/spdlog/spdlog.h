//
// This is spdlog - an extremely fast and easy to use c++11 logging library
//

// example code (create multi threaded daily logger):
// auto my_logger = spdlog::create<daily_file_sink_st>("mylog", "dailylog_filename", "txt");
// ..
// auto mylog = spdlog::get("mylog");
// mylog->info("Hello logger.", "This is message number", 1, "!!") ;
// mylog->info() << "std streams are also supprted: " << std::hex << 255;

// see example.cpp for more examples

#pragma once

#include "logger.h"
#include "details/registry.h"

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



//
// Create a logger with multiple sinks
//
std::shared_ptr<logger> create(const std::string& logger_name, sinks_init_list sinks);

template<class It>
std::shared_ptr<logger> create(const std::string& logger_name, const It& sinks_begin, const It& sinks_end);


// Create a logger with templated sink type
// Example: spdlog::create<daily_file_sink_st>("mylog", "dailylog_filename", "txt");
template <typename Sink, typename... Args>
std::shared_ptr<spdlog::logger> create(const std::string& logger_name, const Args&... args);



// Set global formatter object
void set_formatter(formatter_ptr f);

// Close all loggers and stop logging
void close();


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