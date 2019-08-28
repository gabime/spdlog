//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

//
// Support for logging binary data as hex
// format flags:
// {:X} - print in uppercase.
// {:s} - don't separate each byte with space.
// {:p} - don't print the position on each line start.
// {:n} - don't split the output to lines.

//
// Examples:
//
// std::vector<char> v(200, 0x0b);
// logger->info("Some buffer {}", spdlog::to_hex(v));
// char buf[128];
// logger->info("Some buffer {:X}", spdlog::to_hex(std::begin(buf), std::end(buf)));

namespace spdlog {
namespace details {

template<typename It>
class bytes_range
{
public:
    bytes_range(It range_begin, It range_end)
        : begin_(range_begin)
        , end_(range_end)
    {}

    It begin() const
    {
        return begin_;
    }
    It end() const
    {
        return end_;
    }

private:
    It begin_, end_;
};
} // namespace details

// create a bytes_range that wraps the given container
template<typename Container>
inline details::bytes_range<typename Container::const_iterator> to_hex(const Container &container)
{
    static_assert(sizeof(typename Container::value_type) == 1, "sizeof(Container::value_type) != 1");
    using Iter = typename Container::const_iterator;
    return details::bytes_range<Iter>(std::begin(container), std::end(container));
}

// create bytes_range from ranges
template<typename It>
inline details::bytes_range<It> to_hex(const It range_begin, const It range_end)
{
    return details::bytes_range<It>(range_begin, range_end);
}

} // namespace spdlog

namespace fmt {

template<typename T>
struct formatter<spdlog::details::bytes_range<T>>
{
    const std::size_t line_size = 100;
    const char delimiter = ' ';

    bool put_newlines = true;
    bool put_delimiters = true;
    bool use_uppercase = false;
    bool put_positions = true; // position on start of each line

    // parse the format string flags
    template<typename ParseContext>
    auto parse(ParseContext &ctx) -> decltype(ctx.begin())
    {
        auto it = ctx.begin();
        while (*it && *it != '}')
        {
            switch (*it)
            {
            case 'X':
                use_uppercase = true;
                break;
            case 's':
                put_delimiters = false;
                break;
            case 'p':
                put_positions = false;
                break;
            case 'n':
                put_newlines = false;
                break;
            }

            ++it;
        }
        return it;
    }

    // format the given bytes range as hex
    template<typename FormatContext, typename Container>
    auto format(const spdlog::details::bytes_range<Container> &the_range, FormatContext &ctx) -> decltype(ctx.out())
    {
        SPDLOG_CONSTEXPR const char *hex_upper = "0123456789ABCDEF";
        SPDLOG_CONSTEXPR const char *hex_lower = "0123456789abcdef";
        const char *hex_chars = use_uppercase ? hex_upper : hex_lower;

        std::size_t pos = 0;
        std::size_t column = line_size;
#if FMT_VERSION < 60000
        auto inserter = ctx.begin();
#else
        auto inserter = ctx.out();
#endif

        for (auto &item : the_range)
        {
            auto ch = static_cast<unsigned char>(item);
            pos++;

            if (put_newlines && column >= line_size)
            {
                column = put_newline(inserter, pos);

                // put first byte without delimiter in front of it
                *inserter++ = hex_chars[(ch >> 4) & 0x0f];
                *inserter++ = hex_chars[ch & 0x0f];
                column += 2;
                continue;
            }

            if (put_delimiters)
            {
                *inserter++ = delimiter;
                ++column;
            }

            *inserter++ = hex_chars[(ch >> 4) & 0x0f];
            *inserter++ = hex_chars[ch & 0x0f];
            column += 2;
        }
        return inserter;
    }

    // put newline(and position header)
    // return the next column
    template<typename It>
    std::size_t put_newline(It inserter, std::size_t pos)
    {
#ifdef _WIN32
        *inserter++ = '\r';
#endif
        *inserter++ = '\n';

        if (put_positions)
        {
            fmt::format_to(inserter, "{:<04X}: ", pos - 1);
            return 7;
        }
        else
        {
            return 1;
        }
    }
};
} // namespace fmt
