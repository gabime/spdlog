// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#pragma once

#include <chrono>
#include <iterator>
#include <spdlog/common.h>
#include <spdlog/fmt/fmt.h>
#include <type_traits>

#ifdef SPDLOG_USE_STD_FORMAT
#include <charconv>
#include <limits>
#endif

// Some fmt helpers to efficiently format and pad ints and strings
SPDLOG_NAMESPACE_BEGIN
namespace details {
namespace fmt_helper {

inline void append_string_view(string_view_t view, memory_buf_t &dest) {
    auto *buf_ptr = view.data();
    dest.append(buf_ptr, buf_ptr + view.size());
}

#ifdef SPDLOG_USE_STD_FORMAT
template <typename T>
inline void append_int(T n, memory_buf_t &dest) {
    // Buffer should be large enough to hold all digits (digits10 + 1) and a sign
    SPDLOG_CONSTEXPR const auto BUF_SIZE = std::numeric_limits<T>::digits10 + 2;
    char buf[BUF_SIZE];

    auto [ptr, ec] = std::to_chars(buf, buf + BUF_SIZE, n, 10);
    if (ec == std::errc()) {
        dest.append(buf, ptr);
    } else {
        throw_spdlog_ex("Failed to format int", static_cast<int>(ec));
    }
}
#else
template <typename T>
inline void append_int(T n, memory_buf_t &dest) {
    fmt::format_int i(n);
    dest.append(i.data(), i.data() + i.size());
}
#endif

template <typename T>
SPDLOG_CONSTEXPR_FUNC unsigned int count_digits_fallback(T n) {
    // taken from fmt: https://github.com/fmtlib/fmt/blob/8.0.1/include/fmt/format.h#L899-L912
    unsigned int count = 1;
    for (;;) {
        // Integer division is slow so do it for a group of four digits instead
        // of for every digit. The idea comes from the talk by Alexandrescu
        // "Three Optimization Tips for C++". See speed-test for a comparison.
        if (n < 10) return count;
        if (n < 100) return count + 1;
        if (n < 1000) return count + 2;
        if (n < 10000) return count + 3;
        n /= 10000u;
        count += 4;
    }
}

template <typename T>
inline unsigned int count_digits(T n) {
    using count_type =
        typename std::conditional<(sizeof(T) > sizeof(uint32_t)), uint64_t, uint32_t>::type;
#ifdef SPDLOG_USE_STD_FORMAT
    return count_digits_fallback(static_cast<count_type>(n));
#else
    return static_cast<unsigned int>(fmt::
// fmt 7.0.0 renamed the internal namespace to detail.
// See: https://github.com/fmtlib/fmt/issues/1538
#if FMT_VERSION < 70000
                                         internal
#else
                                         detail
#endif
                                     ::count_digits(static_cast<count_type>(n)));
#endif
}

inline const char *digits2_table() SPDLOG_NOEXCEPT {
    static constexpr char table[201] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";
    return table;
}

inline void pad2(int n, memory_buf_t &dest) {
    if (n >= 0 && n < 100)  // 0-99
    {
        const char *d = digits2_table() + static_cast<size_t>(n) * 2;
        dest.append(d, d + 2);
    } else  // unlikely, but just in case, let fmt deal with it
    {
        fmt_lib::format_to(std::back_inserter(dest), SPDLOG_FMT_STRING("{:02}"), n);
    }
}

template <typename T>
inline void pad_uint(T n, unsigned int width, memory_buf_t &dest) {
    static_assert(std::is_unsigned<T>::value, "pad_uint must get unsigned T");
    for (auto digits = count_digits(n); digits < width; digits++) {
        dest.push_back('0');
    }
    append_int(n, dest);
}

template <typename T>
inline void pad3(T n, memory_buf_t &dest) {
    static_assert(std::is_unsigned<T>::value, "pad3 must get unsigned T");
    if (n < 1000) {
        const auto v = static_cast<uint32_t>(n);
        const auto q = v / 100;
        const auto r = v - q * 100;
        const char *d = digits2_table() + r * 2;
        char buf[3] = {static_cast<char>('0' + q), d[0], d[1]};
        dest.append(buf, buf + 3);
    } else {
        append_int(n, dest);
    }
}

template <typename T>
inline void pad6(T n, memory_buf_t &dest) {
    static_assert(std::is_unsigned<T>::value, "pad6 must get unsigned T");
    if (n < 1000000) {
        const char *table = digits2_table();
        auto v = static_cast<uint32_t>(n);
        const auto hi = v / 10000;
        v -= hi * 10000;
        const auto mid = v / 100;
        const auto lo = v - mid * 100;
        const char *d0 = table + hi * 2;
        const char *d1 = table + mid * 2;
        const char *d2 = table + lo * 2;
        char buf[6] = {d0[0], d0[1], d1[0], d1[1], d2[0], d2[1]};
        dest.append(buf, buf + 6);
    } else {
        append_int(n, dest);
    }
}

template <typename T>
inline void pad9(T n, memory_buf_t &dest) {
    static_assert(std::is_unsigned<T>::value, "pad9 must get unsigned T");
    if (n < 1000000000) {
        const char *table = digits2_table();
        auto v = static_cast<uint32_t>(n);
        const auto d0 = v / 100000000;
        v -= d0 * 100000000;
        const auto p1 = v / 1000000;
        v -= p1 * 1000000;
        const auto p2 = v / 10000;
        v -= p2 * 10000;
        const auto p3 = v / 100;
        const auto p4 = v - p3 * 100;
        const char *t1 = table + p1 * 2;
        const char *t2 = table + p2 * 2;
        const char *t3 = table + p3 * 2;
        const char *t4 = table + p4 * 2;
        char buf[9] = {static_cast<char>('0' + d0), t1[0], t1[1], t2[0], t2[1], t3[0], t3[1],
                       t4[0], t4[1]};
        dest.append(buf, buf + 9);
    } else {
        append_int(n, dest);
    }
}

// return fraction of a second of the given time_point.
// e.g.
// fraction<std::milliseconds>(tp) -> will return the millis part of the second
template <typename ToDuration>
inline ToDuration time_fraction(log_clock::time_point tp) {
    using std::chrono::duration_cast;
    using std::chrono::seconds;
    auto duration = tp.time_since_epoch();
    auto secs = duration_cast<seconds>(duration);
    return duration_cast<ToDuration>(duration) - duration_cast<ToDuration>(secs);
}

}  // namespace fmt_helper
}  // namespace details
SPDLOG_NAMESPACE_END
