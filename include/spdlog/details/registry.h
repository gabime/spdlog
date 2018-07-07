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
        std::lock_guard<Mutex> lock(mutex_);
        auto logger_name = new_logger->name();
        throw_if_exists_(logger_name);
        loggers_[logger_name] = new_logger;
    }

    void register_and_init(std::shared_ptr<logger> new_logger)
    {
        std::lock_guard<Mutex> lock(mutex_);
        auto logger_name = new_logger->name();
        throw_if_exists_(logger_name);

        // create default formatter if not exists

        new_logger->set_formatter<pattern_formatter>(formatter_pattern_, pattern_time_type_);

        if (err_handler_)
        {
            new_logger->set_error_handler(err_handler_);
        }

        new_logger->set_level(level_);
        new_logger->flush_on(flush_level_);

        // Add to registry
        loggers_[logger_name] = new_logger;
    }

    std::shared_ptr<logger> get(const std::string &logger_name)
    {
        std::lock_guard<Mutex> lock(mutex_);
        auto found = loggers_.find(logger_name);
        return found == loggers_.end() ? nullptr : found->second;
    }

    void set_thread_pool(std::shared_ptr<thread_pool> tp)
    {
        std::lock_guard<Mutex> lock(mutex_);
        tp_ = std::move(tp);
    }

    std::shared_ptr<thread_pool> get_thread_pool()
    {
        std::lock_guard<Mutex> lock(mutex_);
        return tp_;
    }

    void set_pattern(const std::string &pattern, pattern_time_type time_type)
    {
        std::lock_guard<Mutex> lock(mutex_);
        formatter_pattern_ = pattern;
        pattern_time_type_ = time_type;
        for (auto &l : loggers_)
        {
            l.second->set_pattern(pattern, time_type);
        }
    }

    void set_level(level::level_enum log_level)
    {
        std::lock_guard<Mutex> lock(mutex_);
        for (auto &l : loggers_)
        {
            l.second->set_level(log_level);
        }
        level_ = log_level;
    }

    void flush_on(level::level_enum log_level)
    {
        std::lock_guard<Mutex> lock(mutex_);
        for (auto &l : loggers_)
        {
            l.second->flush_on(log_level);
        }
        flush_level_ = log_level;
    }

    void set_error_handler(log_err_handler handler)
    {
        for (auto &l : loggers_)
        {
            l.second->set_error_handler(handler);
        }
        err_handler_ = handler;
    }

    void apply_all(std::function<void(std::shared_ptr<logger>)> fun)
    {
        std::lock_guard<Mutex> lock(mutex_);
        for (auto &l : loggers_)
        {
            fun(l.second);
        }
    }

    void drop(const std::string &logger_name)
    {
        std::lock_guard<Mutex> lock(mutex_);
        loggers_.erase(logger_name);
    }

    void drop_all()
    {
        {
            std::lock_guard<Mutex> lock(mutex_);
            loggers_.clear();
        }

        {
            std::lock_guard<Mutex> lock(tp_mutex_);
            tp_.reset();
        }
    }

    Mutex &tp_mutex()
    {
        return tp_mutex_;
    }

    static registry_t<Mutex> &instance()
    {
        static registry_t<Mutex> s_instance;
        return s_instance;
    }

private:
    registry_t<Mutex>() = default;

    void throw_if_exists_(const std::string &logger_name)
    {
        if (loggers_.find(logger_name) != loggers_.end())
        {
            throw spdlog_ex("logger with name '" + logger_name + "' already exists");
        }
    }

    Mutex mutex_;
    Mutex tp_mutex_;
    std::unordered_map<std::string, std::shared_ptr<logger>> loggers_;
    std::string formatter_pattern_ = "%+";
    pattern_time_type pattern_time_type_ = pattern_time_type::local;
    level::level_enum level_ = level::info;
    level::level_enum flush_level_ = level::off;
    log_err_handler err_handler_;
    std::shared_ptr<thread_pool> tp_;
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
