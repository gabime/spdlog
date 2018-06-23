//
// Created by gabi on 6/15/18.
//

#pragma once

// Some fmt helpers to efficiently format and pad ints and strings
namespace spdlog {
namespace details {
namespace fmt_helper {
inline void append_str(const std::string &str, fmt::memory_buffer &dest)
{
    const char *str_ptr = str.data();
    dest.append(str_ptr, str_ptr + str.size());
}

inline void append_c_str(const char *c_str, fmt::memory_buffer &dest)
{
    auto str_size = strlen(c_str);
    dest.append(c_str, c_str + str_size);
}

inline void append_buf(const fmt::memory_buffer &buf, fmt::memory_buffer &dest)
{
    const char *buf_ptr = buf.data();
    dest.append(buf_ptr, buf_ptr + buf.size());
}

inline void append_int(int n, fmt::memory_buffer &dest)
{
    fmt::format_int i(n);
    dest.append(i.data(), i.data() + i.size());
}

inline void append_size_t(size_t n, fmt::memory_buffer &dest)
{
    fmt::format_int i(n);
    dest.append(i.data(), i.data() + i.size());
}

inline void append_and_pad2(int n, fmt::memory_buffer &dest)
{
    if (n > 9)
    {
        append_int(n, dest);
        return;
    }
    if (n >= 0) // 0-9
    {
        dest.push_back('0');
        append_int(n, dest);
        return;
    }
    // negatives (unlikely but just in case, let fmt deal with it)
    fmt::format_to(dest, "{:02}", n);
}

inline void append_and_pad3(int n, fmt::memory_buffer &dest)
{
    if (n > 99)
    {
        append_int(n, dest);
        return;
    }
    if (n > 9) // 10-99
    {
        dest.push_back('0');
    }
    else if (n >= 0)
    {
        dest.push_back('0');
        dest.push_back('0');
    }
    // negatives (unlikely, but just in case let fmt deal with it)
    else
    {
        fmt::format_to(dest, "{:03}", n);
        return;
    }
    append_int(n, dest);
}

inline void append_and_pad6(int n, fmt::memory_buffer &dest)
{
    if (n > 99999)
    {
        append_int(n, dest);
        return;
    }
    if (n > 9999)
    {
        dest.push_back('0');
    }
    else if (n > 999)
    {
        dest.push_back('0');
        dest.push_back('0');
    }
    else if (n > 99)
    {
        dest.push_back('0');
        dest.push_back('0');
        dest.push_back('0');
    }
    else if (n > 9)
    {
        dest.push_back('0');
        dest.push_back('0');
        dest.push_back('0');
        dest.push_back('0');
    }
    else if (n >= 0)
    {
        dest.push_back('0');
        dest.push_back('0');
        dest.push_back('0');
        dest.push_back('0');
        dest.push_back('0');
    }
    else // negatives (unlikely, but just in case let fmt deal with it)
    {
        fmt::format_to(dest, "{:06}", n);
        return;
    }
    append_int(n, dest);
}

} // namespace fmt_helper
} // namespace details
} // namespace spdlog