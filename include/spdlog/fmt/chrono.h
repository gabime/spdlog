//
// Copyright(c) 2016 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#if defined(SPDLOG_USE_STD_FORMAT)
// Add a formatter for std::tm, since std::format only supports std::chrono
// taken from fmtlib
#include <cassert>
#include <ctime>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include <spdlog/common.h>

namespace spdlog::details
{
    inline size_t strftime(char *str, size_t count, const char *format, const std::tm *time)
    {
        // Assign to a pointer to suppress GCCs -Wformat-nonliteral
        // First assign the nullptr to suppress -Wsuggest-attribute=format
        std::size_t (*strftime)(char*, std::size_t, const char*, const std::tm*) = nullptr;
        strftime = std::strftime;
        return strftime(str, count, format, time);
    }

    inline size_t strftime(wchar_t *str, size_t count, const wchar_t *format, const std::tm *time)
    {
        // See above
        std::size_t (*wcsftime)(wchar_t*, std::size_t, const wchar_t*, const std::tm*) = nullptr;
        wcsftime = std::wcsftime;
        return wcsftime(str, count, format, time);
    }

    // Casts a nonnegative integer to unsigned.
    template <typename Int>
    SPDLOG_CONSTEXPR auto to_unsigned(Int value) -> typename std::make_unsigned<Int>::type
    {
        assert(value >= 0, "negative value");
        return static_cast<typename std::make_unsigned<Int>::type>(value);
    }
}

template <typename Char>
struct std::formatter<std::tm, Char>
{
    template <typename ParseContext>
    SPDLOG_CONSTEXPR_FUNC auto parse(ParseContext &ctx) -> decltype(ctx.begin())
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it == ':') ++it;
        auto end = it;
        while (end != ctx.end() && *end != '}') ++end;
        specs = {it, spdlog::details::to_unsigned(end - it)};
        return end;
    }

    template <typename FormatContext>
    auto format(const std::tm &tm, FormatContext &ctx) const -> decltype(ctx.out()) {
        basic_string<Char> tm_format;
        tm_format.append(specs);
        // By appending an extra space we can distinguish an empty result that
        // indicates insufficient buffer size from a guaranteed non-empty result
        // https://github.com/fmtlib/fmt/issues/2238
        tm_format.push_back(' ');

        const size_t MIN_SIZE = 10;
        basic_string<Char> buf;
        buf.resize(MIN_SIZE);
        for (;;)
        {
            size_t count = spdlog::details::strftime(buf.data(), buf.size(), tm_format.c_str(), &tm);
            if (count != 0)
            {
                buf.resize(count);
                break;
            }
            buf.resize(buf.size() * 2);
        }
        // Remove the extra space.
        return std::copy(buf.begin(), buf.end() - 1, ctx.out());
    }

    basic_string_view<Char> specs;
};

#else
//
// include bundled or external copy of fmtlib's chrono support
//
#    if !defined(SPDLOG_FMT_EXTERNAL)
#        ifdef SPDLOG_HEADER_ONLY
#            ifndef FMT_HEADER_ONLY
#                define FMT_HEADER_ONLY
#            endif
#        endif
#        include <spdlog/fmt/bundled/chrono.h>
#    else
#        include <fmt/chrono.h>
#    endif
#endif
