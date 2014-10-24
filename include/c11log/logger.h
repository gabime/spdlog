#pragma once

// Thread safe logger
// Has name, log level, vector of std::shared sink pointers and formatter
// Upon each log write the logger:
// 1. Checks if its log level is enough to log the message
// 2. Format the message using the formatter function
// 3. Pass the formatted message to its sinks to performa the actual logging

#include<vector>
#include<memory>
#include<atomic>
#include <sstream>
#include <exception>
#include "sinks/base_sink.h"
#include "common.h"

namespace c11log
{

namespace details
{
class line_logger;
}

class logger
{
public:

    logger(const std::string& name, sinks_init_list);
    template<class It>
    logger(const std::string& name, const It& begin, const It& end);

    void c11log::logger::set_format(const std::string& format);
    void set_formatter(formatter_ptr);
    formatter_ptr get_formatter() const;


    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;

    void set_level(level::level_enum);
    level::level_enum level() const;

    const std::string& name() const;
    bool should_log(level::level_enum) const;

    template <typename... Args> details::line_logger log(level::level_enum lvl, const Args&... args);
    template <typename... Args> details::line_logger trace(const Args&... args);
    template <typename... Args> details::line_logger debug(const Args&... args);
    template <typename... Args> details::line_logger info(const Args&... args);
    template <typename... Args> details::line_logger warn(const Args&... args);
    template <typename... Args> details::line_logger error(const Args&... args);
    template <typename... Args> details::line_logger critical(const Args&... args);


private:
    friend details::line_logger;
    std::string _name;
    formatter_ptr _formatter;
    std::vector<sink_ptr> _sinks;
    std::atomic_int _level;
    void _variadic_log(details::line_logger& l);
    template <typename First, typename... Rest>
    void _variadic_log(details::line_logger&l, const First& first, const Rest&... rest);
    void _log_msg(details::log_msg& msg);
};

class fflog_exception : public std::exception
{
public:
    fflog_exception(const std::string& msg) :_msg(msg) {};
    const char* what() const throw() override {
        return _msg.c_str();
    }
private:
    std::string _msg;

};

//
// Registry functions for easy loggers creation and retrieval
// example
// auto console_logger = c11log::create("my_logger", c11log::sinks<stdout_sink_mt>);
// auto same_logger = c11log::get("my_logger");
// auto file_logger = c11
//
std::shared_ptr<logger> get(const std::string& name);
std::shared_ptr<logger> create(const std::string& logger_name, sinks_init_list sinks);
template <typename Sink, typename... Args>
std::shared_ptr<c11log::logger> create(const std::string& logger_name, const Args&... args);
template<class It>
std::shared_ptr<logger> create(const std::string& logger_name, const It& sinks_begin, const It& sinks_end);

void formatter(formatter_ptr f);
formatter_ptr formatter();
void set_format(const std::string& format_string);
}

//
// Trace & debug macros
//
#ifdef FFLOG_ENABLE_TRACE
#define FFLOG_TRACE(logger, ...) logger->log(c11log::level::TRACE, __FILE__, " #", __LINE__,": " __VA_ARGS__)
#else
#define FFLOG_TRACE(logger, ...) {}
#endif

#ifdef FFLOG_ENABLE_DEBUG
#define FFLOG_DEBUG(logger, ...) logger->log(c11log::level::DEBUG, __VA_ARGS__)
#else
#define FFLOG_DEBUG(logger, ...) {}
#endif

#include "./details/logger_impl.h"