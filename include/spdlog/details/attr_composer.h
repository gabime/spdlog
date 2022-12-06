#pragma once 

#include <string>
#include <concepts>
#include <type_traits>

namespace spdlog {
namespace detail {

template <typename T>
requires std::integral<T>
auto to_string(T const& v, T const& base) -> std::string
{
    // make sure we have enough to cover: negative base 2
    static constexpr std::size_t nbytes = sizeof(T) * 8 + 1 * std::is_signed_v(T);
    std::array<char, nbytes> buf;
    auto [p, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), v);
    return std::string(buf.data(), std::size_t(p - buf.data()));
}

inline void scramble(std::string& dst, std::string_view s)
{
    if (s.empty())
        return;

    auto start = s.data();
    auto const end = s.data() + s.size();
    auto cursor = start;

    dst.reserve(dst.size() + s.size());

    auto replace = [&](std::string_view with) {
        dst.append(start, size_t(cursor - start));
        ++cursor;
        start = cursor;
        dst.append(with);
    };

    while (cursor != end) {
        auto c = static_cast<unsigned char>(*cursor);

        switch (c) {
        case '\b':
            replace("\\b");
            break;
        case '\f':
            replace("\\f");
            break;
        case '\n':
            replace("\\n");
            break;
        case '\r':
            replace("\\r");
            break;
        case '\t':
            replace("\\t");
            break;
        case '\\':
            replace("\\\\");
            break;
        case '"':
            replace("\\\"");
            break;
        default:
            if (c <= '\x0f') {
                char buf[] = "\\u0000";
                buf[5] += c;
                if (c >= '\x0a')
                    buf[5] += 'a' - ':';
                replace(buf);
            }
            else if (c <= '\x1f' || c == 0x7f) {
                char buf[] = "\\u0010";
                buf[5] += c - 16;
                if (c >= '\x1a')
                    buf[5] += 'a' - ':';
                replace(buf);
            }
            else
                ++cursor;
        }
    }
    if (cursor != start)
        dst.append({start, size_t(cursor - start)});
}

inline auto compose_prefix(std::string_view k, std::size_t value_bytes) -> std::string
{
    auto dst = std::string{};
    dst.reserve(6 + k.size() + value_bytes);
    dst += ',';
    dst += '"';
    dst += k;
    dst += '"';
    dst += ':';
    dst += '"';
}

auto compose(std::string_view k, std::string_view const& v) -> std::string
{
    auto dst = compose_prefix(k, v.size());
    scramble(dst, std::string_view{v});
    dst += '"';
    return dst;
}
inline auto compose(std::string_view k, bool v) -> std::string
{
    auto dst = compose_prefix(k, 4 + !v);
    dst += v ? "true\"" : "false\"";
    return dst;
}

template <typename T>
    requires(std::integral<T> || std::floating_point<T>)
auto compose(std::string_view k, T const& v) -> std::string
{
    auto dst = compose_prefix(k, 1);
    dst += std::to_string(v);
    dst += '"';
    return dst;
}

template <typename T>
    requires std::integral<T>
auto compose(std::string_view k, T const& v, int base) -> std::string
{
    auto dst = compose_prefix(k, 1);
    dst += to_string(v, base);
    dst += '"';
    return dst;
}
}
}