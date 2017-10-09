//
// Copyright(c) 2016 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

//
// Include a bundled header-only copy of fmtlib or an external one.
// By default spdlog include its own copy.
//

#if !defined(SPDLOG_FMT_EXTERNAL)

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#ifndef FMT_USE_WINDOWS_H
#define FMT_USE_WINDOWS_H 0
#endif
#include "spdlog/fmt/bundled/format.h"
#if defined(SPDLOG_FMT_PRINTF)
#include "spdlog/fmt/bundled/printf.h"
#endif

#else //external fmtlib

#include <fmt/format.h>
#if defined(SPDLOG_FMT_PRINTF)
#include <fmt/printf.h>
#endif

#endif

