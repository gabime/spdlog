// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/sinks/basic_file_sink.h"

#include <mutex>

#include "spdlog/common.h"

SPDLOG_NAMESPACE_BEGIN
namespace sinks {

template <typename Mutex>
basic_file_sink<Mutex>::basic_file_sink(const filename_t &filename, bool truncate, const file_event_handlers &event_handlers)
    : file_helper_{event_handlers} {
    file_helper_.open(filename, truncate);
}

template <typename Mutex>
const filename_t &basic_file_sink<Mutex>::filename() const {
    return file_helper_.filename();
}

template <typename Mutex>
void basic_file_sink<Mutex>::truncate() {
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    file_helper_.reopen(true);
}

template <typename Mutex>
void basic_file_sink<Mutex>::sink_it_(const details::log_msg &msg) {
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    file_helper_.write(formatted);
}

template <typename Mutex>
void basic_file_sink<Mutex>::flush_() {
    file_helper_.flush();
}

}  // namespace sinks
SPDLOG_NAMESPACE_END

// template instantiations
#include "spdlog/details/null_mutex.h"
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::basic_file_sink<std::mutex>;
template class SPDLOG_API SPDLOG_NAMESPACE::sinks::basic_file_sink<SPDLOG_NAMESPACE::details::null_mutex>;