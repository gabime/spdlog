#pragma once
// Loggers registy of unique name->logger pointer
// An attempt to create a logger with an alreasy existing name will be ignored
// If user requests a non existing logger, nullptr will be returned
// This class is thread safe

#include <string>
#include <mutex>
#include <unordered_map>

#include "../logger.h"
#include "../common.h"

namespace spdlog
{
namespace details
{

class registry
{
public:
    std::shared_ptr<logger> get(const std::string& logger_name)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto found = _loggers.find(logger_name);
        return found == _loggers.end() ? nullptr : found->second;
    }

    template<class It>
    std::shared_ptr<logger> create(const std::string& logger_name, const It& sinks_begin, const It& sinks_end)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        //If already exists, just return it
        auto found = _loggers.find(logger_name);
        if (found != _loggers.end())
            return found->second;

        auto new_logger = std::make_shared<logger>(logger_name, sinks_begin, sinks_end);
        new_logger->set_formatter(_formatter);
        new_logger->set_level(_level);
        _loggers[logger_name] = new_logger;
        return new_logger;
    }


    std::shared_ptr<logger> create(const std::string& logger_name, sinks_init_list sinks)
    {
        return create(logger_name, sinks.begin(), sinks.end());
    }

    std::shared_ptr<logger> create(const std::string& logger_name, sink_ptr sink)
    {
        return create(logger_name, { sink });
    }


    void formatter(formatter_ptr f)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _formatter = f;
        for (auto& l : _loggers)
            l.second->set_formatter(_formatter);
    }


    void set_pattern(const std::string& pattern)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _formatter = std::make_shared<pattern_formatter>(pattern);
        for (auto& l : _loggers)
            l.second->set_formatter(_formatter);

    }

    void set_level(level::level_enum log_level)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& l : _loggers)
            l.second->set_level(log_level);

    }

    void close_all()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _level = level::OFF;
        for (auto& l : _loggers)
            l.second->close();
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
    level::level_enum _level = level::INFO;
};
}
}
