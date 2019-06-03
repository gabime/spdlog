// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// cirucal q view of std::vector.
#pragma once

#include <vector>

namespace spdlog {
namespace details {
template<typename T>
class circular_q
{
public:
    using item_type = T;

    explicit circular_q(size_t max_items)
        : max_items_(max_items + 1) // one item is reserved as marker for full q
        , v_(max_items_)
    {}

    // push back, overrun (oldest) item if no room left
    void push_back(T &&item)
    {
        v_[tail_] = std::move(item);
        tail_ = (tail_ + 1) % max_items_;

        if (tail_ == head_) // overrun last item if full
        {
            head_ = (head_ + 1) % max_items_;
            ++overrun_counter_;
        }
    }

    // Pop item from front.
    // If there are no elements in the container, the behavior is undefined.
    void pop_front(T &popped_item)
    {
        popped_item = std::move(v_[head_]);
        head_ = (head_ + 1) % max_items_;
    }

    bool empty()
    {
        return tail_ == head_;
    }

    bool full()
    {
        // head is ahead of the tail by 1
        return ((tail_ + 1) % max_items_) == head_;
    }

    size_t overrun_counter() const
    {
        return overrun_counter_;
    }

private:
    size_t max_items_;
    typename std::vector<T>::size_type head_ = 0;
    typename std::vector<T>::size_type tail_ = 0;

    std::vector<T> v_;

    size_t overrun_counter_ = 0;
};
} // namespace details
} // namespace spdlog
