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

#include <spdlog/sinks/base_sink.h>
#include <spdlog/common.h>

#include <vector>
#include <memory>
#include <string>

namespace spdlog
{

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

	// Have problems to make it work with the current buildin version of the format library. Seems to work 
	// with the latest version. Cancelt at the moment.
//	template <typename... Args> void printf(level::level_enum lvl, const fmt_formatchar_t* fmt, fmt::ArgList& argList);

    template <typename... Args> void log(level::level_enum lvl, const fmt_formatchar_t* fmt, const Args&... args);
	template <typename... Args> void log(level::level_enum lvl, const fmt_formatchar_t* fmt, fmt::ArgList& argList);
	template <typename... Args> void log(level::level_enum lvl, const log_char_t* msg);
    template <typename... Args> void trace(const fmt_formatchar_t* fmt, const Args&... args);
    template <typename... Args> void debug(const fmt_formatchar_t* fmt, const Args&... args);
    template <typename... Args> void info(const fmt_formatchar_t* fmt, const Args&... args);
    template <typename... Args> void warn(const fmt_formatchar_t* fmt, const Args&... args);
    template <typename... Args> void error(const fmt_formatchar_t* fmt, const Args&... args);
    template <typename... Args> void critical(const fmt_formatchar_t* fmt, const Args&... args);

    template <typename T> void log(level::level_enum lvl, const T&);
    template <typename T> void trace(const T&);
    template <typename T> void debug(const T&);
    template <typename T> void info(const T&);
    template <typename T> void warn(const T&);
    template <typename T> void error(const T&);
    template <typename T> void critical(const T&);

    bool should_log(level::level_enum) const;
    void set_level(level::level_enum);
	level::level_enum get_level(void) const;
    level::level_enum level() const;
    const std::string& name() const;
    void set_pattern(const fmt_formatstring_t&);
    void set_formatter(formatter_ptr);

    // error handler
    void set_error_handler(log_err_handler);
    log_err_handler error_handler();

    // automatically call flush() if message level >= log_level
    void flush_on(level::level_enum log_level);
	level::level_enum get_flush_on(void) const;

    virtual void flush();

#ifdef SPDLOG_BITMASK_LOG_FILTER
	void set_enable_bit_mask(unsigned long mask);
	unsigned long get_enable_bit_mask();
	bool should_log_b(unsigned long BitFlag);
	bool should_log_bl(unsigned long BitFlag, level::level_enum level);
	//template <typename... Args> void printf(level::level_enum lvl, unsigned long BitFlag, const fmt_formatchar_t* fmt, fmt::ArgList& argList);
	//template <typename... Args> void printf(unsigned long BitFlag, const fmt_formatchar_t* fmt, fmt::ArgList& argList);
	template <typename... Args> void log(level::level_enum lvl, unsigned long BitFlag, const fmt_formatchar_t* fmt, const Args&... args);
	template <typename... Args> void log(level::level_enum lvl, unsigned long BitFlag, const fmt_formatchar_t* fmt, fmt::ArgList& argList);
	template <typename... Args> void log(level::level_enum lvl, unsigned long BitFlag, const log_char_t* msg);
	template <typename T> void log(level::level_enum lvl, unsigned long BitFlag, const T&);
	template <typename... Args> void log(unsigned long BitFlag, const fmt_formatchar_t* fmt, const Args&... args);
	template <typename... Args> void log(unsigned long BitFlag, const fmt_formatchar_t* fmt, fmt::ArgList& argList);
	template <typename... Args> void log(unsigned long BitFlag, const log_char_t* msg);
	template <typename T> void log(unsigned long BitFlag, const T&);
#endif

protected:
    virtual void _sink_it(details::log_msg&);
    virtual void _set_pattern(const fmt_formatstring_t&);
    virtual void _set_formatter(formatter_ptr);

    // default error handler: print the error to stderr with the max rate of 1 message/minute
    virtual void _default_err_handler(const std::string &msg);

    // return true if the given message level should trigger a flush
    bool _should_flush_on(const details::log_msg&);

    const std::string _name;
    std::vector<sink_ptr> _sinks;
    formatter_ptr _formatter;
    spdlog::level_t _level;
    spdlog::level_t _flush_level;
    log_err_handler _err_handler;
    std::atomic<time_t> _last_err_time;

	#ifdef SPDLOG_BITMASK_LOG_FILTER
	unsigned long _log_enable_bit_mask;
	#endif

};
}

#include <spdlog/details/logger_impl.h>


