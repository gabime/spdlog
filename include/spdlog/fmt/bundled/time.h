// Formatting library for C++ - time formatting
//
// Copyright (c) 2012 - 2016, Victor Zverovich
// All rights reserved.
//
// For the license information refer to format.h.

#ifndef FMT_TIME_H_
#define FMT_TIME_H_

#include "format.h"
#include <ctime>

FMT_BEGIN_NAMESPACE

namespace internal{
inline null<> localtime_r(...) { return null<>(); }
inline null<> localtime_s(...) { return null<>(); }
inline null<> gmtime_r(...) { return null<>(); }
inline null<> gmtime_s(...) { return null<>(); }
}

// Thread-safe replacement for std::localtime
inline std::tm localtime(std::time_t time) {
  struct dispatcher {
    std::time_t time_;
    std::tm tm_;

    dispatcher(std::time_t t): time_(t) {}

    bool run() {
      using namespace fmt::internal;
      return handle(localtime_r(&time_, &tm_));
    }

    bool handle(std::tm *tm) { return tm != FMT_NULL; }

    bool handle(internal::null<>) {
      using namespace fmt::internal;
      return fallback(localtime_s(&tm_, &time_));
    }

    bool fallback(int res) { return res == 0; }

    bool fallback(internal::null<>) {
      using namespace fmt::internal;
      std::tm *tm = std::localtime(&time_);
      if (tm) tm_ = *tm;
      return tm != FMT_NULL;
    }
  };
  dispatcher lt(time);
  if (lt.run())
    return lt.tm_;
  // Too big time values may be unsupported.
  FMT_THROW(format_error("time_t value out of range"));
}

// Thread-safe replacement for std::gmtime
inline std::tm gmtime(std::time_t time) {
  struct dispatcher {
    std::time_t time_;
    std::tm tm_;

    dispatcher(std::time_t t): time_(t) {}

    bool run() {
      using namespace fmt::internal;
      return handle(gmtime_r(&time_, &tm_));
    }

    bool handle(std::tm *tm) { return tm != FMT_NULL; }

    bool handle(internal::null<>) {
      using namespace fmt::internal;
      return fallback(gmtime_s(&tm_, &time_));
    }

    bool fallback(int res) { return res == 0; }

    bool fallback(internal::null<>) {
      std::tm *tm = std::gmtime(&time_);
      if (tm) tm_ = *tm;
      return tm != FMT_NULL;
    }
  };
  dispatcher gt(time);
  if (gt.run())
    return gt.tm_;
  // Too big time values may be unsupported.
  FMT_THROW(format_error("time_t value out of range"));
}

namespace internal {
inline std::size_t strftime(char *str, std::size_t count, const char *format,
                            const std::tm *time) {
  return std::strftime(str, count, format, time);
}

inline std::size_t strftime(wchar_t *str, std::size_t count,
                            const wchar_t *format, const std::tm *time) {
  return std::wcsftime(str, count, format, time);
}
}

template <typename Char>
struct formatter<std::tm, Char> {
  template <typename ParseContext>
  auto parse(ParseContext &ctx) -> decltype(ctx.begin()) {
    auto it = internal::null_terminating_iterator<Char>(ctx);
    if (*it == ':')
      ++it;
    auto end = it;
    while (*end && *end != '}')
      ++end;
    tm_format.reserve(end - it + 1);
    using internal::pointer_from;
    tm_format.append(pointer_from(it), pointer_from(end));
    tm_format.push_back('\0');
    return pointer_from(end);
  }

  template <typename FormatContext>
  auto format(const std::tm &tm, FormatContext &ctx) -> decltype(ctx.out()) {
    internal::basic_buffer<Char> &buf = internal::get_container(ctx.out());
    std::size_t start = buf.size();
    for (;;) {
      std::size_t size = buf.capacity() - start;
      std::size_t count =
        internal::strftime(&buf[start], size, &tm_format[0], &tm);
      if (count != 0) {
        buf.resize(start + count);
        break;
      }
      if (size >= tm_format.size() * 256) {
        // If the buffer is 256 times larger than the format string, assume
        // that `strftime` gives an empty result. There doesn't seem to be a
        // better way to distinguish the two cases:
        // https://github.com/fmtlib/fmt/issues/367
        break;
      }
      const std::size_t MIN_GROWTH = 10;
      buf.reserve(buf.capacity() + (size > MIN_GROWTH ? size : MIN_GROWTH));
    }
    return ctx.out();
  }

  basic_memory_buffer<Char> tm_format;
};
FMT_END_NAMESPACE

#endif  // FMT_TIME_H_
