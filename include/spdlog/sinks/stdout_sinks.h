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

#include <iostream>
#include <mutex>
#include "./ostream_sink.h"
#include "../details/null_mutex.h"

namespace spdlog
{
namespace sinks
{

template <class Mutex>
class stdout_sink : public ostream_sink<Mutex>
{
    using MyType = stdout_sink<Mutex>;
public:
    stdout_sink() : ostream_sink<Mutex>(std::cout, true) {}
    static std::shared_ptr<MyType> instance()
    {
        static std::shared_ptr<MyType> instance = std::make_shared<MyType>();
        return instance;
    }
};

typedef stdout_sink<details::null_mutex> stdout_sink_st;
typedef stdout_sink<std::mutex> stdout_sink_mt;


template <class Mutex>
class stderr_sink : public ostream_sink<Mutex>
{
    using MyType = stderr_sink<Mutex>;
public:
    stderr_sink() : ostream_sink<Mutex>(std::cerr, true) {}
    static std::shared_ptr<MyType> instance()
    {
        static std::shared_ptr<MyType> instance = std::make_shared<MyType>();
        return instance;
    }

};

typedef stderr_sink<std::mutex> stderr_sink_mt;
typedef stderr_sink<details::null_mutex> stderr_sink_st;
}
}
