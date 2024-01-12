//
// Copyright(c) 2016-2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

//
// Include a bundled header-only copy of fmtlib or an external one.
// By default, spdlog include its own copy.
//
#include "../spdlog_config.h"

#if defined(SPDLOG_USE_STD_FORMAT)  // use std::format
    #include <format>
#else
    #include "fmt/format.h"
#endif
