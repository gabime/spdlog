//
// Copyright(c) 2016-2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

//
// Include a bundled header-only copy of fmtlib or an external one.
// By default spdlog include its own copy.
//

#if !defined(SPDLOG_FMT_EXTERNAL)
# include "spdlog/common.h"
# if defined(SPDLOG_HEADER_ONLY) && !defined(FMT_HEADER_ONLY)
#  define FMT_HEADER_ONLY
# endif
# if defined(SDPLOG_SHARED) && !defined(FMT_SHARED)
#  define FMT_SHARED
# endif
# ifndef FMT_USE_WINDOWS_H
#  define FMT_USE_WINDOWS_H 0
# endif
# include "bundled/core.h"
# include "bundled/format.h"
#else // SPDLOG_FMT_EXTERNAL is defined - use external fmtlib
# include "fmt/core.h"
# include "fmt/format.h"
#endif
