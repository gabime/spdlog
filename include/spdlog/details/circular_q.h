//
// Copyright(c) 2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

// cirucal q view of std::vector.
#pragma once

namespace spdlog {
namespace details {
template<typename T>
class circular_q
{
public:
    using item_type = T;

    explicit circular_q(size_t max_items)
        : max_items_(max_items + 1)
        , v_(max_items_)
    {
    }

    // push back, overrun last item if no room left
    void push_back(T &&item)
    {
        v_[head_] = std::move(item);
        head_ = (head_ + 1) % max_items_;

        if (head_ == tail_)
        {
            tail_ = (tail_ + 1) % max_items_;
        }
    }

    // Pop item from front.
    // If there are no elements in the container, the behavior is undefined.
    void pop_front(T &popped_item)
    {
        popped_item = std::move(v_[tail_]);
        tail_ = (tail_ + 1) % max_items_;
    }

    bool empty()
    {
        return head_ == tail_;
    }

    bool full()
    {
        // tail is ahead of the head by 1
        return ((head_ + 1) % max_items_) == tail_;
    }

private:
    size_t max_items_;
    typename std::vector<T>::size_type head_ = 0;
    typename std::vector<T>::size_type tail_ = 0;
    std::vector<T> v_;
};
} // namespace details
} // namespace spdlog