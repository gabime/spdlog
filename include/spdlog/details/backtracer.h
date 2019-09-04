// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/common.h"
#include "spdlog/details/log_msg_buffer.h"
#include "spdlog/details/circular_q.h"
#include <mutex>

// Store log messages in circular buffer.
// Useful for storing debug data in case of error/warning happens.

namespace spdlog {
    namespace details {
        class backtracer
        {
            mutable std::mutex mutex_;
            std::atomic<bool> enabled_ {false};      
            circular_q<log_msg_buffer> messages_;

        public:
            backtracer() = default;            
            backtracer(const backtracer& other)
            {
                std::lock_guard<std::mutex> lock(other.mutex_);
                enabled_ = other.enabled();
                messages_ = other.messages_;
            }

            backtracer(backtracer&& other) SPDLOG_NOEXCEPT             
            {
                std::lock_guard<std::mutex> lock(other.mutex_);
                enabled_ = other.enabled();
                messages_ = std::move(other.messages_);
            }

            backtracer& operator=(backtracer other)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                enabled_ = other.enabled();
                messages_ = other.messages_;
                return *this;
            }

            void enable(size_t size)
            {
                std::lock_guard<std::mutex> lock{ mutex_ };
                enabled_.store(true, std::memory_order_relaxed);
                messages_ = circular_q<log_msg_buffer>{ size };
            }


            void disable()
            {
                std::lock_guard<std::mutex> lock{ mutex_ };
                enabled_.store(false, std::memory_order_relaxed);
            }


            bool enabled() const
            {
                return enabled_.load(std::memory_order_relaxed);
            }

            explicit operator bool() const
            {
                return enabled();
            }

            void push_back(const log_msg &msg)
            {
                std::lock_guard<std::mutex> lock{ mutex_ };
                messages_.push_back(log_msg_buffer{ msg });
            }

            // pop all items in the q and apply the given fun on each of them.
            void foreach_pop(std::function<void(const details::log_msg &)> fun)
            {
                std::lock_guard<std::mutex> lock{ mutex_ };
                while (!messages_.empty())
                {
                    log_msg_buffer popped;
                    messages_.pop_front(popped);
                    fun(popped);
                }
            }
        };

    } // namespace details
} // namespace spdlog