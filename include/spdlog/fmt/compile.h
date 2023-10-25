//
// Copyright(c) 2016 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
//
// include bundled or external copy of fmtlib's compile-time support
//
#include <spdlog/tweakme.h>

#if !defined(SPDLOG_USE_STD_FORMAT)
    #if !defined(SPDLOG_FMT_EXTERNAL)
        #include "../fmt/bundled/compile.h"
    #else
        #include <fmt/compile.h>
    #endif
#endif
