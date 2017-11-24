//
// Copyright(c) 2016 Alexander Dalshov.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#if defined(_WIN32)

#include "base_sink.h"
#include "../details/null_mutex.h"

#include <winbase.h>

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
        fmt::MemoryWriter formatted;
        fmt::MemoryWriter* formatted_msg = const_cast <fmt::MemoryWriter*> (get_formatted_msg(msg, formatted));
        OutputDebugStringA(formatted_msg->c_str());
    }

    void _flush() override
    {}
};

typedef msvc_sink<std::mutex> msvc_sink_mt;
typedef msvc_sink<details::null_mutex> msvc_sink_st;

}
}

#endif
