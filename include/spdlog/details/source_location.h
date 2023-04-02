// Copyright(c) 2015-present, Gabi Melman, mguludag & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#pragma once

#include <cstdint>

#ifdef _MSC_VER
#    define SPDLOG_CPLUSPLUS _MSVC_LANG
#else
#    define SPDLOG_CPLUSPLUS __cplusplus
#endif

#ifdef __has_include
#    if __has_include(<source_location>) && SPDLOG_CPLUSPLUS >= 202002L
#        include <source_location>
#        define SPDLOG_SOURCE_LOCATION
#    endif
#endif

#ifdef SPDLOG_SOURCE_LOCATION
namespace spdlog::details {
    using source_location = std::source_location;
}
#else
namespace spdlog {
namespace details {
struct source_location {
public:
#if  !defined(__apple_build_version__) && defined(__clang__) &&            \
    (__clang_major__ >= 9)
  static constexpr source_location
  current(const char *file = __builtin_FILE(),
          const unsigned line = __builtin_LINE(),
          const char *function = __builtin_FUNCTION(),
          const unsigned column = __builtin_COLUMN()) noexcept
#elif defined(__GNUC__) &&                                                    \
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
  static constexpr source_location
  current(const char *file = __builtin_FILE(),
          const unsigned line = __builtin_LINE(),
          const char *function = __builtin_FUNCTION(),
          const unsigned column = 0) noexcept
#elif defined(_MSC_VER) && (_MSC_VER > 1925)
  static constexpr source_location
  current(const char *file = __builtin_FILE(),
          const unsigned line = __builtin_LINE(),
          const char *function = __builtin_FUNCTION(),
          const unsigned column = __builtin_COLUMN()) noexcept
#else
  static constexpr source_location
  current(const char *file = nullptr,
          const unsigned line = 0,
          const char *function = nullptr,
          const unsigned column = 0) noexcept
#endif
  {
    return source_location(file, line, function, column);
  }

  source_location(const source_location &) = default;
  source_location(source_location &&) = default;

  constexpr const char *file_name() const noexcept { return file_; }

  constexpr const char *function_name() const noexcept { return function_; }

  constexpr unsigned line() const noexcept { return line_; }

  constexpr unsigned column() const noexcept { return column_; }

private:
  constexpr source_location(const char *file,
                            const unsigned line,
                            const char *function,
                            const unsigned column) noexcept
      : file_(file), line_(line), function_(function),
        column_(column) {}

  const char *file_;
  const unsigned line_;
  const char *function_;
  const unsigned column_;
};
} // namespace details
} // namespace spdlog
#endif
