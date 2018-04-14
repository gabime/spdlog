
//
// Copyright(c) 2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

//
// async logging using global thread pool
// all loggers created here share same global thread pool.
// each log message is pushed to a queue along withe a shared pointer to the logger.
// If a logger gets out of scope or deleted while having pending messages in the queue,
// it's destruction will defer until all its messages are processed by the thread pool.

#include "async_logger.h"
#include "details/registry.h"
#include "details/thread_pool.h"

#include <memory>
namespace spdlog {

// async logger factory- creates a-synchronous loggers
// creates a global thread pool with default queue size of 8192 items and single thread.
struct create_async
{
    template<typename Sink, typename... SinkArgs>
    static std::shared_ptr<async_logger> create(const std::string &logger_name, SinkArgs &&... args)
    {
        using details::registry;

        std::lock_guard<registry::MutexT> lock(registry::instance().tp_mutex());
        auto tp = registry::instance().get_thread_pool();
        if (tp == nullptr)
        {
            tp = std::make_shared<details::thread_pool>(8192, 1);
            registry::instance().set_thread_pool(tp);
        }

        auto sink = std::make_shared<Sink>(std::forward<SinkArgs>(args)...);
        auto new_logger = std::make_shared<async_logger>(logger_name, std::move(sink), std::move(tp), async_overflow_policy::block_retry);
        registry::instance().register_and_init(new_logger);
        return new_logger;
    }	
};

template<typename Sink, typename... SinkArgs>
inline std::shared_ptr<spdlog::logger> create_as(const std::string &logger_name, SinkArgs &&... sink_args)
{
    return create_async::create<Sink>(logger_name, std::forward<SinkArgs>(sink_args)...);
}

// set global thread pool. q_size must be power of 2
inline void init_thread_pool(size_t q_size, size_t thread_count)
{
    using details::registry;
    using details::thread_pool;
    auto tp = std::make_shared<thread_pool>(q_size, thread_count);
    registry::instance().set_thread_pool(std::move(tp));
}
} // namespace spdlog
