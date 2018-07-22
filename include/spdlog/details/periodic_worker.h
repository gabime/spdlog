
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

#include <atomic>
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
            using std::chrono::steady_clock;

            auto last_flush_tp = steady_clock::now();
            for (;;)
            {
                std::unique_lock<std::mutex> lock(this->mutex_);
                this->cv_.wait_for(lock, interval, [callback_fun, interval, last_flush_tp, this] {
                    return !this->active_ || (steady_clock::now() - last_flush_tp) >= interval;
                });
                if (!this->active_)
                {
                    break;
                }
                callback_fun();
                last_flush_tp = steady_clock::now();
            }
        });
    }

    periodic_worker(const periodic_worker &) = delete;
    periodic_worker &operator=(const periodic_worker &) = delete;

    // stop the back thread and join it
    ~periodic_worker()
    {
        if (active_)
        {
            active_ = false;
            cv_.notify_one();
            flusher_thread_.join();
        }
    }

private:
    std::atomic<bool> active_;
    std::thread flusher_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
} // namespace details
} // namespace spdlog
