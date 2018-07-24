
//
// Copyright(c) 2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

// periodic worker thread - periodically executes the given callback function.
//
// RAII over the owned thread:
//    creates the thread on construction.
//    stops and joins the thread on destruction.

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace spdlog {
namespace details {

class periodic_worker
{
public:
    periodic_worker(std::function<void()> callback_fun, std::chrono::seconds interval)
    {
        if (interval == std::chrono::seconds::zero())
        {
            active_ = false;
            return;
        }
        active_ = true;
        flusher_thread_ = std::thread([callback_fun, interval, this]() {
            for (;;)
            {
                std::unique_lock<std::mutex> lock(this->mutex_);
                bool should_terminate = this->cv_.wait_for(lock, interval, [this] {
                    return !this->active_;
                });
                if (should_terminate)
                {
                    break;
                }
                callback_fun();
            }
        });
    }

    periodic_worker(const periodic_worker &) = delete;
    periodic_worker &operator=(const periodic_worker &) = delete;

    // stop the back thread and join it
    ~periodic_worker()
    {
        if (!active_)
        {
           return;
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            active_ = false;
        }
        cv_.notify_one();   
        flusher_thread_.join();
    }

private:
    bool active_;
    std::thread flusher_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
} // namespace details
} // namespace spdlog
