#pragma once

// Thread safe logger
// Has log level and vector sinks which do the actual logging
#include<vector>
#include<memory>
#include<mutex>
#include<atomic>
#include <algorithm>

#include "common_types.h"
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

    explicit logger(const std::string& name) :
        _logger_name(name),
        _formatter(new formatters::default_formatter()),
        _sinks(),
        _mutex() {
        //Seems that vs2013 doesnt support atomic member initialization in ctor, so its done here
        _atomic_level = level::INFO;
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

    std::string _logger_name = "";
    std::unique_ptr<c11log::formatters::formatter> _formatter;
    sinks_vector_t _sinks;
    std::mutex _mutex;
    std::atomic_int _atomic_level;

    void _log_it(const std::string& msg, const level::level_enum level);

};

logger& get_logger(const std::string& name);

}


//
// Logger inline impl
//
#include "details/line_logger.h"
inline c11log::details::line_logger c11log::logger::log(c11log::level::level_enum msg_level)
{
    return details::line_logger(this, msg_level, msg_level >= _atomic_level);
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
    std::lock_guard<std::mutex> lock(_mutex);
    _logger_name = name;
}

inline const std::string& c11log::logger::get_name()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _logger_name;
}

inline void c11log::logger::add_sink(sink_ptr_t sink_ptr)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _sinks.push_back(sink_ptr);
}

inline void c11log::logger::remove_sink(sink_ptr_t sink_ptr)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _sinks.erase(std::remove(_sinks.begin(), _sinks.end(), sink_ptr), _sinks.end());
}

inline void c11log::logger::set_formatter(std::unique_ptr<formatters::formatter> formatter)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _formatter = std::move(formatter);
}

inline void c11log::logger::set_level(c11log::level::level_enum level)
{
    _atomic_level.store(level);
}

inline c11log::level::level_enum c11log::logger::get_level() const
{
    return static_cast<c11log::level::level_enum>(_atomic_level.load());
}

inline bool c11log::logger::should_log(c11log::level::level_enum level) const
{
    return level >= _atomic_level.load();
}
inline void c11log::logger::_log_it(const std::string& msg, const level::level_enum level)
{
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto &sink : _sinks)
        sink->log(msg, level);
}

// Static factory function
inline c11log::logger& c11log::get_logger(const std::string& name)
{
    return *(c11log::details::factory::instance().get_logger(name));
}

