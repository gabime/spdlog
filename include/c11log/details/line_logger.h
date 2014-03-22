#pragma once

#include "../common_types.h"
#include "../logger.h"
#include "fast_oss.h"


// line logger class. should be used by the logger as an rvalue only.
// aggregates logging string until the end of the line and then calls the logger upon destruction


namespace c11log
{
//class logger;
namespace details
{

class line_logger
{
public:
    line_logger(logger* callback_logger, level::level_enum msg_level, bool enabled):
        _callback_logger(callback_logger),
        _oss(),
        _level(msg_level),
        _enabled(enabled)
    {
        if(enabled)
        {
            callback_logger->_formatter->format_header(callback_logger->_logger_name,
                    msg_level,
                    log_clock::now(),
                    _oss);
        }
    }

    // No copy intended. Only move
    line_logger(const line_logger& other) = delete;
    line_logger& operator=(const line_logger&) = delete;
    line_logger& operator=(line_logger&&) = delete;

    // The move ctor should only be called on start of logging line,
    // where no logging happened yet for this line so no need to copy the string from the other
    line_logger(line_logger&& other) :
        _callback_logger(other._callback_logger),
        _oss(),
        _level(other._level),
        _enabled(other._enabled) {}


    ~line_logger()
    {
        if (_enabled)
        {
            _oss << os::eol();
            _callback_logger->_log_it(_oss.buf(), _level);
        }
    }


    template<typename T>
    line_logger&& operator<<(const T& msg)
    {
        if (_enabled)
            _oss << msg;
        return std::move(*this);
    }

private:
    logger* _callback_logger;
    details::fast_oss _oss;
    level::level_enum _level;
    bool _enabled;

};
} //Namespace details
} // Namespace c11log
