#pragma once

// blocking_queue:
// A blocking multi-consumer/multi-producer thread safe queue.
// Has max capacity and supports timeout on push or pop operations.

#include <cstddef>
#include <chrono>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace c11log {
namespace details
{
   
template<typename T>
class blocking_queue {
public:
    using queue_t = std::queue<T>;
    using size_type = typename queue_t::size_type;
    using clock = std::chrono::system_clock;

    explicit blocking_queue(size_type max_size) :max_size_(max_size), q_()
    {}
    blocking_queue(const blocking_queue&) = delete;
    blocking_queue& operator=(const blocking_queue&) = delete;    
    ~blocking_queue() = default;

    size_type size()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return q_.size();
    }

    // Push copy of item into the back of the queue. 
    // If the queue is full, block the calling thread util there is room or timeout have passed.
    // Return: false on timeout, true on successful push.
    template<typename Duration_Rep, typename Duration_Period, typename TT>
    bool push(TT&& item, const std::chrono::duration<Duration_Rep, Duration_Period>& timeout)
    {
        std::unique_lock<std::mutex> ul(mutex_);
        if (q_.size() >= max_size_)
        {
            if (!item_popped_cond_.wait_until(ul, clock::now() + timeout, [this]() { return this->q_.size() < this->max_size_; }))
                return false;
        }
        q_.push(std::forward<TT>(item));
        if (q_.size() <= 1)
        {
            ul.unlock(); //So the notified thread will have better chance to accuire the lock immediatly..
            item_pushed_cond_.notify_one();
        }
        return true;
    }

    // Push copy of item into the back of the queue. 
    // If the queue is full, block the calling thread until there is room.
    template<typename TT>
    void push(TT&& item)
    {
        while (!push(std::forward<TT>(item), one_hour));
    }

    // Pop a copy of the front item in the queue into the given item ref.
    // If the queue is empty, block the calling thread util there is item to pop or timeout have passed.
    // Return: false on timeout , true on successful pop/
    template<class Duration_Rep, class Duration_Period>
    bool pop(T& item, const std::chrono::duration<Duration_Rep, Duration_Period>& timeout)
    {
        std::unique_lock<std::mutex> ul(mutex_);
        if (q_.empty())
        {
            if (!item_pushed_cond_.wait_until(ul, clock::now() + timeout, [this]() { return !this->q_.empty(); }))
                return false;
        }
        item = std::move(q_.front());        
        q_.pop();
        if (q_.size() >= max_size_ - 1)
        {
            ul.unlock(); //So the notified thread will have better chance to accuire the lock immediatly..
            item_popped_cond_.notify_one();
        }
        return true;
    }

    // Pop a copy of the front item in the queue into the given item ref.
    // If the queue is empty, block the calling thread util there is item to pop.
    void pop(T& item)
    {
        while (!pop(item, one_hour));
    }

    // Clear the queue
    void clear()
    {
    	{
			std::unique_lock<std::mutex> ul(mutex_);
			q_ = queue_t{};
		}
		item_popped_cond_.notify_all();	    
    }

private:
    size_type max_size_;
    std::queue<T> q_;
    std::mutex mutex_;
    std::condition_variable item_pushed_cond_;
    std::condition_variable item_popped_cond_;         
    const std::chrono::hours one_hour{1};
};

}
}
