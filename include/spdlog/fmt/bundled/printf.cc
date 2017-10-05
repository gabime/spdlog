/*
 Formatting library for C++

 Copyright (c) 2012 - 2016, Victor Zverovich
 All rights reserved.

 For the license information refer to format.h.
 */

#include "format.h"
#include "printf.h"

namespace fmt {

template <typename Char>
void printf(BasicWriter<Char> &w, BasicCStringRef<Char> format, ArgList args);

FMT_FUNC int fprintf(std::FILE *f, CStringRef format, ArgList args) {
  MemoryWriter w;
  printf(w, format, args);
  std::size_t size = w.size();
  return std::fwrite(w.data(), 1, size, f) < size ? -1 : static_cast<int>(size);
}

#ifndef FMT_HEADER_ONLY

template void PrintfFormatter<char>::format(CStringRef format);
template void PrintfFormatter<wchar_t>::format(WCStringRef format);

#endif  // FMT_HEADER_ONLY

}  // namespace fmt
