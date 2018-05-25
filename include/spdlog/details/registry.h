//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

// Loggers registy of unique name->logger pointer
// An attempt to create a logger with an already existing name will be ignored
// If user requests a non existing logger, nullptr will be returned
// This class is thread safe

#include "spdlog/common.h"
#include "spdlog/logger.h"

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace spdlog {
namespace details {
class thread_pool;

template<class Mutex>
class registry_t
{
public:
    using MutexT = Mutex;

    registry_t<Mutex>(const registry_t<Mutex> &) = delete;
    registry_t<Mutex> &operator=(const registry_t<Mutex> &) = delete;

    void register_logger(std::shared_ptr<logger> new_logger)
    {
        std::lock_guard<Mutex> lock(_mutex);
        auto logger_name = new_logger->name();
        throw_if_exists(logger_name);
        _loggers[logger_name] = new_logger;
    }

    void register_and_init(std::shared_ptr<logger> new_logger)
    {
        std::lock_guard<Mutex> lock(_mutex);
        auto logger_name = new_logger->name();
        throw_if_exists(logger_name);

        if (_formatter)
        {
            new_logger->set_formatter(_formatter);
        }

        if (_err_handler)
        {
            new_logger->set_error_handler(_err_handler);
        }

        new_logger->set_level(_level);
        new_logger->flush_on(_flush_level);

        // Add to registry
        _loggers[logger_name] = new_logger;
    }

    std::shared_ptr<logger> get(const std::string &logger_name)
    {
        std::lock_guard<Mutex> lock(_mutex);
        auto found = _loggers.find(logger_name);
        return found == _loggers.end() ? nullptr : found->second;
    }

    void set_thread_pool(std::shared_ptr<thread_pool> tp)
    {
        std::lock_guard<Mutex> lock(_mutex);
        _tp = std::move(tp);
    }

    std::shared_ptr<thread_pool> get_thread_pool()
    {
        std::lock_guard<Mutex> lock(_mutex);
        return _tp;
    }

    void set_formatter(formatter_ptr f)
    {
        std::lock_guard<Mutex> lock(_mutex);
        _formatter = f;
        for (auto &l : _loggers)
        {
            l.second->set_formatter(_formatter);
        }
    }

    void set_pattern(const std::string &pattern)
    {
        std::lock_guard<Mutex> lock(_mutex);
        _formatter = std::make_shared<pattern_formatter>(pattern);
        for (auto &l : _loggers)
        {
            l.second->set_formatter(_formatter);
        }
    }

    void set_level(level::level_enum log_level)
    {
        std::lock_guard<Mutex> lock(_mutex);
        for (auto &l : _loggers)
        {
            l.second->set_level(log_level);
        }
        _level = log_level;
    }

    void flush_on(level::level_enum log_level)
    {
        std::lock_guard<Mutex> lock(_mutex);
        for (auto &l : _loggers)
        {
            l.second->flush_on(log_level);
        }
        _flush_level = log_level;
    }

    void set_error_handler(log_err_handler handler)
    {
        for (auto &l : _loggers)
        {
            l.second->set_error_handler(handler);
        }
        _err_handler = handler;
    }

    void apply_all(std::function<void(std::shared_ptr<logger>)> fun)
    {
        std::lock_guard<Mutex> lock(_mutex);
        for (auto &l : _loggers)
        {
            fun(l.second);
        }
    }

    void drop(const std::string &logger_name)
    {
        std::lock_guard<Mutex> lock(_mutex);
        _loggers.erase(logger_name);
    }

    void drop_all()
    {
        {
            std::lock_guard<Mutex> lock(_mutex);
            _loggers.clear();
        }

        {
            std::lock_guard<Mutex> lock(_tp_mutex);
            _tp.reset();
        }
    }

    Mutex &tp_mutex()
    {
        return _tp_mutex;
    }

    static registry_t<Mutex> &instance()
    {
        static registry_t<Mutex> s_instance;
        return s_instance;
    }

private:
    registry_t<Mutex>() = default;

    void throw_if_exists(const std::string &logger_name)
    {
        if (_loggers.find(logger_name) != _loggers.end())
        {
            throw spdlog_ex("logger with name '" + logger_name + "' already exists");
        }
    }

    Mutex _mutex;
    Mutex _tp_mutex;
    std::unordered_map<std::string, std::shared_ptr<logger>> _loggers;
    formatter_ptr _formatter;
    level::level_enum _level = level::info;
    level::level_enum _flush_level = level::off;
    log_err_handler _err_handler;
    std::shared_ptr<thread_pool> _tp;
};

#ifdef SPDLOG_NO_REGISTRY_MUTEX
#include "spdlog/details/null_mutex.h"
using registry = registry_t<spdlog::details::null_mutex>;
#else
#include <mutex>
using registry = registry_t<std::mutex>;
#endif

} // namespace details
} // namespace spdlog
