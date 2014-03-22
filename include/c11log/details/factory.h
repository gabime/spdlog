#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>

namespace c11log
{
class logger;
namespace details
{
class factory
{
public:
    using logger_ptr = std::shared_ptr<c11log::logger>;
    using logger_map = std::unordered_map<std::string, logger_ptr>;
    void add_logger(const std::string& name, logger_ptr);
    logger_ptr get_logger(const std::string &name);
    static c11log::details::factory& instance();
private:
    std::mutex _loggers_mutex;
    logger_map _loggers;

};
}
}


inline void c11log::details::factory::add_logger(const std::string& name, logger_ptr logger_p)
{
    std::lock_guard<std::mutex> lock(_loggers_mutex);
    _loggers.insert(logger_map::value_type(name, logger_p));
}

inline c11log::details::factory::logger_ptr c11log::details::factory::get_logger(const std::string &name)
{
    std::lock_guard<std::mutex> lock(_loggers_mutex);

    auto found = _loggers.find(name);
    if (found != _loggers.end())
        return found->second;
    else
        return logger_ptr(nullptr);
    /*
    auto found = _loggers.find(name);

    if (found == _loggers.end()) {
        auto new_logger_ptr = std::make_shared<c11log::logger>(name);
        _loggers.insert(std::make_pair(name, new_logger_ptr));
        return new_logger_ptr;
    } else {
        return found->second;
    }*/
}

inline c11log::details::factory & c11log::details::factory::instance()
{
    static c11log::details::factory instance;
    return instance;
}
