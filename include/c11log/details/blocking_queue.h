#pragma once

#include <cstddef>
#include <chrono>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace c11log {
namespace details {

template<typename T>
class blocking_queue {
public:
    explicit blocking_queue(std::size_t max_size) :_max_size(max_size), _q()
    {}
    blocking_queue(const blocking_queue&) = delete;
    blocking_queue& operator=(const blocking_queue&) = delete;
    ~blocking_queue() = default;

    std::size_t size()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _q.size();
    }
    bool push(const T& item, const std::chrono::milliseconds& timeout)
    {
        std::unique_lock<std::mutex> ul(_mutex);
        if (_q.size() >= _max_size) {
            if (_item_popped_cond.wait_for(ul, timeout) == std::cv_status::timeout || _q.size() >= _max_size)
                return false;
        }

        _q.push(item);
        if (_q.size() <= 1)
            _item_pushed_cond.notify_all();

        return true;
    }
    bool pop(T& item, const std::chrono::milliseconds& timeout)
    {
        std::unique_lock<std::mutex> ul(_mutex);
        if (_q.empty()) {
            if (_item_pushed_cond.wait_for(ul, timeout) == std::cv_status::timeout || _q.empty())
                return false;
        }
        item = _q.front();
        _q.pop();
        if (_q.size() >= _max_size - 1)
            _item_popped_cond.notify_all();

        return true;
    }
private:
    std::size_t _max_size;
    std::queue<T> _q;
    std::mutex _mutex;
    std::condition_variable _item_pushed_cond;
    std::condition_variable _item_popped_cond;
};

}
}