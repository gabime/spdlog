// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <cstdint>

#if __has_include(<version>)
    #include <version>
#endif

#if __cpp_lib_source_location >= 201907
    #include <source_location>
    #define SPDLOG_HAVE_STD_SOURCE_LOCATION
#elif __has_include(<experimental/source_location>)
    #include <experimental/source_location>
    #define SPDLOG_HAVE_EXPERIMENTAL_SOURCE_LOCATION
#endif

namespace spdlog {
// source location - either initiated from std::source_location or from
// std::experimental::source_location or empty
struct source_loc {
    constexpr source_loc() = default;
    constexpr source_loc(const char *filename_in, std::uint_least32_t line_in, const char *funcname_in)
        : filename{filename_in},
          short_filename{basename(filename_in)},
          line{line_in},
          funcname{funcname_in} {}

#ifdef SPDLOG_HAVE_STD_SOURCE_LOCATION
    static constexpr source_loc current(const std::source_location source_location = std::source_location::current()) {
        return source_loc{source_location.file_name(), source_location.line(), source_location.function_name()};
    }
#elif defined(SPDLOG_HAVE_EXPERIMENTAL_SOURCE_LOCATION)
    static constexpr source_loc current(
        const std::experimental::source_location source_location = std::experimental::source_location::current()) {
        return source_loc{source_location.file_name(), source_location.line(), source_location.function_name()};
    }
#else  // no source location support
    static constexpr source_loc current() { return source_loc{}; }
#endif

    [[nodiscard]] constexpr bool empty() const noexcept { return line == 0 || filename == nullptr || short_filename == nullptr; }

    const char *filename{nullptr};
    const char *short_filename{nullptr};
    std::uint_least32_t line{0};
    const char *funcname{nullptr};

    // return filename without the leading path
    static constexpr const char *basename(const char *path) {
        const char *file = path;
        while (*path) {
#ifdef _WIN32
            if (*path == '\\' || *path == '/')
#else
            if (*path == '/')
#endif
            {
                file = path + 1;
            }
            ++path;
        }
        return file;
    }
};
}  // namespace spdlog