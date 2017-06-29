//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

// Loggers registy of unique name->logger pointer
// An attempt to create a logger with an already existing name will be ignored
// If user requests a non existing logger, nullptr will be returned
// This class is thread safe

#include "spdlog/details/null_mutex.h"
#include "spdlog/logger.h"
#include "spdlog/async_logger.h"
#include "spdlog/common.h"

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace spdlog
{
namespace details
{
template <class Mutex> class registry_t
{
public:

    void register_logger(std::shared_ptr<logger> logger)
    {
        std::lock_guard<Mutex> lock(_mutex);
        auto logger_name = logger->name();
        throw_if_exists(logger_name);
        _loggers[logger_name] = logger;
    }


    std::shared_ptr<logger> get(const std::string& logger_name)
    {
        std::lock_guard<Mutex> lock(_mutex);
        auto found = _loggers.find(logger_name);
        return found == _loggers.end() ? nullptr : found->second;
    }

    template<class It>
    std::shared_ptr<logger> create(const std::string& logger_name, const It& sinks_begin, const It& sinks_end)
    {
        std::lock_guard<Mutex> lock(_mutex);
        throw_if_exists(logger_name);
        std::shared_ptr<logger> new_logger;
        if (_async_mode)
            new_logger = std::make_shared<async_logger>(logger_name, sinks_begin, sinks_end, _async_q_size, _overflow_policy, _worker_warmup_cb, _flush_interval_ms, _worker_teardown_cb);
        else
            new_logger = std::make_shared<logger>(logger_name, sinks_begin, sinks_end);

        if (_formatter)
            new_logger->set_formatter(_formatter);

        if (_err_handler)
            new_logger->set_error_handler(_err_handler);

        new_logger->set_level(_level);


        //Add to registry
        _loggers[logger_name] = new_logger;
        return new_logger;
    }

    template<class It>
    std::shared_ptr<async_logger> create_async(const std::string& logger_name, size_t queue_size, const async_overflow_policy overflow_policy, const std::function<void()>& worker_warmup_cb, const std::chrono::milliseconds& flush_interval_ms, const std::function<void()>& worker_teardown_cb, const It& sinks_begin, const It& sinks_end)
    {
        std::lock_guard<Mutex> lock(_mutex);
        throw_if_exists(logger_name);
        auto new_logger = std::make_shared<async_logger>(logger_name, sinks_begin, sinks_end, queue_size, overflow_policy, worker_warmup_cb, flush_interval_ms, worker_teardown_cb);

        if (_formatter)
            new_logger->set_formatter(_formatter);

        if (_err_handler)
            new_logger->set_error_handler(_err_handler);

        new_logger->set_level(_level);

        //Add to registry
        _loggers[logger_name] = new_logger;
        return new_logger;
    }

    void apply_all(std::function<void(std::shared_ptr<logger>)> fun)
    {
        std::lock_guard<Mutex> lock(_mutex);
        for (auto &l : _loggers)
            fun(l.second);
    }

    void drop(const std::string& logger_name)
    {
        std::lock_guard<Mutex> lock(_mutex);
        _loggers.erase(logger_name);
    }

    void drop_all()
    {
        std::lock_guard<Mutex> lock(_mutex);
        _loggers.clear();
    }
    std::shared_ptr<logger> create(const std::string& logger_name, sinks_init_list sinks)
    {
        return create(logger_name, sinks.begin(), sinks.end());
    }

    std::shared_ptr<logger> create(const std::string& logger_name, sink_ptr sink)
    {
        return create(logger_name, { sink });
    }

    std::shared_ptr<async_logger> create_async(const std::string& logger_name, size_t queue_size, const async_overflow_policy overflow_policy, const std::function<void()>& worker_warmup_cb, const std::chrono::milliseconds& flush_interval_ms, const std::function<void()>& worker_teardown_cb, sinks_init_list sinks)
    {
        return create_async(logger_name, queue_size, overflow_policy, worker_warmup_cb, flush_interval_ms, worker_teardown_cb, sinks.begin(), sinks.end());
    }

    std::shared_ptr<async_logger> create_async(const std::string& logger_name, size_t queue_size, const async_overflow_policy overflow_policy, const std::function<void()>& worker_warmup_cb, const std::chrono::milliseconds& flush_interval_ms, const std::function<void()>& worker_teardown_cb, sink_ptr sink)
    {
        return create_async(logger_name, queue_size, overflow_policy, worker_warmup_cb, flush_interval_ms, worker_teardown_cb, { sink });
    }

    void formatter(formatter_ptr f)
    {
        std::lock_guard<Mutex> lock(_mutex);
        _formatter = f;
        for (auto& l : _loggers)
            l.second->set_formatter(_formatter);
    }

    void set_pattern(const std::string& pattern)
    {
        std::lock_guard<Mutex> lock(_mutex);
        _formatter = std::make_shared<pattern_formatter>(pattern);
        for (auto& l : _loggers)
            l.second->set_formatter(_formatter);
    }

    void set_level(level::level_enum log_level)
    {
        std::lock_guard<Mutex> lock(_mutex);
        for (auto& l : _loggers)
            l.second->set_level(log_level);
        _level = log_level;
    }

    void set_error_handler(log_err_handler handler)
    {
        for (auto& l : _loggers)
            l.second->set_error_handler(handler);
        _err_handler = handler;
    }

    void set_async_mode(size_t q_size, const async_overflow_policy overflow_policy, const std::function<void()>& worker_warmup_cb, const std::chrono::milliseconds& flush_interval_ms, const std::function<void()>& worker_teardown_cb)
    {
        std::lock_guard<Mutex> lock(_mutex);
        _async_mode = true;
        _async_q_size = q_size;
        _overflow_policy = overflow_policy;
        _worker_warmup_cb = worker_warmup_cb;
        _flush_interval_ms = flush_interval_ms;
        _worker_teardown_cb = worker_teardown_cb;
    }

    void set_sync_mode()
    {
        std::lock_guard<Mutex> lock(_mutex);
        _async_mode = false;
    }

    static registry_t<Mutex>& instance()
    {
        static registry_t<Mutex> s_instance;
        return s_instance;
    }

private:
    registry_t<Mutex>() {}
    registry_t<Mutex>(const registry_t<Mutex>&) = delete;
    registry_t<Mutex>& operator=(const registry_t<Mutex>&) = delete;

    void throw_if_exists(const std::string &logger_name)
    {
        if (_loggers.find(logger_name) != _loggers.end())
            throw spdlog_ex("logger with name '" + logger_name + "' already exists");
    }
    Mutex _mutex;
    std::unordered_map <std::string, std::shared_ptr<logger>> _loggers;
    formatter_ptr _formatter;
    level::level_enum _level = level::info;
    log_err_handler _err_handler;
    bool _async_mode = false;
    size_t _async_q_size = 0;
    async_overflow_policy _overflow_policy = async_overflow_policy::block_retry;
    std::function<void()> _worker_warmup_cb = nullptr;
    std::chrono::milliseconds _flush_interval_ms;
    std::function<void()> _worker_teardown_cb = nullptr;
};
#ifdef SPDLOG_NO_REGISTRY_MUTEX
typedef registry_t<spdlog::details::null_mutex> registry;
#else
typedef registry_t<std::mutex> registry;
#endif
}
}
