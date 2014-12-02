/*
A modified version of Intrusive MPSC node-based queue

Original code from
http://www.1024cores.net/home/lock-free-algorithms/queues/intrusive-mpsc-node-based-queue
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

/*************************************************************************/
/********* The code in its current form adds the license below: **********/
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

#include <atomic>
namespace spdlog
{
namespace details
{
template<typename T>
class mpsc_q
{

public:
    using item_type = T;
    explicit mpsc_q(size_t max_size) :
        _max_size(max_size),
        _size(0),
        _stub(),
        _head(&_stub),
        _tail(&_stub)
    {
    }

    mpsc_q(const mpsc_q&) = delete;
    mpsc_q& operator=(const mpsc_q&) = delete;

    ~mpsc_q()
    {
        clear();
    }

    template<typename TT>
    bool push(TT&& value)
    {
        if (_size >= _max_size)
            return false;
        mpscq_node_t* new_node = new mpscq_node_t(std::forward<TT>(value));
        push_node(new_node);
        ++_size;
        return true;
    }

    // Try to pop or return false immediatly is queue is empty
    bool pop(T& value)
    {
        mpscq_node_t* node = pop_node();
        if (node != nullptr)
        {
            --_size;
            value = std::move(node->value);
            delete(node);
            return true;
        }
        else
        {
            return false;
        }
    }

    // Empty the queue by popping all its elements
    void clear()
    {
        while (mpscq_node_t* node = pop_node())
        {
            --_size;
            delete(node);
        }

    }

    // Return approx size
    size_t approx_size() const
    {
        return _size.load();
    }

private:
    struct mpscq_node_t
    {
        std::atomic<mpscq_node_t*>  next;
        T value;

        mpscq_node_t() :next(nullptr) {}
        mpscq_node_t(const mpscq_node_t&) = delete;
        mpscq_node_t& operator=(const mpscq_node_t&) = delete;

        explicit mpscq_node_t(const T& value):
            next(nullptr),
            value(value) {}

        explicit mpscq_node_t(T&& value) :
            next(nullptr),
            value(std::move(value)) {}
    };

    size_t _max_size;
    std::atomic<size_t> _size;
    mpscq_node_t            _stub;
    std::atomic<mpscq_node_t*>  _head;
    mpscq_node_t*           _tail;

    // Lockfree push
    void push_node(mpscq_node_t* n)
    {
        n->next = nullptr;
        mpscq_node_t* prev = _head.exchange(n);
        prev->next = n;
    }

    // Clever lockfree pop algorithm by Dmitry Vyukov using single xchng instruction..
    // Return pointer to the poppdc node or nullptr if no items left in the queue
    mpscq_node_t* pop_node()
    {
        mpscq_node_t* tail = _tail;
        mpscq_node_t* next = tail->next;
        if (tail == &_stub)
        {
            if (nullptr == next)
                return nullptr;
            _tail = next;
            tail = next;
            next = next->next;
        }
        if (next)
        {
            _tail = next;
            return tail;
        }
        mpscq_node_t* head = _head;
        if (tail != head)
            return nullptr;

        push_node(&_stub);
        next = tail->next;
        if (next)
        {
            _tail = next;
            return tail;
        }
        return nullptr;
    }

};
}
}