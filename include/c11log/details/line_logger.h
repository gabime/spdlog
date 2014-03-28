#pragma once

#include "../common_types.h"
#include "../logger.h"
#include "stack_oss.h"


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
        _log_msg(),
        _oss(),
        _enabled(enabled)
    {
        if(enabled)
        {
            _log_msg.when = log_clock::now();
            _log_msg.msg_level = msg_level;
            callback_logger->_formatter->format_header(callback_logger->_logger_name,
                    _log_msg.msg_level,
                    _log_msg.when,
                    _oss);
            _log_msg.header_size = _oss.size();
        }
    }

    // No copy intended. Only move
    line_logger(const line_logger& other) = delete;
    line_logger& operator=(const line_logger&) = delete;
    line_logger& operator=(line_logger&&) = delete;

    line_logger(line_logger&& other) :
        _callback_logger(other._callback_logger),
        _log_msg(other._log_msg),
        // The move ctor should only be called on start of logging line,
        // where no logging happened yet for this line so no need to copy the oss from the other
        _oss(),
        _enabled(other._enabled) {}


    ~line_logger()
    {
        if (_enabled)
        {
            _oss << os::eol();
            _log_msg.msg_buf = _oss.buf();
            _callback_logger->_log_it(_log_msg);
        }
    }


    template<typename T>
    line_logger&& operator<<(const T& what)
    {
        if (_enabled)
            _oss << what;

        return std::move(*this);
    }

private:
    logger* _callback_logger;
    c11log::log_msg _log_msg;
    details::stack_oss _oss;
    bool _enabled;


};
} //Namespace details
} // Namespace c11log
