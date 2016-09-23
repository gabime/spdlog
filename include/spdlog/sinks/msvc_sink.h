//
// Copyright(c) 2016 Alexander Dalshov.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#if defined(_MSC_VER)

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>

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

    void flush() override
    {
    }

protected:
    void _sink_it(const details::log_msg& msg) override
    {
#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
		OutputDebugStringW(msg.formatted.c_str());
#else
		OutputDebugStringA(msg.formatted.c_str());
#endif
    }
};

typedef msvc_sink<std::mutex> msvc_sink_mt;
typedef msvc_sink<details::null_mutex> msvc_sink_st;

}
}

#endif
