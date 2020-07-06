//
// Copyright(c) 2016-2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

// fmt 7.0.0 renamed the internal namespace to detail
// The former is still provided as an alias if the FMT_USE_INTERNAL macro is defined.
#define FMT_USE_INTERNAL

//
// Include a bundled header-only copy of fmtlib or an external one.
// By default spdlog include its own copy.
//

#if !defined(SPDLOG_FMT_EXTERNAL)
#if !defined(SPDLOG_COMPILED_LIB) && !defined(FMT_HEADER_ONLY)
#define FMT_HEADER_ONLY
#endif
#ifndef FMT_USE_WINDOWS_H
#define FMT_USE_WINDOWS_H 0
#endif
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#else // SPDLOG_FMT_EXTERNAL is defined - use external fmtlib
#include <fmt/core.h>
#include <fmt/format.h>
#endif
