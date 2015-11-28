//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

// Thread safe logger
// Has name, log level, vector of std::shared sink pointers and formatter
// Upon each log write the logger:
// 1. Checks if its log level is enough to log the message
// 2. Format the message using the formatter function
// 3. Pass the formatted message to its sinks to performa the actual logging

#include<vector>
#include<memory>
#include "sinks/base_sink.h"
#include "common.h"

namespace spdlog
{

namespace details
{
class line_logger;
}

class logger
{
public:
    logger(const std::string& logger_name, sink_ptr single_sink);
    logger(const std::string& name, sinks_init_list);
    template<class It>
    logger(const std::string& name, const It& begin, const It& end);

    virtual ~logger();
    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;

    void set_level(level::level_enum);
    level::level_enum level() const;

    const std::string& name() const;
    bool should_log(level::level_enum) const;

    // logger.info(cppformat_string, arg1, arg2, arg3, ...) call style
    template <typename... Args> details::line_logger trace(const char* fmt, const Args&... args);
    template <typename... Args> details::line_logger debug(const char* fmt, const Args&... args);
    template <typename... Args> details::line_logger info(const char* fmt, const Args&... args);
    template <typename... Args> details::line_logger notice(const char* fmt, const Args&... args);
    template <typename... Args> details::line_logger warn(const char* fmt, const Args&... args);
    template <typename... Args> details::line_logger error(const char* fmt, const Args&... args);
    template <typename... Args> details::line_logger critical(const char* fmt, const Args&... args);
    template <typename... Args> details::line_logger alert(const char* fmt, const Args&... args);
    template <typename... Args> details::line_logger emerg(const char* fmt, const Args&... args);


    // logger.info(msg) << ".." call style
    template <typename T> details::line_logger trace(const T&);
    template <typename T> details::line_logger debug(const T&);
    template <typename T> details::line_logger info(const T&);
    template <typename T> details::line_logger notice(const T&);
    template <typename T> details::line_logger warn(const T&);
    template <typename T> details::line_logger error(const T&);
    template <typename T> details::line_logger critical(const T&);
    template <typename T> details::line_logger alert(const T&);
    template <typename T> details::line_logger emerg(const T&);


    // logger.info() << ".." call  style
    details::line_logger trace();
    details::line_logger debug();
    details::line_logger info();
    details::line_logger notice();
    details::line_logger warn();
    details::line_logger error();
    details::line_logger critical();
    details::line_logger alert();
    details::line_logger emerg();



    // Create log message with the given level, no matter what is the actual logger's level
    template <typename... Args>
    details::line_logger force_log(level::level_enum lvl, const char* fmt, const Args&... args);

    // Set the format of the log messages from this logger
    void set_pattern(const std::string&);
    void set_formatter(formatter_ptr);

    virtual void flush();

protected:
    virtual void _log_msg(details::log_msg&);
    virtual void _set_pattern(const std::string&);
    virtual void _set_formatter(formatter_ptr);
    details::line_logger _log_if_enabled(level::level_enum lvl);
    template <typename... Args>
    details::line_logger _log_if_enabled(level::level_enum lvl, const char* fmt, const Args&... args);
    template<typename T>
    inline details::line_logger _log_if_enabled(level::level_enum lvl, const T& msg);


    friend details::line_logger;
    std::string _name;
    std::vector<sink_ptr> _sinks;
    formatter_ptr _formatter;
    std::atomic_int _level;

};
}

#include "./details/logger_impl.h"
