//
// Copyright(c) 2017 Alexander Dalshov.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#if defined(_WIN32)

#include "msvc_sink.h"

namespace spdlog
{
namespace sinks
{

/*
* Windows debug sink (logging using OutputDebugStringA, synonym for msvc_sink)
*/
template<class Mutex>
using windebug_sink = msvc_sink<Mutex>;

typedef msvc_sink_mt windebug_sink_mt;
typedef msvc_sink_st windebug_sink_st;

}
}

#endif
