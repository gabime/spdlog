//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include "../details/null_mutex.h"
#include "base_sink.h"

#include <mutex>

namespace spdlog {
namespace sinks {

template<class Mutex>
class test_sink : public base_sink<Mutex>
{
public:
    size_t msg_counter()
    {
        return msg_counter_;
    }

protected:
    void _sink_it(const details::log_msg &) override
    {
        msg_counter_++;
    }

    void _flush() override {}
    size_t msg_counter_{0};
};

using test_sink_mt = test_sink<std::mutex>;
using test_sink_st = test_sink<details::null_mutex>;

} // namespace sinks
} // namespace spdlog
