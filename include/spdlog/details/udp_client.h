// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifdef _WIN32
#error include udp_client-windows.h instead
#endif

// udp client helper
#include <spdlog/common.h>
#include <spdlog/details/os.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/udp.h>

#include <string>

namespace spdlog {
namespace details {
class udp_client
{
    int socket_ = -1;
    struct sockaddr_in sockAddr_;
public:

    bool init(const std::string &host, uint16_t port)
    {
        socket_ = socket(PF_INET, SOCK_DGRAM, 0);
        if (socket_ < 0)
        {
            throw_spdlog_ex("error: Create Socket Failed!");
            return false;
        }

        sockAddr_.sin_family = AF_INET;
        sockAddr_.sin_port = htons(port);
        inet_aton(host.c_str(), &sockAddr_.sin_addr);

        memset(sockAddr_.sin_zero, 0x00, sizeof(sockAddr_.sin_zero));
        return true;
    }

    bool is_init() const
    {
        return socket_ != -1;
    }

    void close()
    {
        if (is_init())
        {
            ::close(socket_);
            socket_ = -1;
        }
    }

    int fd() const
    {
        return socket_;
    }

    ~udp_client()
    {
        close();
    }

    // Send exactly n_bytes of the given data.
    // On error close the connection and throw.
    void send(const char *data, size_t n_bytes)
    {
        ssize_t toslen = 0;
        socklen_t tolen = sizeof(struct sockaddr);
        if (( toslen = sendto(socket_, data, n_bytes, 0, (struct sockaddr *)&sockAddr_, tolen)) == -1)
        {
            throw_spdlog_ex("sendto(2) failed", errno);
            close();
        }
    }
};
} // namespace details
} // namespace spdlog