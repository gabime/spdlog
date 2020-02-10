// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include <mutex>
#include <string>

#pragma once

namespace spdlog {
namespace sinks {

template<typename Mutex>
class tcp_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
    // connect to tcp host/port or throw if failed
    // host can be hostname or ip address
    tcp_sink(std::string host, int port)
    {
        sock_ = connect_to(host, port);
    }

    ~tcp_sink() override
    {
        if (sock_ != -1)
        {
            ::close(sock_);
        }
    }

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        size_t bytes_sent = 0;
        while (bytes_sent < formatted.size())
        {
            auto write_result = ::write(sock_, formatted.data() + bytes_sent, formatted.size() - bytes_sent);
            if (write_result < 0)
            {
                SPDLOG_THROW(spdlog::spdlog_ex("write(2) failed", errno));
            }

            if (write_result == 0) // (probably should not happen but in any case..)
            {
                break;
            }
            bytes_sent += static_cast<size_t>(write_result);
        }
    }

    void flush_() override {}

private:
    // try to connect and return socket fd or throw on failure
    int connect_to(const std::string &host, int port)
    {
        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;       // IPv4
        hints.ai_socktype = SOCK_STREAM; // TCP
        hints.ai_flags = AI_NUMERICSERV; // port passed as as numeric value
        hints.ai_protocol = 0;

        auto port_str = std::to_string(port);
        struct addrinfo *addrinfo_result;
        auto rv = ::getaddrinfo(host.c_str(), port_str.c_str(), &hints, &addrinfo_result);
        if (rv != 0)
        {
            auto msg = fmt::format("::getaddrinfo failed: {}", gai_strerror(rv));
            SPDLOG_THROW(spdlog::spdlog_ex(msg));
        }

        // Try each address until we successfully connect(2).
        int socket_rv = -1;
        int last_errno = 0;
        for (auto *rp = addrinfo_result; rp != nullptr; rp = rp->ai_next)
        {
 #ifdef SPDLOG_PREVENT_CHILD_FD
            int const flags = SOCK_CLOEXEC;
 #else
            int const flags = 0;
 #endif
            socket_rv = ::socket(rp->ai_family, rp->ai_socktype | flags, rp->ai_protocol);
            if (socket_rv == -1)
            {
                last_errno = errno;
                continue;
            }
            rv = ::connect(socket_rv, rp->ai_addr, rp->ai_addrlen);
            if (rv == 0)
            {
                break;
            }
            else
            {
                last_errno = errno;
                ::close(socket_rv);
                socket_rv = -1;
            }
        }
        ::freeaddrinfo(addrinfo_result);
        if (socket_rv == -1)
        {
            SPDLOG_THROW(spdlog::spdlog_ex("::connect failed", last_errno));
        }
        return socket_rv;
    }

private:
    int sock_ = -1;
};

using tcp_sink_mt = tcp_sink<std::mutex>;
using tcp_sink_st = tcp_sink<spdlog::details::null_mutex>;

} // namespace sinks
} // namespace spdlog
