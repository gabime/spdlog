// Copyright(c) 2015-present, Gabi Melman, mguludag and spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

//
// Custom sink for your own lambda functions or functors
//

#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/details/synchronous_factory.h"
#include "spdlog/sinks/base_sink.h"

#include <functional>

//
// lambda_sink class
//
namespace spdlog {
namespace sinks {
template<typename Mutex>
class lambda_sink : public base_sink<Mutex>
{
public:
    explicit lambda_sink(const std::function<void(const char*, int)>& f) : f_(f) { }

    ~lambda_sink() { flush_(); }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        f_(formatted.data(), formatted.size());
    }

    void flush_() override {}

private:
    std::function<void(const char*, int)> f_;
};

#include "spdlog/details/null_mutex.h"
#include <mutex>
using lambda_sink_mt = lambda_sink<std::mutex>;
using lambda_sink_st = lambda_sink<spdlog::details::null_mutex>;
} // namespace sinks

//
// Factory functions
//
template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
lambda_logger_mt(const std::string &logger_name, const std::function<void(const char*, int)>& f) {
    return Factory::template create<sinks::lambda_sink_mt>(logger_name, f);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
lambda_logger_st(const std::string &logger_name, const std::function<void(const char*, int)>& f) {
    return Factory::template create<sinks::lambda_sink_st>(logger_name, f);
}
} // namespace spdlog
