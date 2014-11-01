/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2014 Gabi Melman.                                       */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

// A blocking multi-consumer/multi-producer thread safe queue.
// Has max capacity and supports timeout on push or pop operations.

#include <chrono>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace spdlog
{
namespace details
{

template<typename T>
class blocking_queue
{
public:
    using queue_type = std::queue<T>;
    using item_type = T;
    using size_type = typename queue_type::size_type;
    using clock = std::chrono::system_clock;

    explicit blocking_queue(size_type max_size) :
        _max_size(max_size),
        _q(),
        _mutex()
    {
    }
    blocking_queue(const blocking_queue&) = delete;
    blocking_queue& operator=(const blocking_queue&) = delete;
    ~blocking_queue() = default;

    size_type size()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _q.size();
    }

    // Push copy of item into the back of the queue.
    // If the queue is full, block the calling thread util there is room or timeout have passed.
    // Return: false on timeout, true on successful push.
    template<typename Duration_Rep, typename Duration_Period, typename TT>
    bool push(TT&& item, const std::chrono::duration<Duration_Rep, Duration_Period>& timeout)
    {
        std::unique_lock<std::mutex> ul(_mutex);
        if (_q.size() >= _max_size)
        {
            if (!_item_popped_cond.wait_until(ul, clock::now() + timeout,[this]()
        {
            return this->_q.size() < this->_max_size;
            }))
            return false;
        }
        _q.push(std::forward<TT>(item));
        if (_q.size() <= 1)
        {
            ul.unlock(); //So the notified thread will have better chance to accuire the lock immediatly..
            _item_pushed_cond.notify_all();
        }
        return true;
    }

    // Push copy of item into the back of the queue.
    // If the queue is full, block the calling thread until there is room.
    template<typename TT>
    void push(TT&& item)
    {
        while (!push(std::forward<TT>(item), std::chrono::seconds(10)));
    }

    // Pop a copy of the front item in the queue into the given item ref.
    // If the queue is empty, block the calling thread util there is item to pop or timeout have passed.
    // Return: false on timeout , true on successful pop/
    template<class Duration_Rep, class Duration_Period>
    bool pop(T& item, const std::chrono::duration<Duration_Rep, Duration_Period>& timeout)
    {
        std::unique_lock<std::mutex> ul(_mutex);
        if (_q.empty())
        {
            if (!_item_pushed_cond.wait_until(ul, clock::now() + timeout, [this]()
        {
            return !this->_q.empty();
            }))
            return false;
        }
        item = std::move(_q.front());
        _q.pop();
        if (_q.size() >= _max_size - 1)
        {
            ul.unlock(); //So the notified thread will have better chance to accuire the lock immediatly..
            _item_popped_cond.notify_all();
        }
        return true;
    }

    // Pop a copy of the front item in the queue into the given item ref.
    // If the queue is empty, block the calling thread util there is item to pop.
    void pop(T& item)
    {
        while (!pop(item, std::chrono::hours(1)));
    }

    // Clear the queue
    void clear()
    {
        {
            std::unique_lock<std::mutex> ul(_mutex);
            queue_type().swap(_q);
        }
        _item_popped_cond.notify_all();
    }

private:
    size_type _max_size;
    std::queue<T> _q;
    std::mutex _mutex;
    std::condition_variable _item_pushed_cond;
    std::condition_variable _item_popped_cond;
};

}
}
