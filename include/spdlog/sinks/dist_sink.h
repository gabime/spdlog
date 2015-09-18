/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2015 David Schury.                                      */
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

#include <algorithm>
#include <memory>
#include <mutex>
#include <list>

#include "../details/log_msg.h"
#include "../details/null_mutex.h"
#include "./base_sink.h"
#include "./sink.h"

namespace spdlog
{
namespace sinks
{
template<class Mutex>
class dist_sink: public base_sink<Mutex>
{
public:
    explicit dist_sink() :_sinks() {}
    dist_sink(const dist_sink&) = delete;
    dist_sink& operator=(const dist_sink&) = delete;
    virtual ~dist_sink() = default;

protected:
    void _sink_it(const details::log_msg& msg) override
    {
        for (auto iter = _sinks.begin(); iter != _sinks.end(); iter++)
            (*iter)->log(msg);
    }

    std::vector<std::shared_ptr<sink>> _sinks;

public:
    void flush() override
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::_mutex);
        for (auto iter = _sinks.begin(); iter != _sinks.end(); iter++)
            (*iter)->flush();
    }

    void add_sink(std::shared_ptr<sink> sink)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::_mutex);
        if (sink &&
            _sinks.end() == std::find(_sinks.begin(), _sinks.end(), sink))
        {
            _sinks.push_back(sink);
        }
    }

    void remove_sink(std::shared_ptr<sink> sink)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::_mutex);
        auto pos = std::find(_sinks.begin(), _sinks.end(), sink);
        if (pos != _sinks.end())
        {
            _sinks.erase(pos);
        }
    }
};

typedef dist_sink<std::mutex> dist_sink_mt;
typedef dist_sink<details::null_mutex> dist_sink_st;
}
}
