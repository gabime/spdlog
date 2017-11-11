//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "base_sink.h"
#include "../details/null_mutex.h"

#include <mutex>

namespace spdlog
{
namespace sinks
{

template <class Mutex>
class null_sink : public base_sink < Mutex >
{
protected:
    void _sink_it(const details::log_msg&) override
    {}

    void _flush() override
    {}

};
typedef null_sink<details::null_mutex> null_sink_st;
typedef null_sink<details::null_mutex> null_sink_mt;

}
}

