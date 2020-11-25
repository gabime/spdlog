// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#define WIN32_LEAN_AND_MEAN
// tcp client helper
#include <spdlog/common.h>
#include <spdlog/details/os.h>

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

namespace spdlog {
namespace details {
class udp_client
{
    SOCKET socket_ = INVALID_SOCKET;
    sockaddr_in addr_ = { 0 };

    static bool winsock_initialized_()
    {
        SOCKET s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (s == INVALID_SOCKET)
        {
            return false;
        }
        else
        {
            closesocket(s);
            return true;
        }
    }

    static void init_winsock_()
    {
        WSADATA wsaData;
        auto rv = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (rv != 0)
        {
            throw_winsock_error_("WSAStartup failed", ::WSAGetLastError());
        }
    }

    static void throw_winsock_error_(const std::string &msg, int last_error)
    {
        char buf[512];
        ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, last_error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, (sizeof(buf) / sizeof(char)), NULL);

        throw_spdlog_ex(fmt::format("tcp_sink - {}: {}", msg, buf));
    }

public:
    bool is_init() const
    {
        return socket_ != INVALID_SOCKET;
    }

    bool init(const std::string &host, int port)
    {
        // initialize winsock if needed
        if (!winsock_initialized_())
        {
            init_winsock_();
        }

        if (is_init())
        {
            close();
        }

        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        InetPton(AF_INET, TEXT(host.c_str()), &addr_.sin_addr.s_addr);
        return true;
    }

    void close()
    {
        ::closesocket(socket_);
        socket_ = INVALID_SOCKET;
        WSACleanup();
    }

    SOCKET fd() const
    {
        return socket_;
    }

    ~udp_client()
    {
        close();
    }

    void send(const char *data, size_t n_bytes)
    {
        size_t toslen = 0;
        size_t tolen = sizeof(struct sockaddr);
        if (( toslen = sendto(socket_, data, n_bytes, 0, (struct sockaddr *)&addr_, tolen)) == -1)
        {
            throw_spdlog_ex("write(2) failed", errno);
            close();
        }
    }
};
} // namespace details
} // namespace spdlog
