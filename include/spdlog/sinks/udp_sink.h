// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "../common.h"
#include "../details/null_mutex.h"
#include "./base_sink.h"
#ifdef _WIN32
    #include "../details/udp_client_windows.h"
#else
    #include "../details/udp_client_unix.h"
#endif

#include <chrono>
#include <functional>
#include <mutex>
#include <string>

// Simple udp client sink
// Sends formatted log via udp

SPDLOG_NAMESPACE_BEGIN
namespace sinks {

struct udp_sink_config {
    std::string server_host;
    uint16_t server_port;

    udp_sink_config(std::string host, uint16_t port)
        : server_host{std::move(host)},
          server_port{port} {}
};

template <typename Mutex>
class udp_sink final : public base_sink<Mutex> {
public:
    // host can be hostname or ip address
    explicit udp_sink(const udp_sink_config &sink_config)
        : client_{sink_config.server_host, sink_config.server_port} {}

    ~udp_sink() override = default;

protected:
    void sink_it_(const details::log_msg &msg) override {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        client_.send(formatted.data(), formatted.size());
    }

    void flush_() override {}
    details::udp_client_unix client_;
};

using udp_sink_mt = udp_sink<std::mutex>;
using udp_sink_st = udp_sink<details::null_mutex>;

}  // namespace sinks
SPDLOG_NAMESPACE_END
