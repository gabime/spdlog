// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#ifdef _WIN32
#include <spdlog/details/udp_client-windows.h>
#else
#include <spdlog/details/udp_client.h>
#endif

#include <mutex>
#include <string>
#include <chrono>
#include <functional>

#pragma once

// Simple tcp client sink
// Connects to remote address and send the formatted log.
// Will attempt to reconnect if connection drops.
// If more complicated behaviour is needed (i.e get responses), you can inherit it and override the sink_it_ method.

namespace spdlog {
namespace sinks {

struct udp_sink_config
{
    std::string server_host;
    int server_port;

    udp_sink_config(std::string host, int port)
        : server_host{std::move(host)}
        , server_port{port}
    {}
};

template<typename Mutex>
class udp_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
    // connect to tcp host/port or throw if failed
    // host can be hostname or ip address

    explicit udp_sink(udp_sink_config sink_config)
        : config_{std::move(sink_config)}
    {

    }

    ~udp_sink() override = default;

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        if (!client_.is_init())
        {
            client_.init(config_.server_host, config_.server_port);
        }
        client_.send(formatted.data(), formatted.size());
    }

    void flush_() override {}
    udp_sink_config config_;
    details::udp_client client_;
};

using udp_sink_mt = udp_sink<std::mutex>;
using udp_sink_st = udp_sink<spdlog::details::null_mutex>;

} // namespace sinks

//
// factory functions
//
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> udp_logger_mt(const std::string &logger_name, sinks::udp_sink_config skin_config)
{
    return Factory::template create<sinks::udp_sink_mt>(logger_name, skin_config);
}

} // namespace spdlog
