#pragma once

// Thread safe logger
// Has name, log level, vector of std::shared sink pointers and formatter
// Upon each log write the logger:
// 1. Checks if its log level is enough to log the message
// 2. Format the message using the formatter function
// 3. Pass the formatted message to it sinks to performa the actual logging

#include<vector>
#include<memory>
#include<atomic>
#include <sstream>

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
    using sink_ptr = std::shared_ptr<sinks::isink>;
    using sinks_vector_t = std::vector<sink_ptr>;
    using sinks_init_list = std::initializer_list<sink_ptr>;
    using formatter_ptr = std::unique_ptr<c11log::formatters::formatter>;

    logger(const std::string& name, sinks_init_list, formatter_ptr = nullptr);
    logger(const std::string& name, sink_ptr, formatter_ptr = nullptr);
    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;
    ~logger() = default;


    void level(c11log::level::level_enum);
    c11log::level::level_enum level() const;

    const std::string& name() const;
    bool should_log(c11log::level::level_enum) const;

    template<typename T> details::line_logger trace(const T&);
    template<typename T> details::line_logger debug(const T&);
    template<typename T> details::line_logger info(const T&);
    template<typename T> details::line_logger warn(const T&);
    template<typename T> details::line_logger error(const T&);
    template<typename T> details::line_logger critical(const T&);
    template<typename T> details::line_logger fatal(const T&);

    details::line_logger trace();
    details::line_logger debug();
    details::line_logger info();
    details::line_logger warn();
    details::line_logger error();
    details::line_logger critical();
    details::line_logger fatal();


private:
    friend details::line_logger;

    std::string _logger_name;
    formatter_ptr _formatter;
    sinks_vector_t _sinks;
    std::atomic_int _level;

    void _log_it(details::log_msg& msg);
};


//std::shared_ptr<c11log::logger> create_logger(const std::string& name, logger::sinks_init_list sinks, logger::formatter_ptr formatter = nullptr);
//std::shared_ptr<logger> get_logger(const std::string& name);

}

//
// Logger implementation
//

#include "details/line_logger.h"
#include "details/factory.h"

/*
inline std::shared_ptr<c11log::logger> c11log::create_logger(const std::string& name, logger::sinks_init_list sinks, logger::formatter_ptr formatter)
{
    return details::factory::instance().create_logger(name, sinks, std::move(formatter));
}
inline std::shared_ptr<c11log::logger> c11log::get_logger(const std::string& name)
{
    return details::factory::instance().get_logger(name);
}*/


inline c11log::logger::logger(const std::string& name, sinks_init_list sinks_list, formatter_ptr f) :
    _logger_name(name),
    _formatter(std::move(f)),
    _sinks(sinks_list)
{
    //Seems that vs2013 doesn't support std::atomic member initialization yet
    _level = level::INFO;
    if(!_formatter)
        _formatter = std::make_unique<formatters::default_formatter>();

}


inline c11log::logger::logger(const std::string& name, sink_ptr sink, formatter_ptr f) :
    logger(name, {sink}, std::move(f)) {}


template<typename T>
inline c11log::details::line_logger c11log::logger::trace(const T& msg)
{
    details::line_logger l(this, level::TRACE, should_log(level::TRACE));
    l.write(msg);
    return l;
}

template<typename T>
inline c11log::details::line_logger c11log::logger::debug(const T& msg)
{
    details::line_logger l(this, level::DEBUG, should_log(level::DEBUG));
    l.write(msg);
    return l;
}

template<typename T>
inline c11log::details::line_logger c11log::logger::info(const T& msg)
{
    details::line_logger l(this, level::INFO, should_log(level::INFO));
    l.write(msg);
    return l;
}

template<typename T>
inline c11log::details::line_logger c11log::logger::warn(const T& msg)
{
    details::line_logger l(this, level::WARNING, should_log(level::WARNING));
    l.write(msg);
    return l;
}

template<typename T>
inline c11log::details::line_logger c11log::logger::critical(const T& msg)
{
    details::line_logger l(this, level::CRITICAL, should_log(level::CRITICAL));
    l.write(msg);
    return l;
}

template<typename T>
inline c11log::details::line_logger c11log::logger::fatal(const T& msg)
{
    details::line_logger l(this, level::FATAL, should_log(level::FATAL));
    l.write(msg);
    return l;
}


inline c11log::details::line_logger c11log::logger::trace()
{
    return details::line_logger(this, level::TRACE, should_log(level::TRACE));
}

inline c11log::details::line_logger c11log::logger::debug()
{
    return details::line_logger(this, level::DEBUG, should_log(level::DEBUG));
}

inline c11log::details::line_logger c11log::logger::info()
{
    return details::line_logger(this, level::INFO, should_log(level::INFO));
}

inline c11log::details::line_logger c11log::logger::warn()
{
    return details::line_logger(this, level::WARNING, should_log(level::WARNING));
}

inline c11log::details::line_logger c11log::logger::critical()
{
    return details::line_logger(this, level::CRITICAL, should_log(level::CRITICAL));
}

inline c11log::details::line_logger c11log::logger::fatal()
{
    return details::line_logger(this, level::FATAL, should_log(level::FATAL));
}


inline const std::string& c11log::logger::name() const
{
    return _logger_name;
}

inline void c11log::logger::level(c11log::level::level_enum level)
{
    _level.store(level);
}

inline c11log::level::level_enum c11log::logger::level() const
{
    return static_cast<c11log::level::level_enum>(_level.load());
}

inline bool c11log::logger::should_log(c11log::level::level_enum level) const
{
    return level >= _level.load();
}

inline void c11log::logger::_log_it(details::log_msg& msg)
{
    _formatter->format(msg);
    for (auto &sink : _sinks)
        sink->log(msg);
}


