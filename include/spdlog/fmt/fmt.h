//
// Copyright(c) 2016-2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

//
// Include a bundled header-only copy of fmtlib or an external one.
// By default, spdlog include its own copy.
//
#include <spdlog/tweakme.h>

#if defined(SPDLOG_USE_STD_FORMAT)  // use std::format
    #include <format>
#elif defined(SPDLOG_FMT_EXTERNAL)  // use external {fmt} lib
    #include <fmt/core.h>
    #include <fmt/format.h>
#else  // use bundled {fmt} lib
    #include "bundled/core.h"
    #include "bundled/format.h"
#endif
