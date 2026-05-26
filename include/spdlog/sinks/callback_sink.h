// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <mutex>
#include <string>

#include "../details/null_mutex.h"
#include "./base_sink.h"

SPDLOG_NAMESPACE_BEGIN

// callbacks type
typedef std::function<void(const details::log_msg &msg)> custom_log_callback;

namespace sinks {
/*
 * Trivial callback sink, gets a callback function and calls it on each log
 */
template <typename Mutex>
class callback_sink final : public base_sink<Mutex> {
public:
    explicit callback_sink(const custom_log_callback &callback)
        : callback_{callback} {}

protected:
    void sink_it_(const details::log_msg &msg) override { callback_(msg); }
    void flush_() override {}

private:
    custom_log_callback callback_;
};

using callback_sink_mt = callback_sink<std::mutex>;
using callback_sink_st = callback_sink<details::null_mutex>;

}  // namespace sinks
SPDLOG_NAMESPACE_END
