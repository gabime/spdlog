
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
    periodic_worker(const std::function<void()> &callback_fun, std::chrono::seconds interval)
    {
        active_ = (interval > std::chrono::seconds::zero());
        if (!active_)
        {
            return;
        }

        worker_thread_ = std::thread([this, callback_fun, interval]() {
            for (;;)
            {
                std::unique_lock<std::mutex> lock(this->mutex_);
                if (this->cv_.wait_for(lock, interval, [this] { return !this->active_; }))
                {
                    return; // active_ == false, so exit this thread
                }
                callback_fun();
            }
        });
    }

    periodic_worker(const periodic_worker &) = delete;
    periodic_worker &operator=(const periodic_worker &) = delete;

    // stop the worker thread and join it
    ~periodic_worker()
    {
        if (worker_thread_.joinable())
        {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                active_ = false;
            }
            cv_.notify_one();
            worker_thread_.join();
        }
    }

private:
    bool active_;
    std::thread worker_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
} // namespace details
} // namespace spdlog
