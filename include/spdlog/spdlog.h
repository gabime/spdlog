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


// Example:
// auto logger = spdlog::create("mylog", {sink1, sink2});
std::shared_ptr<logger> create(const std::string& logger_name, sinks_init_list sinks);


// Example (create a logger with daily rotating file):
// using namespace spdlog::sinks;
// spdlog::create<daily_file_sink_st>("mylog", "dailylog_filename", "txt");
template <typename Sink, typename... Args>
std::shared_ptr<spdlog::logger> create(const std::string& logger_name, const Args&... args);

// Example:
// using namespace spdlog::sinks;
// std::vector<spdlog::sink_ptr> mySinks;
// mySinks.push_back(std::make_shared<rotating_file_sink_mt>("filename", "txt", 1024 * 1024 * 5, 10));
// mySinks.push_back(std::make_shared<stdout_sink_mt>());
// spdlog::create("mylog", mySinks.begin(), mySinks.end());
template<class It>
std::shared_ptr<logger> create(const std::string& logger_name, const It& sinks_begin, const It& sinks_end);


// Set global formatting
// Example:
// spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e %l : %t");
void set_pattern(const std::string& format_string);


// Set global formatter object
void set_formatter(formatter_ptr f);


//Set global active logging level
void set_level(level::level_enum log_level);


//Stop all loggers
void stop();


//
// Trace macro to turn on/off at compile time
// Example: SPDLOG_TRACE(my_logger, "Some trace message");
//
#ifdef _DEBUG
#define SPDLOG_TRACE(logger, ...) logger->log(__FILE__, " #", __LINE__,": " __VA_ARGS__)
#else
#define SPDLOG_TRACE(logger, ...) {}
#endif


}

#include "details/spdlog_impl.h"