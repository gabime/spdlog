// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <cstdint>

#include "./namespace.h"

SPDLOG_NAMESPACE_BEGIN  // source location
struct source_loc {
    constexpr source_loc() = default;
    constexpr source_loc(const char *filename_in, std::uint_least32_t line_in, const char *funcname_in)
        : filename{filename_in},
          short_filename{basename(filename_in)},
          line{line_in},
          funcname{funcname_in} {}
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
SPDLOG_NAMESPACE_END
