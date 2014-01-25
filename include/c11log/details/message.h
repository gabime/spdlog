#pragma once

#include <chrono>
#include <string>
#include "../level.h"

namespace c11log {
namespace details {
struct message {
    message(const std::string& logger_name,
            level::level_enum log_level,
            const std::chrono::system_clock::time_point time_p
            , const std::string& msg) :
        logger_name(logger_name),
        log_level(log_level),
        time_p(time_p),
        msg(msg) {}

    ~message() = default;
    message(const message& other) = default;
    message& operator=(const message& rhs) = default;


    std::string logger_name;
    level::level_enum log_level;
    std::chrono::system_clock::time_point time_p;
    std::string msg;
};

}
}
