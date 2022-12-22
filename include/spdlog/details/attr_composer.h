#pragma once

#include <string>

namespace spdlog {
namespace details {

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

}
}