// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#    include <spdlog/async_logger.h>
#endif

#include <spdlog/sinks/sink.h>
#include <spdlog/details/thread_pool.h>

#include <memory>
#include <string>

SPDLOG_INLINE spdlog::async_logger::async_logger(
    std::string logger_name, sinks_init_list sinks_list, std::weak_ptr<details::thread_pool> tp, async_overflow_policy overflow_policy)
    : async_logger(std::move(logger_name), sinks_list.begin(), sinks_list.end(), std::move(tp), overflow_policy)
{}

SPDLOG_INLINE spdlog::async_logger::async_logger(
    std::string logger_name, sink_ptr single_sink, std::weak_ptr<details::thread_pool> tp, async_overflow_policy overflow_policy)
    : async_logger(std::move(logger_name), {std::move(single_sink)}, std::move(tp), overflow_policy)
{}

SPDLOG_INLINE spdlog::async_logger::async_logger(const async_logger& other) : logger(other)
{
    thread_pool_ = other.thread_pool_;
    overflow_policy_ = other.overflow_policy_;
    pending_log_count_ = other.pending_log_count_.load();
}

SPDLOG_INLINE size_t spdlog::async_logger::pending_log_count() const noexcept
{
    return pending_log_count_;
}

SPDLOG_INLINE void spdlog::async_logger::wait()
{
    if (pending_log_count_ > 0)
    {
        std::unique_lock<std::mutex> lock(wait_mutex_);
        wait_condition_.wait(lock);
    }
}

template< class Rep, class Period >
SPDLOG_INLINE std::cv_status spdlog::async_logger::wait_for(const std::chrono::duration<Rep, Period>& rel_time)
{
    if (pending_log_count_ > 0)
    {
        std::unique_lock<std::mutex> lock(wait_mutex_);
        return wait_condition_.wait_for(rel_time);
    }
}

template< class Clock, class Duration>
SPDLOG_INLINE std::cv_status spdlog::async_logger::wait_until(const std::chrono::time_point<Clock, Duration>& timeout_time)
{
    if (pending_log_count_ > 0)
    {
        std::unique_lock<std::mutex> lock(wait_mutex_);
        return wait_condition_.wait_until(timeout_time);
    }
}

SPDLOG_INLINE bool spdlog::async_logger::block_on_flush() const noexcept
{
    return block_on_flush_;
}

SPDLOG_INLINE void spdlog::async_logger::block_on_flush(bool value) noexcept
{
    block_on_flush_ = value;
}

// send the log message to the thread pool
SPDLOG_INLINE void spdlog::async_logger::sink_it_(const details::log_msg &msg)
{
    if (auto pool_ptr = thread_pool_.lock())
    {
        on_log_dispatched_();
        pool_ptr->post_log(shared_from_this(), msg, overflow_policy_);
    }
    else
    {
        throw_spdlog_ex("async log: thread pool doesn't exist anymore");
    }
}

// send flush request to the thread pool
SPDLOG_INLINE void spdlog::async_logger::flush_()
{
    if (auto pool_ptr = thread_pool_.lock())
    {
        on_log_dispatched_();
        pool_ptr->post_flush(shared_from_this(), overflow_policy_);

        /// this is to provide blocking functionality through logger(not async_logger) interface 
        if (block_on_flush_)
        {
            wait();
        }
    }
    else
    {
        throw_spdlog_ex("async flush: thread pool doesn't exist anymore");
    }
}

//
// backend functions - called from the thread pool to do the actual job
//
SPDLOG_INLINE void spdlog::async_logger::backend_sink_it_(const details::log_msg &msg)
{
    on_logged_();

    for (auto& sink : sinks_)
    {
        if (sink->should_log(msg.level))
        {
            SPDLOG_TRY
            {
                sink->log(msg);
            }
            SPDLOG_LOGGER_CATCH(msg.source)
        }
    }

    if (should_flush_(msg))
    {
        backend_flush_();
    }
}

SPDLOG_INLINE void spdlog::async_logger::backend_flush_()
{
    on_logged_();

    for (auto& sink : sinks_)
    {
        SPDLOG_TRY
        {
            sink->flush();
        }
        SPDLOG_LOGGER_CATCH(source_loc())
    }
}

SPDLOG_INLINE void spdlog::async_logger::on_log_dispatched_()
{
    ++pending_log_count_;
}

SPDLOG_INLINE void spdlog::async_logger::on_logged_()
{
    if (pending_log_count_ == 0)
    {
        throw_spdlog_ex("all dispatched messages by logger "
            + name_ + " have already been logged, but on_message_logged() was invoked");
    }

    pending_log_count_--;

    if (pending_log_count_ == 0)
    {
        wait_condition_.notify_all();
    }
}

SPDLOG_INLINE std::shared_ptr<spdlog::logger> spdlog::async_logger::clone(std::string new_name)
{
    auto cloned = std::make_shared<spdlog::async_logger>(*this);
    cloned->name_ = std::move(new_name);
    return cloned;
}
