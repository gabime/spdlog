// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/sinks/base_sink.h>

#include <cstdint>
#include <mutex>
#include <string>

#ifdef _WIN32
#include <spdlog/details/windows_include.h>
#include <cctype>
#else
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace spdlog {
namespace sinks {

enum class serial_data_size : uint8_t { five = 5, six = 6, seven = 7, eight = 8 };

enum class serial_stop_bits : uint8_t { one = 1, two = 2 };

enum class serial_parity : uint8_t { none = 0, odd = 1, even = 2 };

enum class serial_handshake : uint8_t { none = 0, hardware_flow_control = 1 };

struct serial_port_sink_config {
    std::string port_name;
    uint32_t baud_rate = 115200;
    serial_data_size data_size = serial_data_size::eight;
    serial_stop_bits stop_bits = serial_stop_bits::one;
    serial_parity parity = serial_parity::none;
    serial_handshake handshake = serial_handshake::none;
    int write_timeout_ms = 1000;
    bool lazy_open = false;

    explicit serial_port_sink_config(std::string serial_port_name)
        : port_name{std::move(serial_port_name)} {}

    serial_port_sink_config(std::string serial_port_name,
                            uint32_t serial_baud_rate,
                            int serial_write_timeout_ms,
                            bool serial_lazy_open)
        : port_name{std::move(serial_port_name)},
          baud_rate{serial_baud_rate},
          write_timeout_ms{serial_write_timeout_ms},
          lazy_open{serial_lazy_open} {}

    serial_port_sink_config(std::string serial_port_name,
                            uint32_t serial_baud_rate,
                            serial_data_size serial_data_size_value = serial_data_size::eight,
                            serial_stop_bits serial_stop_bits_value = serial_stop_bits::one,
                            serial_parity serial_parity_value = serial_parity::none,
                            serial_handshake serial_handshake_value = serial_handshake::none,
                            int serial_write_timeout_ms = 1000,
                            bool serial_lazy_open = false)
        : port_name{std::move(serial_port_name)},
          baud_rate{serial_baud_rate},
          data_size{serial_data_size_value},
          stop_bits{serial_stop_bits_value},
          parity{serial_parity_value},
          handshake{serial_handshake_value},
          write_timeout_ms{serial_write_timeout_ms},
          lazy_open{serial_lazy_open} {}
};

namespace serial_port_details {

class serial_port {
public:
    serial_port() = default;

    serial_port(const serial_port&) = delete;
    serial_port& operator=(const serial_port&) = delete;

    ~serial_port() { close(); }

    void open(const serial_port_sink_config& config) {
        if (is_open()) {
            return;
        }

#ifdef _WIN32
        const std::string normalized_port_name = normalize_port_name_(config.port_name);
        handle_ = ::CreateFileA(normalized_port_name.c_str(), GENERIC_WRITE, 0, nullptr,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (handle_ == INVALID_HANDLE_VALUE) {
            throw_spdlog_ex("serial_port_sink: failed opening serial port",
                            static_cast<int>(::GetLastError()));
        }

        DCB dcb{};
        dcb.DCBlength = sizeof(DCB);
        if (!::GetCommState(handle_, &dcb)) {
            close();
            throw_spdlog_ex("serial_port_sink: GetCommState failed",
                            static_cast<int>(::GetLastError()));
        }

        dcb.BaudRate = config.baud_rate;
        dcb.ByteSize = to_win_data_size_(config.data_size);
        dcb.StopBits = to_win_stop_bits_(config.stop_bits);
        dcb.Parity = to_win_parity_(config.parity);
        dcb.fBinary = TRUE;
        dcb.fDtrControl = DTR_CONTROL_ENABLE;
        dcb.fOutxCtsFlow =
            config.handshake == serial_handshake::hardware_flow_control ? TRUE : FALSE;
        dcb.fRtsControl = config.handshake == serial_handshake::hardware_flow_control
                              ? RTS_CONTROL_HANDSHAKE
                              : RTS_CONTROL_ENABLE;

        if (!::SetCommState(handle_, &dcb)) {
            close();
            throw_spdlog_ex("serial_port_sink: SetCommState failed",
                            static_cast<int>(::GetLastError()));
        }

        COMMTIMEOUTS timeouts{};
        timeouts.WriteTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant =
            config.write_timeout_ms > 0 ? static_cast<DWORD>(config.write_timeout_ms) : 0;
        if (!::SetCommTimeouts(handle_, &timeouts)) {
            close();
            throw_spdlog_ex("serial_port_sink: SetCommTimeouts failed",
                            static_cast<int>(::GetLastError()));
        }
#else
        fd_ = ::open(config.port_name.c_str(), O_WRONLY | O_NOCTTY | O_SYNC);
        if (fd_ < 0) {
            throw_spdlog_ex("serial_port_sink: failed opening serial port", errno);
        }

        termios tty{};
        if (::tcgetattr(fd_, &tty) != 0) {
            close();
            throw_spdlog_ex("serial_port_sink: tcgetattr failed", errno);
        }

        ::cfmakeraw(&tty);
        tty.c_cflag |= static_cast<unsigned int>(CLOCAL | CREAD);
        tty.c_cflag &= static_cast<unsigned int>(~CSIZE);
        tty.c_cflag |= to_posix_data_size_(config.data_size);

        if (config.stop_bits == serial_stop_bits::two) {
            tty.c_cflag |= static_cast<unsigned int>(CSTOPB);
        } else {
            tty.c_cflag &= static_cast<unsigned int>(~CSTOPB);
        }

        switch (config.parity) {
            case serial_parity::none:
                tty.c_cflag &= static_cast<unsigned int>(~PARENB);
                tty.c_cflag &= static_cast<unsigned int>(~PARODD);
                break;
            case serial_parity::odd:
                tty.c_cflag |= static_cast<unsigned int>(PARENB);
                tty.c_cflag |= static_cast<unsigned int>(PARODD);
                break;
            case serial_parity::even:
                tty.c_cflag |= static_cast<unsigned int>(PARENB);
                tty.c_cflag &= static_cast<unsigned int>(~PARODD);
                break;
        }

#ifdef CRTSCTS
        if (config.handshake == serial_handshake::hardware_flow_control) {
            tty.c_cflag |= static_cast<unsigned int>(CRTSCTS);
        } else {
            tty.c_cflag &= static_cast<unsigned int>(~CRTSCTS);
        }
#endif

        const speed_t speed = to_speed_(config.baud_rate);
        if (::cfsetispeed(&tty, speed) != 0 || ::cfsetospeed(&tty, speed) != 0) {
            close();
            throw_spdlog_ex("serial_port_sink: failed setting baud rate", errno);
        }

        if (::tcsetattr(fd_, TCSANOW, &tty) != 0) {
            close();
            throw_spdlog_ex("serial_port_sink: tcsetattr failed", errno);
        }
#endif
    }

    void write(const char* data, size_t size) {
        if (!is_open()) {
            throw_spdlog_ex("serial_port_sink: serial port is not open");
        }

#ifdef _WIN32
        size_t offset = 0;
        while (offset < size) {
            DWORD written = 0;
            const DWORD chunk = static_cast<DWORD>(size - offset);
            if (!::WriteFile(handle_, data + offset, chunk, &written, nullptr)) {
                throw_spdlog_ex("serial_port_sink: WriteFile failed",
                                static_cast<int>(::GetLastError()));
            }
            if (written == 0) {
                throw_spdlog_ex("serial_port_sink: WriteFile returned zero bytes");
            }
            offset += static_cast<size_t>(written);
        }
#else
        size_t offset = 0;
        while (offset < size) {
            const ssize_t written = ::write(fd_, data + offset, size - offset);
            if (written < 0) {
                if (errno == EINTR) {
                    continue;
                }
                throw_spdlog_ex("serial_port_sink: write failed", errno);
            }
            if (written == 0) {
                throw_spdlog_ex("serial_port_sink: write returned zero bytes");
            }
            offset += static_cast<size_t>(written);
        }
#endif
    }

    void flush() {
#ifdef _WIN32
        if (is_open() && !::FlushFileBuffers(handle_)) {
            throw_spdlog_ex("serial_port_sink: FlushFileBuffers failed",
                            static_cast<int>(::GetLastError()));
        }
#else
        if (is_open() && ::tcdrain(fd_) != 0) {
            throw_spdlog_ex("serial_port_sink: tcdrain failed", errno);
        }
#endif
    }

    bool is_open() const {
#ifdef _WIN32
        return handle_ != INVALID_HANDLE_VALUE;
#else
        return fd_ >= 0;
#endif
    }

    void close() {
#ifdef _WIN32
        if (handle_ != INVALID_HANDLE_VALUE) {
            ::CloseHandle(handle_);
            handle_ = INVALID_HANDLE_VALUE;
        }
#else
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
#endif
    }

private:
#ifdef _WIN32
    static BYTE to_win_data_size_(serial_data_size data_size) {
        return static_cast<BYTE>(data_size);
    }

    static BYTE to_win_stop_bits_(serial_stop_bits stop_bits) {
        return stop_bits == serial_stop_bits::two ? TWOSTOPBITS : ONESTOPBIT;
    }

    static BYTE to_win_parity_(serial_parity parity) {
        switch (parity) {
            case serial_parity::none:
                return NOPARITY;
            case serial_parity::odd:
                return ODDPARITY;
            case serial_parity::even:
                return EVENPARITY;
            default:
                return NOPARITY;
        }
    }

    static std::string normalize_port_name_(const std::string& port_name) {
        if (port_name.rfind("\\\\.\\", 0) == 0) {
            return port_name;
        }

        if (port_name.size() >= 4 && (port_name[0] == 'C' || port_name[0] == 'c') &&
            (port_name[1] == 'O' || port_name[1] == 'o') &&
            (port_name[2] == 'M' || port_name[2] == 'm')) {
            bool all_digits = true;
            for (size_t i = 3; i < port_name.size(); ++i) {
                if (!std::isdigit(static_cast<unsigned char>(port_name[i]))) {
                    all_digits = false;
                    break;
                }
            }

            if (all_digits) {
                return std::string("\\\\.\\") + port_name;
            }
        }

        return port_name;
    }
#endif

#ifndef _WIN32
    static tcflag_t to_posix_data_size_(serial_data_size data_size) {
        switch (data_size) {
            case serial_data_size::five:
                return CS5;
            case serial_data_size::six:
                return CS6;
            case serial_data_size::seven:
                return CS7;
            case serial_data_size::eight:
            default:
                return CS8;
        }
    }

    static speed_t to_speed_(uint32_t baud_rate) {
        switch (baud_rate) {
            case 50:
                return B50;
            case 75:
                return B75;
            case 110:
                return B110;
            case 134:
                return B134;
            case 150:
                return B150;
            case 200:
                return B200;
            case 300:
                return B300;
            case 600:
                return B600;
            case 1200:
                return B1200;
            case 1800:
                return B1800;
            case 2400:
                return B2400;
            case 4800:
                return B4800;
            case 9600:
                return B9600;
            case 19200:
                return B19200;
            case 38400:
                return B38400;
#ifdef B57600
            case 57600:
                return B57600;
#endif
#ifdef B115200
            case 115200:
                return B115200;
#endif
#ifdef B230400
            case 230400:
                return B230400;
#endif
#ifdef B460800
            case 460800:
                return B460800;
#endif
#ifdef B500000
            case 500000:
                return B500000;
#endif
#ifdef B576000
            case 576000:
                return B576000;
#endif
#ifdef B921600
            case 921600:
                return B921600;
#endif
#ifdef B1000000
            case 1000000:
                return B1000000;
#endif
#ifdef B1152000
            case 1152000:
                return B1152000;
#endif
#ifdef B1500000
            case 1500000:
                return B1500000;
#endif
#ifdef B2000000
            case 2000000:
                return B2000000;
#endif
#ifdef B2500000
            case 2500000:
                return B2500000;
#endif
#ifdef B3000000
            case 3000000:
                return B3000000;
#endif
#ifdef B3500000
            case 3500000:
                return B3500000;
#endif
#ifdef B4000000
            case 4000000:
                return B4000000;
#endif
            default:
                throw_spdlog_ex("serial_port_sink: unsupported baud rate");
        }
    }
#endif

#ifdef _WIN32
    HANDLE handle_ = INVALID_HANDLE_VALUE;
#else
    int fd_ = -1;
#endif
};

}  // namespace serial_port_details

template <typename Mutex>
class serial_port_sink : public base_sink<Mutex> {
public:
    explicit serial_port_sink(serial_port_sink_config sink_config)
        : config_{std::move(sink_config)} {
        if (!config_.lazy_open) {
            serial_port_.open(config_);
        }
    }

    ~serial_port_sink() override = default;

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        if (!serial_port_.is_open()) {
            serial_port_.open(config_);
        }
        serial_port_.write(formatted.data(), formatted.size());
    }

    void flush_() override { serial_port_.flush(); }

private:
    serial_port_sink_config config_;
    serial_port_details::serial_port serial_port_;
};

using serial_port_sink_mt = serial_port_sink<std::mutex>;
using serial_port_sink_st = serial_port_sink<spdlog::details::null_mutex>;

}  // namespace sinks

//
// factory functions
//
template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> serial_logger_mt(const std::string& logger_name,
                                                sinks::serial_port_sink_config sink_config) {
    return Factory::template create<sinks::serial_port_sink_mt>(logger_name,
                                                                std::move(sink_config));
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> serial_logger_st(const std::string& logger_name,
                                                sinks::serial_port_sink_config sink_config) {
    return Factory::template create<sinks::serial_port_sink_st>(logger_name,
                                                                std::move(sink_config));
}

}  // namespace spdlog
