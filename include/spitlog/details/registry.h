#pragma once
// Loggers registy of unique name->logger pointer
// If 2 loggers with same name are added, the second will be overrun the first
// If user requests a non existing logger, nullptr will be returned
// This class is thread safe

#include <string>
#include <mutex>
#include <unordered_map>
#include "../logger.h"
#include "../common.h"

namespace spitlog {
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
        auto new_logger = std::make_shared<logger>(logger_name, sinks);
        new_logger->set_formatter(_formatter);
        _loggers[logger_name] = new_logger;
        return new_logger;
    }

    std::shared_ptr<logger> create(const std::string& logger_name, sink_ptr sink)
    {
        return create(logger_name, { sink });
    }


    template<class It>
    std::shared_ptr<logger> create (const std::string& logger_name, const It& sinks_begin, const It& sinks_end)
    {
        std::lock_guard<std::mutex> l(_mutex);
        auto new_logger = std::make_shared<logger>(logger_name, sinks_begin, sinks_end);
        new_logger->set_formatter(_formatter);
        _loggers[logger_name] = new_logger;
        return new_logger;

    }

    void formatter(formatter_ptr f)
    {
        _formatter = f;
    }

    formatter_ptr formatter()
    {
        return _formatter;
    }

    void set_format(const std::string& format_string)
    {
        _formatter = std::make_shared<pattern_formatter>(format_string);
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
    formatter_ptr _formatter;

};
}
}
