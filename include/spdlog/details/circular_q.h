// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// circular q view of std::vector.
#pragma once

#include <vector>
#include <cassert>
#include <iterator>
#include <cstddef>

namespace spdlog {
namespace details {
template<typename T>
class circular_q
{
    size_t max_items_ = 0;
    typename std::vector<T>::size_type head_ = 0;
    typename std::vector<T>::size_type tail_ = 0;
    size_t overrun_counter_ = 0;
    std::vector<T> v_;

public:
    using value_type = T;

    // empty ctor - create a disabled queue with no elements allocated at all
    circular_q() = default;

    explicit circular_q(size_t max_items)
        : max_items_(max_items + 1) // one item is reserved as marker for full q
        , v_(max_items_)
    {}

    circular_q(const circular_q &) = default;
    circular_q &operator=(const circular_q &) = default;

    // move cannot be default,
    // since we need to reset head_, tail_, etc to zero in the moved object
    circular_q(circular_q &&other) noexcept
    {
        copy_moveable(std::move(other));
    }

    circular_q &operator=(circular_q &&other) noexcept
    {
        copy_moveable(std::move(other));
        return *this;
    }

    // push back, overrun (oldest) item if no room left
    void push_back(T &&item)
    {
        if (max_items_ > 0)
        {
            v_[tail_] = std::move(item);
            tail_ = (tail_ + 1) % max_items_;

            if (tail_ == head_) // overrun last item if full
            {
                head_ = (head_ + 1) % max_items_;
                ++overrun_counter_;
            }
        }
    }

    // Return reference to the front item.
    // If there are no elements in the container, the behavior is undefined.
    const T &front() const
    {
        return v_[head_];
    }

    T &front()
    {
        return v_[head_];
    }

    // Return number of elements actually stored
    [[nodiscard]] size_t size() const
    {
        if (tail_ >= head_)
        {
            return tail_ - head_;
        }
        else
        {
            return max_items_ - (head_ - tail_);
        }
    }

    // Return const reference to item by index.
    // If index is out of range 0…size()-1, the behavior is undefined.
    const T &operator[](size_t idx) const
    {
        assert(idx < size());
        assert(max_items_ > 0);
        return v_[(head_ + idx) % max_items_];
    }

    // Return reference to item by index.
    T &operator[](size_t idx)
    {
        return const_cast<T &>(static_cast<const circular_q &>(*this)[idx]);
    }

    // Pop item from front if exists.
    void pop_front()
    {
        if(!empty())
        {
            head_ = (head_ + 1) % max_items_;
        }
    }

    [[nodiscard]] bool empty() const
    {
        return tail_ == head_;
    }

    [[nodiscard]] bool full() const
    {
        // head is ahead of the tail by 1
        if (max_items_ > 0)
        {
            return ((tail_ + 1) % max_items_) == head_;
        }
        return true;
    }

    [[nodiscard]] size_t overrun_counter() const
    {
        return overrun_counter_;
    }

    void reset_overrun_counter()
    {
        overrun_counter_ = 0;
    }

    // Iterator support
    class iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T *;
        using reference = T &;

        explicit iterator(circular_q *circular_q = nullptr, size_t index = 0)
            : cq_(circular_q)
            , index_(index)
        {}

        reference operator*() const
        {
            return cq_->operator[](index_);
        }

        pointer operator->()
        {
            return &operator*();
        }

        // Prefix increment
        iterator &operator++()
        {
            if(cq_ != nullptr)
            {
                index_ = (index_ + 1);
            }
            return *this;
        }

        // Postfix increment
        iterator operator++(int)
        {
            iterator retval = *this;
            ++(*this);
            return retval;
        }

        bool operator==(iterator other) const
        {
            return cq_ == other.cq_ && index_ == other.index_;
        }

        bool operator!=(iterator other) const
        {
            return index_ != other.index_ || cq_ != other.cq_;
        }

    private:
        circular_q *cq_;
        size_t index_;
    };

    iterator begin()
    {
        return iterator(this, 0);
    }
    iterator end()
    {
        return iterator(this, size());
    }

private:
    // copy from other&& and reset it to disabled state
    void copy_moveable(circular_q &&other) noexcept
    {
        max_items_ = other.max_items_;
        head_ = other.head_;
        tail_ = other.tail_;
        overrun_counter_ = other.overrun_counter_;
        v_ = std::move(other.v_);

        // put &&other in disabled, but valid state
        other.max_items_ = 0;
        other.head_ = other.tail_ = 0;
        other.overrun_counter_ = 0;
    }
};
} // namespace details
} // namespace spdlog
