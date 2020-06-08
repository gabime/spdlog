// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/sinks/base_sink.h>
#include <spdlog/common.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/details/null_mutex.h>
#include <mutex>
#include <memory>

template<typename Mutex>
spdlog::sinks::base_sink<Mutex>::base_sink()
    : formatter_{std::make_unique<spdlog::pattern_formatter>()}
{}

template<typename Mutex>
spdlog::sinks::base_sink<Mutex>::base_sink(std::unique_ptr<spdlog::formatter> formatter)
    : formatter_{std::move(formatter)}
{}

template<typename Mutex>
void spdlog::sinks::base_sink<Mutex>::log(const details::log_msg &msg)
{
    std::lock_guard<Mutex> lock(mutex_);
    sink_it_(msg);
}

template<typename Mutex>
void spdlog::sinks::base_sink<Mutex>::flush()
{
    std::lock_guard<Mutex> lock(mutex_);
    flush_();
}

template<typename Mutex>
void spdlog::sinks::base_sink<Mutex>::set_pattern(const std::string &pattern)
{
    std::lock_guard<Mutex> lock(mutex_);
    set_pattern_(pattern);
}

template<typename Mutex>
void spdlog::sinks::base_sink<Mutex>::set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter)
{
    std::lock_guard<Mutex> lock(mutex_);
    set_formatter_(std::move(sink_formatter));
}

template<typename Mutex>
void spdlog::sinks::base_sink<Mutex>::set_pattern_(const std::string &pattern)
{
    set_formatter_(std::make_unique<spdlog::pattern_formatter>(pattern));
}

template<typename Mutex>
void spdlog::sinks::base_sink<Mutex>::set_formatter_(std::unique_ptr<spdlog::formatter> sink_formatter)
{
    formatter_ = std::move(sink_formatter);
}

// template instantiations
template class SPDLOG_API spdlog::sinks::base_sink<std::mutex>;
template class SPDLOG_API spdlog::sinks::base_sink<spdlog::details::null_mutex>;
