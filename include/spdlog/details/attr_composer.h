#pragma once

#include <string>
#include <spdlog/common.h>

namespace spdlog {
namespace details {

inline void scramble(std::string& dst, string_view_t s)
{
    if (s.size() == 0)
        return;

    auto start = s.data();
    auto const end = s.data() + s.size();
    auto cursor = start;

    dst.reserve(dst.size() + s.size());

    auto replace = [&](string_view_t with) {
        dst.append(start, size_t(cursor - start));
        ++cursor;
        start = cursor;
        // dst.append(with);
        dst.append(with.data(), with.size());
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