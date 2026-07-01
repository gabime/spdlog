// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "../details/null_mutex.h"
#include "./base_sink.h"

SPDLOG_NAMESPACE_BEGIN
namespace sinks {

template <typename Mutex>
class null_sink final : public base_sink<Mutex> {
protected:
    void sink_it_(const details::log_msg &) override {}
    void flush_() override {}
};

using null_sink_mt = null_sink<details::null_mutex>;
using null_sink_st = null_sink<details::null_mutex>;

}  // namespace sinks
SPDLOG_NAMESPACE_END
