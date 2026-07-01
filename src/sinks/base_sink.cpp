// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/sinks/base_sink.h"

#include <memory>
#include <mutex>

#include "spdlog/common.h"
#include "spdlog/pattern_formatter.h"

SPDLOG_NAMESPACE_BEGIN
namespace sinks {

template <typename Mutex>
base_sink<Mutex>::base_sink()
    : formatter_{std::make_unique<pattern_formatter>()} {}

template <typename Mutex>
base_sink<Mutex>::base_sink(std::unique_ptr<formatter> formatter)
    : formatter_{std::move(formatter)} {}

template <typename Mutex>
void base_sink<Mutex>::log(const details::log_msg &msg) {
    std::lock_guard<Mutex> lock(mutex_);
    sink_it_(msg);
}

template <typename Mutex>
void base_sink<Mutex>::flush() {
    std::lock_guard<Mutex> lock(mutex_);
    flush_();
}

template <typename Mutex>
void base_sink<Mutex>::set_pattern(const std::string &pattern) {
    std::lock_guard<Mutex> lock(mutex_);
    set_pattern_(pattern);
}

template <typename Mutex>
void base_sink<Mutex>::set_formatter(std::unique_ptr<formatter> sink_formatter) {
    std::lock_guard<Mutex> lock(mutex_);
    set_formatter_(std::move(sink_formatter));
}

template <typename Mutex>
void base_sink<Mutex>::set_pattern_(const std::string &pattern) {
    set_formatter_(std::make_unique<pattern_formatter>(pattern));
}

template <typename Mutex>
void base_sink<Mutex>::set_formatter_(std::unique_ptr<formatter> sink_formatter) {
    formatter_ = std::move(sink_formatter);
}

}  // namespace sinks
SPDLOG_NAMESPACE_END

// template instantiations
#include "spdlog/details/null_mutex.h"
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::base_sink<std::mutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::base_sink<SPDLOG_NAMESPACE::details::null_mutex>;
