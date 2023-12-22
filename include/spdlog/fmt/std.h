//
// Copyright(c) 2016 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
//
// include bundled or external copy of fmtlib's std support (for formatting e.g.
// std::filesystem::path, std::thread::id, std::monostate, std::variant, ...)
//
#include <spdlog/tweakme.h>

#if !defined(SPDLOG_USE_STD_FORMAT)
    #include <fmt/std.h>
#endif
