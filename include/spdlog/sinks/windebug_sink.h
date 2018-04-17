//
// Copyright(c) 2017 Alexander Dalshov.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#if defined(_WIN32)

#include "msvc_sink.h"

namespace spdlog {
namespace sinks {

/*
 * Windows debug sink (logging using OutputDebugStringA, synonym for msvc_sink)
 */
template<class Mutex>
using windebug_sink = msvc_sink<Mutex>;

using windebug_sink_mt = msvc_sink_mt;
using windebug_sink_st = msvc_sink_st;

} // namespace sinks
} // namespace spdlog

#endif
