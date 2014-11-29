#pragma once
/*************************************************************************/
/*
Modified version of Intrusive MPSC node-based queue

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

/*************************************************************************/
/* The code in its current form adds the license below:                  */
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


#include <atomic>
namespace spdlog {
namespace details {
template<typename T>
class mpsc_q
{

public:
    mpsc_q(size_t size) :_stub(T()), _head(&_stub), _tail(&_stub)
    {
        _stub.next = nullptr;
    }

    ~mpsc_q()
    {
        reset();
    }

    void reset()
    {
        T dummy_val;
        while (pop(dummy_val));
    }

    bool push(const T& value)
    {
        mpscq_node_t* new_node = new mpscq_node_t(value);
        push_node(new_node);
        return true;
    }

    bool pop(T& value)
    {
        mpscq_node_t* node = pop_node();
        if (node != nullptr)
        {
            value = node->value;
            delete(node);
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    struct mpscq_node_t
    {
        std::atomic<mpscq_node_t*>  next;
        T value;

        explicit mpscq_node_t(const T& value) :next(nullptr), value(value)
        {
        }
    };

    mpscq_node_t            _stub;
    std::atomic<mpscq_node_t*>  _head;
    mpscq_node_t*           _tail;



    void push_node(mpscq_node_t* n)
    {
        n->next = nullptr;
        mpscq_node_t* prev = _head.exchange(n);
        prev->next = n;
    }

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
            return 0;

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