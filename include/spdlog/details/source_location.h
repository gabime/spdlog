// Copyright(c) 2015-present, Gabi Melman, mguludag & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#pragma once

#include <cstdint>

namespace spdlog {
namespace details {
struct source_location {
public:
#if  !defined(__apple_build_version__) && defined(__clang__) &&            \
    (__clang_major__ >= 9)
  static constexpr source_location
  current(const char *fileName = __builtin_FILE(),
          const int lineNumber = __builtin_LINE(),
          const char *functionName = __builtin_FUNCTION(),
          const int columnOffset = __builtin_COLUMN()) noexcept
#elif defined(__GNUC__) &&                                                    \
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
  static constexpr source_location
  current(const char *fileName = __builtin_FILE(),
          const int lineNumber = __builtin_LINE(),
          const char *functionName = __builtin_FUNCTION(),
          const int columnOffset = 0) noexcept
#elif defined(_MSC_VER) && (_MSC_VER > 1925)
  static constexpr source_location
  current(const char *fileName = __builtin_FILE(),
          const int lineNumber = __builtin_LINE(),
          const char *functionName = __builtin_FUNCTION(),
          const int columnOffset = __builtin_COLUMN()) noexcept
#else
  static constexpr source_location
  current(const char *fileName = "unsupported",
          const int lineNumber = 0,
          const char *functionName = "unsupported",
          const int columnOffset = 0) noexcept
#endif
  {
    return source_location(fileName, lineNumber, functionName, columnOffset);
  }

  source_location(const source_location &) = default;
  source_location(source_location &&) = default;

  constexpr const char *file_name() const noexcept { return fileName; }

  constexpr const char *function_name() const noexcept { return functionName; }

  constexpr int line() const noexcept { return lineNumber; }

  constexpr int column() const noexcept { return columnOffset; }

private:
  constexpr source_location(const char *fileName,
                            const int lineNumber,
                            const char *functionName,
                            const int columnOffset) noexcept
      : fileName(fileName), lineNumber(lineNumber), functionName(functionName),
        columnOffset(columnOffset) {}

  const char *fileName;
  const int lineNumber;
  const char *functionName;
  const int columnOffset;
};
} // namespace details

} // namespace spdlog
