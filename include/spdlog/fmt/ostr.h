//
// Copyright(c) 2016 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
//
// include bundled or external copy of fmtlib's ostream support
//
#if !defined(SPDLOG_FMT_EXTERNAL)
#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include "bundled/ostream.h"
#include "fmt.h"
#else
#include <fmt/ostream.h>
#endif
