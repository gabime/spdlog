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
#include <fcntl.h>
#include <poll.h>

#include <mutex>
#include <string>
#include <memory>

#pragma once

namespace spdlog {
namespace sinks {

template<typename Mutex>
class tcp_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
    // connect to tcp host/port or throw if failed
    // host can be hostname or ip address
    tcp_sink(std::string host, int port, std::chrono::nanoseconds timeout = std::chrono::seconds{10})
    {
        sock_ = connect_to(host, port, timeout);
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
    int connect_to(const std::string &host, int port, std::chrono::nanoseconds timeout)
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
            struct SocketCloser
            {
                using pointer = int;
                void operator()(int socket_fd)
                {
                    ::close(socket_fd);
                }
            };
            std::unique_ptr<int, SocketCloser> socket_up(socket_rv);
            socket_rv = -1;

            int const oldFlag = ::fcntl(socket_up.get(), F_GETFL);
            if (oldFlag == -1)
            {
                last_errno = errno;
                continue;
            }
            if (-1 == ::fcntl(socket_up.get(), F_SETFL, oldFlag | O_NONBLOCK))
            {
                last_errno = errno;
                continue;
            }

            if (0 != ::connect(socket_up.get(), rp->ai_addr, rp->ai_addrlen))
            {
                if (errno == EINPROGRESS || errno == EINTR)
                {
                    pollfd fds{};

                    fds.fd = socket_up.get();
                    fds.events = POLLOUT | POLLRDHUP;

                    auto calcTs = [](std::chrono::nanoseconds timeout_) {
                        timeout_ = std::chrono::nanoseconds{} > timeout_ ? std::chrono::nanoseconds{} : timeout_;
                        std::chrono::seconds const sec = std::chrono::duration_cast<std::chrono::seconds>(timeout_);
                        timespec ts{};
                        if (sec.count() >= std::numeric_limits<decltype(ts.tv_sec)>::max())
                        {
                            ts.tv_sec = std::numeric_limits<decltype(ts.tv_sec)>::max();
                            ts.tv_nsec = 0;
                        }
                        else
                        {
                            ts.tv_sec = sec.count();
                            ts.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout_ - sec).count();
                        }
                        return ts;
                    };

                    timeout = std::chrono::nanoseconds{} > timeout ? std::chrono::nanoseconds{} : timeout;

                    auto const stoptime =
                        timeout > std::chrono::hours(24 * 365 * 100)
                            ? std::chrono::steady_clock::time_point::max()
                            : std::chrono::steady_clock::now() +
                                  timeout; // TODO could overflow but the program run for ~191 years in that case so that should be OK

                    auto ts = calcTs(timeout);

                    bool pollOk = false;
                    last_errno = 0;
                    while (true)
                    {
                        int const status = ::ppoll(&fds, 1, &ts, nullptr);
                        if (status == -1)
                        {
                            if (errno == EINTR)
                            {
                                if (stoptime != std::chrono::steady_clock::time_point::max())
                                {
                                    auto const now = std::chrono::steady_clock::now();
                                    if (now >= stoptime)
                                    {
                                        break;
                                    }
                                    ts = calcTs(std::chrono::duration_cast<std::chrono::nanoseconds>(stoptime - now));
                                }
                                continue;
                            }
                            last_errno = errno;
                            break;
                        }
                        else if (status == 0)
                        {
                            // check that ppoll has not returned to early (more common then I thought)
                            auto const now = std::chrono::steady_clock::now();
                            if (now < stoptime)
                            {
                                ts = calcTs(std::chrono::duration_cast<std::chrono::nanoseconds>(stoptime - now));
                                continue;
                            }
                            break;
                        }
                        else
                        {
                            if ((fds.revents & POLLNVAL) != 0)
                            {
                                break;
                            }

                            if (((fds.revents & POLLOUT) != 0) || ((fds.revents & POLLERR) != 0) || ((fds.revents & POLLHUP) != 0))
                            {
                                pollOk = true;
                            }
                            break;
                        }
                    }

                    if (pollOk)
                    {
                        socklen_t lon{sizeof(int)};
                        int valopt{};
                        if (0 != ::getsockopt(socket_up.get(), SOL_SOCKET, SO_ERROR, &valopt, &lon))
                        {
                            last_errno = errno;
                            continue;
                        }
                        if (valopt != 0)
                        {
                            last_errno = valopt;
                            continue;
                        }
                    }
                    else
                    {
                        if (last_errno == 0)
                        {
                            last_errno = ETIMEDOUT;
                        }
                        continue;
                    }
                }
                else
                {
                    last_errno = errno;
                    continue;
                }
            }

            if (-1 == ::fcntl(socket_up.get(), F_SETFL, oldFlag))
            {
                last_errno = errno;
                continue;
            }

            if (-1 == ::shutdown(socket_up.get(), SHUT_RD))
            {
                last_errno = errno;
                continue;
            }

            socket_rv = socket_up.release();
            break;
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
