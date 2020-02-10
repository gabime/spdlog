// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
#include <spdlog/sinks/base_sink.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

namespace spdlog {
namespace sinks {

template<typename Mutex>
class tcp_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
    tcp_sink(std::string address, int port)
    {
        if ((sock_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            SPDLOG_THROW(spdlog::spdlog_ex("Socket creation error", errno));
        }
        struct hostent *he = gethostbyname(address.c_str());
        if (he == nullptr)
        {
            SPDLOG_THROW(spdlog::spdlog_ex("gethostbyname failed", errno));
        }

        serv_addr_.sin_family = AF_INET;

        serv_addr_.sin_addr = *(struct in_addr *)(he->h_addr);
        serv_addr_.sin_port = ::htons(static_cast<uint16_t>(port));
        if (connect(sock_, (struct sockaddr *)&serv_addr_, sizeof(serv_addr_)) < 0)
        {
            SPDLOG_THROW(spdlog::spdlog_ex("Connection Failed", errno));
        }
    }

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        auto res = ::send(sock_, formatted.data(), formatted.size(), 0);
        if (res < 0)
        {
            SPDLOG_THROW(spdlog::spdlog_ex("Message Send Failed", errno));
        }
    }

    void flush_() override {}

private:

private:
    int sock_;
    struct sockaddr_in serv_addr_;
};

using tcp_sink_mt = tcp_sink<std::mutex>;
using tcp_sink_st = tcp_sink<spdlog::details::null_mutex>;

} // namespace sinks
} // namespace spdlog
