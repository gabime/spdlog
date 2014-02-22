#pragma once

#include "../level.h"
#include "../logger.h"
#include "fast_oss.h"

namespace c11log {
class logger;
namespace details {

class line_logger {
public:
    line_logger(logger* callback_logger, level::level_enum msg_level):
        _callback_logger(callback_logger),
        _oss(),
        _level(msg_level) {
        callback_logger->_formatter->format_header(callback_logger->_logger_name,
                msg_level,
                c11log::formatters::clock::now(),
                _oss);
    }
    line_logger(logger*):_callback_logger(nullptr) {};
    line_logger(const line_logger& other):
        _callback_logger(other._callback_logger),
        _oss(other._oss),
        _level(other._level) {};

    line_logger& operator=(const line_logger&) = delete;

    ~line_logger() {
        if (_callback_logger) {
            _oss << '\n';
            _callback_logger->_log_it(_oss.str_ref());
        }
    }

    template<typename T>
    line_logger& operator<<(const T& msg) {
        if (_callback_logger)
            _oss << msg;
        return *this;
    }

private:
    logger* _callback_logger;
    details::fast_oss _oss;
    level::level_enum _level;

};
} //Namespace details
} // Namespace c11log
