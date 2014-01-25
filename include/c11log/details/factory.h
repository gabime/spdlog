#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>

namespace c11log {
class logger;
namespace details {
class factory {
public:
    typedef std::shared_ptr<c11log::logger>  logger_ptr;
    typedef std::unordered_map<std::string, logger_ptr> logger_map;
    logger_ptr get_logger(const std::string &name);
    static c11log::details::factory& instance();
private:
    logger_map _loggers;
    std::mutex _loggers_mutex;
};
}
}
