/*
A modified version of Bounded MPMC queue by Dmitry Vyukov.

Original code from:
http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue

licensed by Dmitry Vyukov under the terms below:

Simplified BSD license

Copyright (c) 2010-2011 Dmitry Vyukov. All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list
of conditions and the following disclaimer in the documentation and/or other materials
provided with the distribution.

THIS SOFTWARE IS PROVIDED BY DMITRY VYUKOV "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL DMITRY VYUKOV OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the authors and
should not be interpreted as representing official policies, either expressed or implied, of Dmitry Vyukov.
*/

/*
The code in its current form adds the license below:

Copyright(c) 2015 Gabi Melman.
Distributed under the MIT License (http://opensource.org/licenses/MIT)

*/

#pragma once

#include "spdlog/common.h"

#include <atomic>
#include <utility>

namespace spdlog
{
namespace details
{

template<typename T>
class mpmc_bounded_queue
{
public:

    using item_type = T;
    mpmc_bounded_queue(size_t buffer_size)
        :max_size_(buffer_size),
         buffer_(new cell_t [buffer_size]),
         buffer_mask_(buffer_size - 1)
    {
        //queue size must be power of two
        if(!((buffer_size >= 2) && ((buffer_size & (buffer_size - 1)) == 0)))
            throw spdlog_ex("async logger queue size must be power of two");

        for (size_t i = 0; i != buffer_size; i += 1)
            buffer_[i].sequence_.store(i, std::memory_order_relaxed);
        enqueue_pos_.store(0, std::memory_order_relaxed);
        dequeue_pos_.store(0, std::memory_order_relaxed);
    }

    ~mpmc_bounded_queue()
    {
        delete [] buffer_;
    }


    bool enqueue(T&& data)
    {
        cell_t* cell;
        size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
        for (;;)
        {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence_.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)pos;
            if (dif == 0)
            {
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                    break;
            }
            else if (dif < 0)
            {
                return false;
            }
            else
            {
                pos = enqueue_pos_.load(std::memory_order_relaxed);
            }
        }
        cell->data_ = std::move(data);
        cell->sequence_.store(pos + 1, std::memory_order_release);
        return true;
    }

    bool dequeue(T& data)
    {
        cell_t* cell;
        size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
        for (;;)
        {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq =
                cell->sequence_.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
            if (dif == 0)
            {
                if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                    break;
            }
            else if (dif < 0)
                return false;
            else
                pos = dequeue_pos_.load(std::memory_order_relaxed);
        }
        data = std::move(cell->data_);
        cell->sequence_.store(pos + buffer_mask_ + 1, std::memory_order_release);
        return true;
    }

    size_t approx_size()
    {
        size_t first_pos = dequeue_pos_.load(std::memory_order_relaxed);
        size_t last_pos = enqueue_pos_.load(std::memory_order_relaxed);
        if (last_pos <= first_pos)
            return 0;
        auto size = last_pos - first_pos;
        return size < max_size_ ? size : max_size_;
    }

private:
    struct cell_t
    {
        std::atomic<size_t>   sequence_;
        T                     data_;
    };

    size_t const max_size_;

    static size_t const     cacheline_size = 64;
    typedef char            cacheline_pad_t [cacheline_size];

    cacheline_pad_t         pad0_;
    cell_t* const           buffer_;
    size_t const            buffer_mask_;
    cacheline_pad_t         pad1_;
    std::atomic<size_t>     enqueue_pos_;
    cacheline_pad_t         pad2_;
    std::atomic<size_t>     dequeue_pos_;
    cacheline_pad_t         pad3_;

    mpmc_bounded_queue(mpmc_bounded_queue const&) = delete;
    void operator= (mpmc_bounded_queue const&) = delete;
};

} // ns details
} // ns spdlog
