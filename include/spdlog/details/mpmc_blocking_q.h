#pragma once

//
// Copyright(c) 2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

// async log helper :
// multi producer-multi consumer blocking queue
// enqueue(..) - will block until room found to put the new message
// enqueue_nowait(..) - will return immediatly with false if no room left in the queue
// dequeue_for(..) - will block until the queue is not empty or timeout passed

#include <condition_variable>
#include <mutex>
#include <queue>

namespace spdlog {
namespace details {

template<typename T>
class mpmc_bounded_queue
{
public:
    using item_type = T;
    explicit mpmc_bounded_queue(size_t max_items)
        : max_items_(max_items)
    {
    }

    // try to enqueue and block if no room left
    void enqueue(T &&item)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            pop_cv_.wait(lock, [this] { return this->q_.size() < this->max_items_; });
            q_.push(std::move(item));
        }
        push_cv_.notify_one();
    }

    // try to enqueue and return immdeialty false if no room left
    bool enqueue_nowait(T &&item)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (q_.size() == this->max_items_)
            {
                return false;
            }
            q_.push(std::forward<T>(item));
        }
        push_cv_.notify_one();
        return true;
    }

    // try to dequeue item. if no item found. wait upto timeout and try again
    // Return true, if succeeded dequeue item, false otherwise
    bool dequeue_for(T &popped_item, std::chrono::milliseconds wait_duration)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (!push_cv_.wait_for(lock, wait_duration, [this] { return this->q_.size() > 0; }))
            {
                return false;
            }

            popped_item = std::move(q_.front());
            q_.pop();
        }
        pop_cv_.notify_one();
        return true;
    }

private:
    size_t max_items_;
    std::mutex queue_mutex_;
    std::condition_variable push_cv_;
    std::condition_variable pop_cv_;

    std::queue<T> q_;
};
} // namespace details
} // namespace spdlog
