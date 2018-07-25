//
// Created by gabi on 6/15/18.
//

#pragma once

#include "chrono"
#include "spdlog/fmt/fmt.h"

// Some fmt helpers to efficiently format and pad ints and strings
namespace spdlog {
namespace details {
namespace fmt_helper {

template<size_t Buffer_Size>
inline void append_str(const std::string &str, fmt::basic_memory_buffer<char, Buffer_Size> &dest)
{
    auto *str_ptr = str.data();
    dest.append(str_ptr, str_ptr + str.size());
}

template<size_t Buffer_Size>
inline void append_c_str(const char *c_str, fmt::basic_memory_buffer<char, Buffer_Size> &dest)
{
    char ch;
    while ((ch = *c_str) != '\0')
    {
        dest.push_back(ch);
        ++c_str;
    }
}

template<size_t Buffer_Size1, size_t Buffer_Size2>
inline void append_buf(const fmt::basic_memory_buffer<char, Buffer_Size1> &buf, fmt::basic_memory_buffer<char, Buffer_Size2> &dest)
{
    auto *buf_ptr = buf.data();
    dest.append(buf_ptr, buf_ptr + buf.size());
}

template<typename T, size_t Buffer_Size>
inline void append_int(T n, fmt::basic_memory_buffer<char, Buffer_Size> &dest)
{
    fmt::format_int i(n);
    dest.append(i.data(), i.data() + i.size());
}

template<size_t Buffer_Size>
inline void pad2(int n, fmt::basic_memory_buffer<char, Buffer_Size> &dest)
{
    if (n > 99)
    {
        append_int(n, dest);
        return;
    }
    if (n > 9) // 10-99
    {
        dest.push_back('0' + static_cast<char>(n / 10));
        dest.push_back('0' + static_cast<char>(n % 10));
        return;
    }
    if (n >= 0) // 0-9
    {
        dest.push_back('0');
        dest.push_back('0' + static_cast<char>(n));
        return;
    }
    // negatives (unlikely, but just in case, let fmt deal with it)
    fmt::format_to(dest, "{:02}", n);
}

template<size_t Buffer_Size>
inline void pad3(int n, fmt::basic_memory_buffer<char, Buffer_Size> &dest)
{
    if (n > 999)
    {
        append_int(n, dest);
        return;
    }

    if (n > 99) // 100-999
    {
        append_int(n / 100, dest);
        pad2(n % 100, dest);
        return;
    }
    if (n > 9) // 10-99
    {
        dest.push_back('0');
        dest.push_back('0' + static_cast<char>(n / 10));
        dest.push_back('0' + static_cast<char>(n % 10));
        return;
    }
    if (n >= 0)
    {
        dest.push_back('0');
        dest.push_back('0');
        dest.push_back('0' + static_cast<char>(n));
        return;
    }
    // negatives (unlikely, but just in case let fmt deal with it)
    fmt::format_to(dest, "{:03}", n);
}

template<size_t Buffer_Size>
inline void pad6(size_t n, fmt::basic_memory_buffer<char, Buffer_Size> &dest)
{
    if (n > 99999)
    {
        append_int(n, dest);
        return;
    }
    pad3(static_cast<int>(n / 1000), dest);
    pad3(static_cast<int>(n % 1000), dest);
}

// return fraction of a second of the given time_point.
// e.g.
// fraction<std::milliseconds>(tp) -> will return the millis part of the second
template<typename ToDuration>
inline ToDuration time_fraction(const log_clock::time_point &tp)
{
    using namespace std::chrono;
    auto duration = tp.time_since_epoch();
    auto secs = duration_cast<seconds>(duration);
    return duration_cast<ToDuration>(duration) - duration_cast<ToDuration>(secs);
}

} // namespace fmt_helper
} // namespace details
} // namespace spdlog