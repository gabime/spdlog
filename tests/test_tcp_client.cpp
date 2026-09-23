#include "includes.h"

#ifndef _WIN32
#include <spdlog/details/tcp_client.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

struct fd_guard {
    explicit fd_guard(int value) : fd(value) {}
    ~fd_guard() {
        if (fd >= 0) ::close(fd);
    }
    int fd;
};

struct fd_limit_guard {
    rlimit original{};
    bool changed = false;

    fd_limit_guard() {
        if (::getrlimit(RLIMIT_NOFILE, &original) == 0 && original.rlim_cur <= FD_SETSIZE &&
            original.rlim_max > FD_SETSIZE) {
            auto raised = original;
            raised.rlim_cur = FD_SETSIZE + 1;
            changed = ::setrlimit(RLIMIT_NOFILE, &raised) == 0;
        }
    }

    ~fd_limit_guard() {
        if (changed) ::setrlimit(RLIMIT_NOFILE, &original);
    }
};

TEST_CASE("tcp_client timeout supports high file descriptors", "[tcp_sink]") {
    fd_limit_guard limit;
    fd_guard listener(::socket(AF_INET, SOCK_STREAM, 0));
    REQUIRE(listener.fd >= 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    REQUIRE(::bind(listener.fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == 0);
    REQUIRE(::listen(listener.fd, 1) == 0);

    socklen_t addr_len = sizeof(addr);
    REQUIRE(::getsockname(listener.fd, reinterpret_cast<sockaddr *>(&addr), &addr_len) == 0);

    fd_guard socket_fd(::socket(AF_INET, SOCK_STREAM, 0));
    REQUIRE(socket_fd.fd >= 0);
    fd_guard high_fd(::fcntl(socket_fd.fd, F_DUPFD, FD_SETSIZE));
    if (high_fd.fd < 0) {
        WARN("file descriptor limit is too low for the high-fd test");
        return;
    }

    spdlog::details::tcp_client client;
    REQUIRE(client.connect_socket_with_timeout(
                high_fd.fd, reinterpret_cast<sockaddr *>(&addr), addr_len, 1000) == 0);
}
#endif
