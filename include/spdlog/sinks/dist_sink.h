//
// Copyright (c) 2015 David Schury, Gabi Melman
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "../details/log_msg.h"
#include "../details/null_mutex.h"
#include "base_sink.h"
#include "sink.h"

#include <algorithm>
#include <mutex>
#include <memory>
#include <vector>

// Distribution sink (mux). Stores a vector of sinks which get called when log is called

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
    std::vector<std::shared_ptr<sink>> _sinks;

    void _sink_it(const details::log_msg& msg) override
    {
        for (auto &sink : _sinks)
        {
            if( sink->should_log( msg.level))
            {
                sink->log(msg);
            }
        }
    }

    void _flush() override
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::_mutex);
        for (auto &sink : _sinks)
            sink->flush();
    }

public:


    void add_sink(std::shared_ptr<sink> sink)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::_mutex);
        _sinks.push_back(sink);
    }

    void remove_sink(std::shared_ptr<sink> sink)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::_mutex);
        _sinks.erase(std::remove(_sinks.begin(), _sinks.end(), sink), _sinks.end());
    }
};

typedef dist_sink<std::mutex> dist_sink_mt;
typedef dist_sink<details::null_mutex> dist_sink_st;
}
}
