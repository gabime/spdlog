#pragma once

#include "../level.h"
#include "fast_oss.h"

namespace c11log {
class logger;
namespace details {

class line_logger {
public:
    line_logger(logger* callback_logger, level::level_enum msg_level);
    line_logger(logger* callback_logger):_callback_logger(nullptr) {};
    line_logger(const line_logger&){};
    ~line_logger();

    template<typename T>
    line_logger& operator<<(const T& msg)
    {
        if (_callback_logger)
            _oss << msg;
        return *this;
    }

private:
    logger* _callback_logger;
    details::fast_oss _oss;

};
} //Namespace details
} // Namespace c11log
