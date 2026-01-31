/**
 * @file printf.h
 * @author Lei Peng
 * @brief 
 * @version 0.1
 * @date 2026-01-31
 * 
 */
#pragma once

#include <array>
#include <string_view> // C++17

// Helper to identify format specifiers
constexpr bool is_specifier(char c) {
    constexpr std::string_view specifiers = "diuoxXfFeEgGaAcsp";
    return specifiers.find(c) != std::string_view::npos;
}

// 1. Calculate the size of the converted string (compile-time)
template <size_t N>
constexpr size_t count_brace_size(const char (&fmt)[N]) {
    size_t size = 0;
    for (size_t i = 0; i < N - 1; ++i) {
        if (fmt[i] == '%') {
            if (i + 1 < N && fmt[i + 1] == '%') { // %% -> %
                size++;
                i++;
            } else {
                size_t j = i + 1;
                while (j < N - 1 && !is_specifier(fmt[j]) && fmt[j] != '%')
                    j++;
                if (j < N - 1 && is_specifier(fmt[j])) {
                    size += 2; // {}
                    i = j;
                } else {
                    size++; // Invalid specifier found, treat '%' as literal char
                }
            }
        } else {
            size++;
        }
    }
    return size + 1; // null terminator
}

// 2. Generate the new brace-style format string (compile-time)
template <size_t N, size_t M>
constexpr std::array<char, M> to_brace_fmt(const char (&fmt)[N]) {
    std::array<char, M> res {};
    size_t ri = 0;
    for (size_t i = 0; i < N - 1; ++i) {
        if (fmt[i] == '%') {
            if (i + 1 < N && fmt[i + 1] == '%') {
                res[ri++] = '%';
                i++;
            } else {
                size_t j = i + 1;
                while (j < N - 1 && !is_specifier(fmt[j]) && fmt[j] != '%')
                    j++;

                if (j < N - 1 && is_specifier(fmt[j])) {
                    res[ri++] = '{';
                    res[ri++] = '}';
                    i = j;
                } else {
                    // Invalid specifier (like %q) or hit another %, treat original '%' as literal
                    res[ri++] = '%';
                }
            }
        } else {
            res[ri++] = fmt[i];
        }
    }
    res[ri] = '\0';
    return res;
}
// Explicit 'constexpr' to lambda.
#define map_to_spdlog(str_literal)                           \
    ([]() constexpr {                                        \
        constexpr size_t _N = sizeof(str_literal);           \
        constexpr size_t _M = count_brace_size(str_literal); \
        return to_brace_fmt<_N, _M>(str_literal);            \
    }())

// ================= static aassert test on map_to_spdlog start =================
constexpr auto test_fmt1 = map_to_spdlog("simple string without format");
static_assert(std::string_view(test_fmt1.data()) == "simple string without format");

constexpr auto test_fmt2 = map_to_spdlog("value: %d");
static_assert(std::string_view(test_fmt2.data()) == "value: {}");

constexpr auto test_fmt3 = map_to_spdlog("values: %d, %.2f, %s");
static_assert(std::string_view(test_fmt3.data()) == "values: {}, {}, {}");

constexpr auto test_fmt4 = map_to_spdlog("escaped percent: %% and value: %u");
static_assert(std::string_view(test_fmt4.data()) == "escaped percent: % and value: {}");
constexpr auto test_fmt5 = map_to_spdlog("mixed: %s %% %d %.3f %%");
static_assert(std::string_view(test_fmt5.data()) == "mixed: {} % {} {} %");

constexpr auto test_fmt6 = map_to_spdlog("complex: %08x, %+6.2f, %-10s, %%, %lld, (%llu)");
static_assert(std::string_view(test_fmt6.data()) == "complex: {}, {}, {}, %, {}, ({})");

constexpr auto test_fmt7 = map_to_spdlog("no specifier here: %q %z %y");
static_assert(std::string_view(test_fmt7.data()) == "no specifier here: %q %z %y");

constexpr auto test_fmt8 = map_to_spdlog("nospace: %d%.2f%s");
static_assert(std::string_view(test_fmt8.data()) == "nospace: {}{}{}");
//  ================= static aassert test on map_to_spdlog end =================

#define USE_SPDLOG_MACRO
#ifdef USE_SPDLOG_MACRO

// PRINTF("test logging: %s %d %.2f", "test", 123, 45.67);
#define PRINTF(fmt, ...) SPDLOG_INFO(map_to_spdlog(fmt).data(), ##__VA_ARGS__)

// LOG_STREAM << "test logging: " << "test" << 123 << 45.67;
#define LOG_STREAM ::SpdLogStream(spdlog::level::info, __FILE__, __LINE__, __FUNCTION__)

#endif
