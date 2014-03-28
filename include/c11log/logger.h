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


//Thread safe, fast logger.
//All initialization is done in ctor only, so we get away lot of locking
namespace c11log
{

namespace details
{
class line_logger;
template<std::size_t> class fast_buf;
}


class logger
{
public:

    using sink_ptr = std::shared_ptr<sinks::base_sink>;
    using formatter_ptr = std::shared_ptr<c11log::formatters::formatter>;
    using sinks_vector_t = std::vector<sink_ptr>;
    using sinks_init_list = std::initializer_list<sink_ptr>;

    logger(const std::string& name, formatter_ptr, sinks_init_list);
    logger(const std::string& name, sinks_init_list);
    logger(sinks_init_list sinks_list);


    ~logger() = default;

    //Non copybale in anyway
    logger(const logger&) = delete;
    logger(logger&&) = delete;
    logger& operator=(const logger&) = delete;
    logger& operator=(logger&&) = delete;

    void set_level(c11log::level::level_enum);
    c11log::level::level_enum get_level() const;

    const std::string& get_name() const;
    bool should_log(c11log::level::level_enum) const;

    details::line_logger log(level::level_enum);
    details::line_logger debug();
    details::line_logger info();
    details::line_logger warn();
    details::line_logger error();
    details::line_logger fatal();

private:
    friend details::line_logger;

    std::string _logger_name = "";
    formatter_ptr _formatter;
    sinks_vector_t _sinks;
    std::atomic_int _logger_level;

    void _log_it(const log_msg& msg);

};

logger& get_logger(const std::string& name);

}

//
// Logger inline implementation
//
#include "details/line_logger.h"


inline c11log::logger::logger(const std::string& name, formatter_ptr f, sinks_init_list sinks_list) :
    _logger_name(name),
    _formatter(f),
    _sinks(sinks_list)
{
    //Seems that vs2013 doesnt support std::atomic member initialization, so its done here
    _logger_level = level::INFO;
}

inline c11log::logger::logger(const std::string& name, sinks_init_list sinks_list) :
    logger(name, std::make_shared<formatters::default_formatter>(), sinks_list) {}

inline c11log::logger::logger(sinks_init_list sinks_list) :
    logger("", std::make_shared<formatters::default_formatter>(), sinks_list) {}


inline c11log::details::line_logger c11log::logger::log(c11log::level::level_enum msg_level)
{
    return details::line_logger(this, msg_level, msg_level >= _logger_level);
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


inline const std::string& c11log::logger::get_name() const
{
    return _logger_name;
}


inline void c11log::logger::set_level(c11log::level::level_enum level)
{
    _logger_level.store(level);
}

inline c11log::level::level_enum c11log::logger::get_level() const
{
    return static_cast<c11log::level::level_enum>(_logger_level.load());
}

inline bool c11log::logger::should_log(c11log::level::level_enum level) const
{
    return level >= _logger_level.load();
}

inline void c11log::logger::_log_it(const log_msg& msg)
{
    for (auto &sink : _sinks)
        sink->log(msg);
}

// Static factory function

inline c11log::logger& c11log::get_logger(const std::string& name)
{
    return *(c11log::details::factory::instance().get_logger(name));
}
