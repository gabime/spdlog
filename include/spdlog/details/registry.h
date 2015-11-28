//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

// Loggers registy of unique name->logger pointer
// An attempt to create a logger with an alreasy existing name will be ignored
// If user requests a non existing logger, nullptr will be returned
// This class is thread safe

#include <string>
#include <mutex>
#include <unordered_map>
#include <functional>

#include "./null_mutex.h"
#include "../logger.h"
#include "../async_logger.h"
#include "../common.h"

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
        register_logger_impl(logger);
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

        std::shared_ptr<logger> new_logger;

        std::lock_guard<Mutex> lock(_mutex);


        if (_async_mode)
            new_logger = std::make_shared<async_logger>(logger_name, sinks_begin, sinks_end, _async_q_size, _overflow_policy, _worker_warmup_cb, _flush_interval_ms);
        else
            new_logger = std::make_shared<logger>(logger_name, sinks_begin, sinks_end);

        if (_formatter)
            new_logger->set_formatter(_formatter);

        new_logger->set_level(_level);
        register_logger_impl(new_logger);
        return new_logger;
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

    void set_async_mode(size_t q_size, const async_overflow_policy overflow_policy, const std::function<void()>& worker_warmup_cb, const std::chrono::milliseconds& flush_interval_ms)
    {
        std::lock_guard<Mutex> lock(_mutex);
        _async_mode = true;
        _async_q_size = q_size;
        _overflow_policy = overflow_policy;
        _worker_warmup_cb = worker_warmup_cb;
        _flush_interval_ms = flush_interval_ms;
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
    void register_logger_impl(std::shared_ptr<logger> logger)
    {
        auto logger_name = logger->name();
        if (_loggers.find(logger_name) != std::end(_loggers))
            throw spdlog_ex("logger with name " + logger_name + " already exists");
        _loggers[logger->name()] = logger;
    }
    registry_t<Mutex>() {}
    registry_t<Mutex>(const registry_t<Mutex>&) = delete;
    registry_t<Mutex>& operator=(const registry_t<Mutex>&) = delete;
    Mutex _mutex;
    std::unordered_map <std::string, std::shared_ptr<logger>> _loggers;
    formatter_ptr _formatter;
    level::level_enum _level = level::info;
    bool _async_mode = false;
    size_t _async_q_size = 0;
    async_overflow_policy _overflow_policy = async_overflow_policy::block_retry;
    std::function<void()> _worker_warmup_cb = nullptr;
    std::chrono::milliseconds _flush_interval_ms;
};
#ifdef SPDLOG_NO_REGISTRY_MUTEX
typedef registry_t<spdlog::details::null_mutex> registry;
#else
typedef registry_t<std::mutex> registry;
#endif
}
}
