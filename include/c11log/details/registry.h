#pragma once
// Loggers registy of unique name->logger pointer
// If 2 loggers with same name are added, the last will be used
// If user requests a non existing logger, nullptr will be returned
// This class is thread safe

#include <string>
#include <mutex>
#include <unordered_map>
#include "../logger.h"
#include "../common.h"

namespace c11log {
namespace details {
class registry {
public:
    std::shared_ptr<logger> get(const std::string& name)
    {
        std::lock_guard<std::mutex> l(_mutex);
        auto found = _loggers.find(name);
        return found == _loggers.end() ? nullptr : found->second;
    }
    std::shared_ptr<logger> create(const std::string& logger_name, sinks_init_list sinks)
    {
        std::lock_guard<std::mutex> l(_mutex);
        return _loggers[logger_name] = std::make_shared<logger>(logger_name, sinks);
    }

    std::shared_ptr<logger> create(const std::string& logger_name, sink_ptr sink)
    {
        create(logger_name, { sink });
    }


    template<class It>
    std::shared_ptr<logger> create (const std::string& logger_name, const It& sinks_begin, const It& sinks_end)
    {
        std::lock_guard<std::mutex> l(_mutex);
        return _loggers[logger_name] = std::make_shared<logger>(logger_name, sinks_begin, sinks_end);
    }




    static registry& instance()
    {
        static registry s_instance;
        return s_instance;
    }

private:
    registry() = default;
    registry(const registry&) = delete;
    std::mutex _mutex;
    std::unordered_map <std::string, std::shared_ptr<logger>> _loggers;

};
}
}
