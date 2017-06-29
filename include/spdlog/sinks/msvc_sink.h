//
// Copyright(c) 2016 Alexander Dalshov.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#if defined(_MSC_VER)

#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/null_mutex.h"

#include <WinBase.h>

#include <mutex>
#include <string>

namespace spdlog
{
namespace sinks
{
/*
* MSVC sink (logging using OutputDebugStringA)
*/
template<class Mutex>
class msvc_sink : public base_sink < Mutex >
{
public:
    explicit msvc_sink()
    {
    }



protected:
    void _sink_it(const details::log_msg& msg) override
    {
        OutputDebugStringA(msg.formatted.c_str());
    }

    void _flush() override
    {}
};

typedef msvc_sink<std::mutex> msvc_sink_mt;
typedef msvc_sink<details::null_mutex> msvc_sink_st;

}
}

#endif
