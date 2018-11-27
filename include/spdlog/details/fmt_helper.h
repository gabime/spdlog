//
// Created by gabi on 6/15/18.
//

#pragma once

#include <chrono>
#include <type_traits>
#include "spdlog/fmt/fmt.h"

// Some fmt helpers to efficiently format and pad ints and strings
namespace spdlog {
namespace details {
namespace fmt_helper {

template<size_t Buffer_Size>
inline spdlog::string_view_t to_string_view(const fmt::basic_memory_buffer<char, Buffer_Size> &buf) SPDLOG_NOEXCEPT
{
    return spdlog::string_view_t(buf.data(), buf.size());
}

template<size_t Buffer_Size1, size_t Buffer_Size2>
inline void append_buf(const fmt::basic_memory_buffer<char, Buffer_Size1> &buf, fmt::basic_memory_buffer<char, Buffer_Size2> &dest)
{
    auto *buf_ptr = buf.data();
    dest.append(buf_ptr, buf_ptr + buf.size());
}

template<size_t Buffer_Size>
inline void append_string_view(spdlog::string_view_t view, fmt::basic_memory_buffer<char, Buffer_Size> &dest)
{
    auto *buf_ptr = view.data();
    if (buf_ptr != nullptr)
    {
        dest.append(buf_ptr, buf_ptr + view.size());
    }
}

template<typename T, size_t Buffer_Size>
inline void append_int(T n, fmt::basic_memory_buffer<char, Buffer_Size> &dest)
{
    fmt::format_int i(n);
    dest.append(i.data(), i.data() + i.size());
}

template<typename T>
inline unsigned count_digits(T n)
{
    using count_type = typename std::conditional<(sizeof(T) > sizeof(uint32_t)), uint64_t, uint32_t>::type;
    return fmt::internal::count_digits(static_cast<count_type>(n));
}

template<size_t Buffer_Size>
inline void pad2(int n, fmt::basic_memory_buffer<char, Buffer_Size> &dest)
{
    if (n > 99)
    {
        append_int(n, dest);
    }
    else if (n > 9) // 10-99
    {
        dest.push_back(static_cast<char>('0' + n / 10));
        dest.push_back(static_cast<char>('0' + n % 10));
    }
    else if (n >= 0) // 0-9
    {
        dest.push_back('0');
        dest.push_back(static_cast<char>('0' + n));
    }
    else // negatives (unlikely, but just in case, let fmt deal with it)
    {
        fmt::format_to(dest, "{:02}", n);
    }
}

template<typename T, size_t Buffer_Size>
inline void pad_uint(T n, unsigned int width, fmt::basic_memory_buffer<char, Buffer_Size> &dest)
{
    static_assert(std::is_unsigned<T>::value, "pad_uint must get unsigned T");
    auto digits = count_digits(n);
    if (width > digits)
    {
        const char *zeroes = "0000000000000000000";
        dest.append(zeroes, zeroes + width - digits);
    }
    append_int(n, dest);
}

template<typename T, size_t Buffer_Size>
inline void pad3(T n, fmt::basic_memory_buffer<char, Buffer_Size> &dest)
{
    pad_uint(n, 3, dest);
}

template<typename T, size_t Buffer_Size>
inline void pad6(T n, fmt::basic_memory_buffer<char, Buffer_Size> &dest)
{
    pad_uint(n, 6, dest);
}

template<typename T, size_t Buffer_Size>
inline void pad9(T n, fmt::basic_memory_buffer<char, Buffer_Size> &dest)
{
    pad_uint(n, 9, dest);
}

// return fraction of a second of the given time_point.
// e.g.
// fraction<std::milliseconds>(tp) -> will return the millis part of the second
template<typename ToDuration>
inline ToDuration time_fraction(const log_clock::time_point &tp)
{
    using std::chrono::duration_cast;
    using std::chrono::seconds;
    auto duration = tp.time_since_epoch();
    auto secs = duration_cast<seconds>(duration);
    return duration_cast<ToDuration>(duration) - duration_cast<ToDuration>(secs);
}

} // namespace fmt_helper
} // namespace details
} // namespace spdlog
