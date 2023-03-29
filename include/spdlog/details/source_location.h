// Copyright(c) 2015-present, Gabi Melman, mguludag & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#pragma once

#include <cstdint>

#ifdef __has_include
#    if __has_include(<source_location>) && __cplusplus >= 202002L
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
  current(const char *fileName = __builtin_FILE(),
          const unsigned lineNumber = __builtin_LINE(),
          const char *functionName = __builtin_FUNCTION(),
          const unsigned columnOffset = __builtin_COLUMN()) noexcept
#elif defined(__GNUC__) &&                                                    \
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
  static constexpr source_location
  current(const char *fileName = __builtin_FILE(),
          const unsigned lineNumber = __builtin_LINE(),
          const char *functionName = __builtin_FUNCTION(),
          const unsigned columnOffset = 0) noexcept
#elif defined(_MSC_VER) && (_MSC_VER > 1925)
  static constexpr source_location
  current(const char *fileName = __builtin_FILE(),
          const unsigned lineNumber = __builtin_LINE(),
          const char *functionName = __builtin_FUNCTION(),
          const unsigned columnOffset = __builtin_COLUMN()) noexcept
#else
  static constexpr source_location
  current(const char *fileName = "unsupported",
          const unsigned lineNumber = 0,
          const char *functionName = "unsupported",
          const unsigned columnOffset = 0) noexcept
#endif
  {
    return source_location(fileName, lineNumber, functionName, columnOffset);
  }

  source_location(const source_location &) = default;
  source_location(source_location &&) = default;

  constexpr const char *file_name() const noexcept { return fileName; }

  constexpr const char *function_name() const noexcept { return functionName; }

  constexpr unsigned line() const noexcept { return lineNumber; }

  constexpr unsigned column() const noexcept { return columnOffset; }

private:
  constexpr source_location(const char *fileName,
                            const unsigned lineNumber,
                            const char *functionName,
                            const unsigned columnOffset) noexcept
      : fileName(fileName), lineNumber(lineNumber), functionName(functionName),
        columnOffset(columnOffset) {}

  const char *fileName;
  const unsigned lineNumber;
  const char *functionName;
  const unsigned columnOffset;
};
} // namespace details
} // namespace spdlog
#endif
