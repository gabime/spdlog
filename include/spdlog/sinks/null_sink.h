//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "spdlog/details/null_mutex.h"
#include "spdlog/sinks/base_sink.h"

#include <mutex>

namespace spdlog {
namespace sinks {

template<typename Mutex>
class null_sink : public base_sink<Mutex>
{
protected:
    void sink_it_(const details::log_msg &) override {}
    void flush_() override {}
};

using null_sink_mt = null_sink<std::mutex>;
using null_sink_st = null_sink<details::null_mutex>;

} // namespace sinks
} // namespace spdlog
