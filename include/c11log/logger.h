#pragma once

// Thread safe logger
// Has log level and vector sinks which do the actual logging
#include<vector>
#include<memory>
#include<mutex>
#include<atomic>
#include <algorithm>

#include "level.h"
#include "sinks/base_sink.h"
#include "details/factory.h"

namespace c11log
{
namespace details
{
class line_logger;
}

class logger
{
public:

	typedef std::shared_ptr<sinks::base_sink>  sink_ptr_t;
	typedef std::vector<sink_ptr_t> sinks_vector_t;

	explicit logger(const std::string& name) : logger_name_(name),
		formatter_(std::make_unique<formatters::default_formatter>()) {
		atomic_level_.store(level::INFO);
	}

	~logger() = default;

	logger(const logger&) = delete;
	logger& operator=(const logger&) = delete;

	void set_name(const std::string& name);
	const std::string& get_name();
	void add_sink(sink_ptr_t sink_ptr);
	void remove_sink(sink_ptr_t sink_ptr);
	void set_formatter(std::unique_ptr<formatters::formatter> formatter);
	void set_level(c11log::level::level_enum level);
	c11log::level::level_enum get_level() const;
	bool should_log(c11log::level::level_enum level) const;

	details::line_logger log(level::level_enum level);
	details::line_logger debug();
	details::line_logger info();
	details::line_logger warn();
	details::line_logger error();
	details::line_logger fatal();

private:
	friend details::line_logger;


	std::string logger_name_ = "";
	std::unique_ptr<c11log::formatters::formatter> formatter_;
	sinks_vector_t sinks_;
	std::mutex mutex_;
	std::atomic_int atomic_level_;

	void log_it_(const std::string& msg);

};

logger& get_logger(const std::string& name);
}

#include "details/line_logger.h"
//
// Logger inline impl
//
inline c11log::details::line_logger c11log::logger::log(c11log::level::level_enum msg_level)
{

	if (msg_level >= atomic_level_) {
		return details::line_logger(this, msg_level);
	} else {
		return details::line_logger(nullptr);
	}

}

inline c11log::details::line_logger c11log::logger::debug()
{
	return log(c11log::level::DEBUG);
}
inline c11log::details::line_logger c11log::logger::info()
{
	return log(c11log::level::INFO);
}
inline c11log::details::line_logger c11log::logger::warn()
{
	return log(c11log::level::WARNING);
}
inline c11log::details::line_logger c11log::logger::error()
{
	return log(level::ERROR);
}
inline c11log::details::line_logger c11log::logger::fatal()
{
	return log(c11log::level::FATAL);
}

inline void c11log::logger::set_name(const std::string& name)
{
	std::lock_guard<std::mutex> lock(mutex_);
	logger_name_ = name;
}

inline const std::string& c11log::logger::get_name()
{
	std::lock_guard<std::mutex> lock(mutex_);
	return logger_name_;
}

inline void c11log::logger::add_sink(sink_ptr_t sink_ptr)
{
	std::lock_guard<std::mutex> lock(mutex_);
	sinks_.push_back(sink_ptr);
}

inline void c11log::logger::remove_sink(sink_ptr_t sink_ptr)
{
	std::lock_guard<std::mutex> lock(mutex_);
	sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), sink_ptr), sinks_.end());
}

inline void c11log::logger::set_formatter(std::unique_ptr<formatters::formatter> formatter)
{
	std::lock_guard<std::mutex> lock(mutex_);
	formatter_ = std::move(formatter);
}

inline void c11log::logger::set_level(c11log::level::level_enum level)
{
	atomic_level_.store(level);
}

inline c11log::level::level_enum c11log::logger::get_level() const
{
	return static_cast<c11log::level::level_enum>(atomic_level_.load());
}

inline bool c11log::logger::should_log(c11log::level::level_enum level) const
{
	return level >= atomic_level_.load();
}
inline void c11log::logger::log_it_(const std::string& msg)
{
	level::level_enum level = static_cast<level::level_enum>(atomic_level_.load());
	std::lock_guard<std::mutex> lock(mutex_);
	for (auto &sink : sinks_)
		sink->log(msg, level);
}

// Static factory function
inline c11log::logger& c11log::get_logger(const std::string& name)
{
	return *(c11log::details::factory::instance().get_logger(name));
}
