#pragma once

#include <sstream>
#include "../common.h"
#include "../logger.h"
#include "fast_oss.h"


// Line logger class - aggregates operator<< calls to fast ostream
// and logs upon destruction

namespace c11log
{
namespace details
{
class line_logger
{
public:
    line_logger(logger* callback_logger, level::level_enum msg_level, bool enabled):
        _callback_logger(callback_logger),
        _log_msg(msg_level),
        _enabled(enabled)
    {}

    // No copy intended. Only move
    line_logger(const line_logger& other) = delete;
    line_logger& operator=(const line_logger&) = delete;
    line_logger& operator=(line_logger&&) = delete;


    line_logger(line_logger&& other) :
        _callback_logger(other._callback_logger),
        _log_msg(std::move(other._log_msg)),
        _enabled(other._enabled)
    {
        other.disable();
    }

    //Log the log message using the callback logger
    ~line_logger()
    {
        if (_enabled)
        {
            _log_msg.logger_name = _callback_logger->name();
            _log_msg.time = log_clock::now();
            _log_msg.tm_time = details::os::localtime(log_clock::to_time_t(_log_msg.time));
            _callback_logger->_log_msg(_log_msg);
        }
    }

    template<typename T>
    void write(const T& what)
    {
        if (_enabled)
        {
            _log_msg.raw << what;
        }
    }

    template<typename T>
    line_logger& operator<<(const T& what)
    {
        write(what);
        return *this;
    }

    void disable()
    {
        _enabled = false;
    }



private:
    logger* _callback_logger;
    log_msg _log_msg;
    bool _enabled;
};
} //Namespace details
} // Namespace c11log
